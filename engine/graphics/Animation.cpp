#include "Animation.h"
#include <algorithm>
#include <stdexcept>

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

            m_elapsed += dt;

            const Frame& frame = m_frames[m_currentFrame];

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

    }
}
