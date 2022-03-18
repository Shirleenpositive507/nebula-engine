#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <vector>
#include <string>

namespace nebula {
    namespace graphics {

        struct Frame {
            sf::IntRect rect;
            float duration;

            Frame() : rect(0, 0, 0, 0), duration(0.1f) {}
            Frame(const sf::IntRect& r, float d) : rect(r), duration(d) {}
        };

        class Animation {
        public:
            Animation();
            explicit Animation(const std::string& name);
            Animation(const std::vector<Frame>& frames, bool loop = true);
            ~Animation() = default;

            void addFrame(const sf::IntRect& rect, float duration);
            void addFrame(const Frame& frame);

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
            std::size_t m_currentFrame;
            float m_elapsed;
            bool m_playing;
            bool m_loop;
            bool m_finished;
        };

    }
}
