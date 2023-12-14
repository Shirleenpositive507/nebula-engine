#include "Animation.h"
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cmath>

namespace nebula {
    namespace graphics {

        Animation::Animation()
            : m_name("default")
            , m_currentFrame(0)
            , m_elapsed(0.f)
            , m_playing(false)
            , m_loop(true)
            , m_finished(false) {}

        Animation::Animation(const std::string& name)
            : m_name(name)
            , m_currentFrame(0)
            , m_elapsed(0.f)
            , m_playing(false)
            , m_loop(true)
            , m_finished(false) {}

        Animation::Animation(const std::vector<Frame>& frames, bool loop)
            : m_name("default")
            , m_frames(frames)
            , m_currentFrame(0)
            , m_elapsed(0.f)
            , m_playing(false)
            , m_loop(loop)
            , m_finished(false) {}

        void Animation::addFrame(const sf::IntRect& rect, float duration) {
            m_frames.emplace_back(rect, std::max(0.001f, duration));
        }

        void Animation::addFrame(const Frame& frame) {
            m_frames.push_back(frame);
        }

        void Animation::addEvent(const AnimationEvent& event) {
            m_events.push_back(event);
        }

        void Animation::addEvent(const std::string& name, float triggerTime, std::function<void()> callback) {
            m_events.emplace_back(name, triggerTime, callback);
        }

        void Animation::clearEvents() {
            m_events.clear();
        }

        const std::vector<AnimationEvent>& Animation::getEvents() const {
            return m_events;
        }

        void Animation::setFrameSpeedMultiplier(std::size_t index, float multiplier) {
            if (index < m_frames.size()) {
                m_frames[index].speedMultiplier = std::max(0.01f, multiplier);
            }
        }

        float Animation::getFrameSpeedMultiplier(std::size_t index) const {
            if (index < m_frames.size()) {
                return m_frames[index].speedMultiplier;
            }
            return 1.0f;
        }

        void Animation::play() {
            if (m_frames.empty()) return;
            m_playing = true;
            if (m_finished) {
                m_currentFrame = 0;
                m_elapsed = 0.f;
                m_finished = false;
            }
        }

        void Animation::pause() {
            m_playing = false;
        }

        void Animation::stop() {
            m_playing = false;
            m_currentFrame = 0;
            m_elapsed = 0.f;
            m_finished = false;
        }

        void Animation::reset() {
            m_currentFrame = 0;
            m_elapsed = 0.f;
            m_finished = false;
            m_playing = false;
        }

        void Animation::update(float dt) {
            if (!m_playing || m_frames.empty()) return;

            float prevElapsed = m_elapsed;
            const Frame& frame = m_frames[m_currentFrame];
            float adjustedDt = dt * frame.speedMultiplier;
            m_elapsed += adjustedDt;

            if (m_elapsed >= frame.duration) {
                m_elapsed -= frame.duration;

                if (m_currentFrame + 1 < m_frames.size()) {
                    m_currentFrame++;
                } else {
                    if (m_loop) {
                        m_currentFrame = 0;
                    } else {
                        m_currentFrame = m_frames.size() - 1;
                        m_elapsed = m_frames.back().duration;
                        m_playing = false;
                        m_finished = true;
                    }
                }
            }

            checkEvents(prevElapsed, prevElapsed + adjustedDt);
        }

        void Animation::checkEvents(float prevElapsed, float currentElapsed) {
            float frameStart = 0.f;
            for (size_t i = 0; i < m_currentFrame; ++i) {
                frameStart += m_frames[i].duration;
            }
            float frameEnd = frameStart + m_frames[m_currentFrame].duration;

            for (auto& event : m_events) {
                if (event.triggered) continue;
                if (event.triggerTime >= frameStart && event.triggerTime < frameEnd) {
                    float eventLocal = event.triggerTime - frameStart;
                    float frameLocal = m_elapsed;
                    if (frameLocal >= eventLocal) {
                        event.triggered = true;
                        if (event.callback) event.callback();
                    }
                }
            }
        }

        const Frame& Animation::getCurrentFrame() const {
            if (m_frames.empty()) {
                static Frame emptyFrame;
                return emptyFrame;
            }
            return m_frames[m_currentFrame];
        }

        std::size_t Animation::getCurrentFrameIndex() const {
            return m_currentFrame;
        }

        void Animation::setFrame(std::size_t index) {
            if (index < m_frames.size()) {
                m_currentFrame = index;
                m_elapsed = 0.f;
            }
        }

        std::size_t Animation::getFrameCount() const {
            return m_frames.size();
        }

        bool Animation::isPlaying() const {
            return m_playing;
        }

        bool Animation::isFinished() const {
            return m_finished;
        }

