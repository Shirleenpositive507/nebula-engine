#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>

namespace nebula {
    namespace graphics {

        struct Frame {
            sf::IntRect rect;
            float duration;
            float speedMultiplier;

            Frame() : rect(0, 0, 0, 0), duration(0.1f), speedMultiplier(1.0f) {}
            Frame(const sf::IntRect& r, float d, float speed = 1.0f)
                : rect(r), duration(d), speedMultiplier(speed) {}
        };

        struct AnimationEvent {
            std::string name;
            float triggerTime;
            std::function<void()> callback;
            bool triggered;

            AnimationEvent() : triggerTime(0.f), triggered(false) {}
            AnimationEvent(const std::string& n, float t, std::function<void()> cb)
                : name(n), triggerTime(t), callback(cb), triggered(false) {}
        };

        class SpriteSheetParser {
        public:
            static bool parseAsepriteJson(const std::string& jsonPath,
                                          std::vector<Frame>& outFrames,
                                          std::unordered_map<std::string, std::vector<size_t>>& outTags);
            static bool parseSpriterJson(const std::string& jsonPath,
                                         std::vector<Frame>& outFrames,
                                         std::unordered_map<std::string, std::vector<size_t>>& outAnimations);
        };

        class Animation {
        public:
            Animation();
            explicit Animation(const std::string& name);
            Animation(const std::vector<Frame>& frames, bool loop = true);
            ~Animation() = default;

            void addFrame(const sf::IntRect& rect, float duration);
            void addFrame(const Frame& frame);

            void addEvent(const AnimationEvent& event);
            void addEvent(const std::string& name, float triggerTime, std::function<void()> callback);
            void clearEvents();
            const std::vector<AnimationEvent>& getEvents() const;

            void setFrameSpeedMultiplier(std::size_t index, float multiplier);
            float getFrameSpeedMultiplier(std::size_t index) const;

            void play();
            void pause();
            void stop();
            void reset();

            void update(float dt);

            const Frame& getCurrentFrame() const;
            std::size_t getCurrentFrameIndex() const;
            void setFrame(std::size_t index);

            std::size_t getFrameCount() const;
            bool isPlaying() const;
            bool isFinished() const;

            void setLoop(bool loop);
            bool getLoop() const;

            float getAnimationDuration() const;
            const std::string& getName() const;
            void setName(const std::string& name);

            const std::vector<Frame>& getFrames() const;

        private:
            std::string m_name;
            std::vector<Frame> m_frames;
            std::vector<AnimationEvent> m_events;
            std::size_t m_currentFrame;
            float m_elapsed;
            bool m_playing;
            bool m_loop;
            bool m_finished;

            void checkEvents(float prevElapsed, float currentElapsed);
        };

    }
}