        void Animation::setLoop(bool loop) {
            m_loop = loop;
            if (loop) m_finished = false;
        }

        bool Animation::getLoop() const {
            return m_loop;
        }

        float Animation::getAnimationDuration() const {
            float total = 0.f;
            for (const auto& frame : m_frames) {
                total += frame.duration;
            }
            return total;
        }

        const std::string& Animation::getName() const {
            return m_name;
        }

        void Animation::setName(const std::string& name) {
            m_name = name;
        }

        const std::vector<Frame>& Animation::getFrames() const {
            return m_frames;
        }

        bool SpriteSheetParser::parseAsepriteJson(const std::string& jsonPath,
                                                  std::vector<Frame>& outFrames,
                                                  std::unordered_map<std::string, std::vector<size_t>>& outTags) {
            std::ifstream file(jsonPath);
            if (!file.is_open()) return false;

            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string content = buffer.str();
            file.close();

            size_t pos = 0;
            std::string targetFrameTag = "\"frame\"";
            std::string xTag = "\"x\"";
            std::string yTag = "\"y\"";
            std::string wTag = "\"w\"";
            std::string hTag = "\"h\"";
            std::string durationTag = "\"duration\"";
            std::string metaTag = "\"meta\"";
            std::string frameTagsTag = "\"frameTags\"";
            std::string nameTag = "\"name\"";
            std::string fromTag = "\"from\"";
            std::string toTag = "\"to\"";

            auto findInt = [&](const std::string& tag) -> int {
                size_t p = content.find(tag, pos);
                if (p == std::string::npos) return 0;
                p = content.find(':', p);
                if (p == std::string::npos) return 0;
                p = content.find_first_of("0123456789-", p);
                if (p == std::string::npos) return 0;
                char* end = nullptr;
                return static_cast<int>(std::strtol(content.c_str() + p, &end, 10));
            };

            auto findFloat = [&](const std::string& tag) -> float {
                size_t p = content.find(tag, pos);
                if (p == std::string::npos) return 0.f;
                p = content.find(':', p);
                if (p == std::string::npos) return 0.f;
                p = content.find_first_of("0123456789.", p);
                if (p == std::string::npos) return 0.f;
                char* end = nullptr;
                return std::strtof(content.c_str() + p, &end);
            };

            auto findString = [&](const std::string& tag) -> std::string {
                size_t p = content.find(tag, pos);
                if (p == std::string::npos) return "";
                p = content.find(':', p);
                if (p == std::string::npos) return "";
                p = content.find('"', p);
                if (p == std::string::npos) return "";
                ++p;
                size_t end = content.find('"', p);
                if (end == std::string::npos) return "";
                return content.substr(p, end - p);
            };

            outFrames.clear();
            outTags.clear();

            std::string framesSection = "\"frames\"";
            size_t framesStart = content.find(framesSection);
            if (framesStart == std::string::npos) return false;

            pos = framesStart;
            while (true) {
                pos = content.find('{', pos + 1);
                if (pos == std::string::npos) break;

                size_t nextFrame = content.find(targetFrameTag, pos);
                if (nextFrame == std::string::npos || nextFrame > content.find('}', pos)) break;

                int x = findInt(xTag);
                int y = findInt(yTag);
                int w = findInt(wTag);
                int h = findInt(hTag);
                float dur = findFloat(durationTag) / 1000.f;

                if (w > 0 && h > 0) {
                    outFrames.emplace_back(sf::IntRect(x, y, w, h), std::max(0.001f, dur));
                }

                pos = content.find('}', pos);
            }

            size_t metaPos = content.find(metaTag);
            if (metaPos != std::string::npos) {
                size_t tagsPos = content.find(frameTagsTag, metaPos);
                if (tagsPos != std::string::npos) {
                    pos = tagsPos;
                    while (true) {
                        pos = content.find('{', pos + 1);
                        if (pos == std::string::npos) break;
                        if (content.find('}', pos) == std::string::npos) break;

                        std::string tagName = findString(nameTag);
                        int from = findInt(fromTag);
                        int to = findInt(toTag);

                        if (!tagName.empty()) {
                            std::vector<size_t> frameIndices;
                            for (int i = from; i <= to && i < static_cast<int>(outFrames.size()); ++i) {
                                frameIndices.push_back(static_cast<size_t>(i));
                            }
                            outTags[tagName] = frameIndices;
                        }

                        pos = content.find('}', pos);
                    }
                }
            }

            return !outFrames.empty();
        }

        bool SpriteSheetParser::parseSpriterJson(const std::string& jsonPath,
                                                 std::vector<Frame>& outFrames,
                                                 std::unordered_map<std::string, std::vector<size_t>>& outAnimations) {
            (void)jsonPath;
            (void)outFrames;
            (void)outAnimations;
            return false;
        }

    }
}
