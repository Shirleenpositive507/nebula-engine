#pragma once

#include "Animation.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace nebula {
    namespace graphics {

        struct TransitionCondition {
            std::string fromState;
            std::string toState;
            bool immediate;
            float crossFadeTime;

            TransitionCondition()
                : fromState(""), toState(""), immediate(true), crossFadeTime(0.f) {}

            TransitionCondition(const std::string& from, const std::string& to,
                                bool imm = true, float fade = 0.f)
                : fromState(from), toState(to), immediate(imm), crossFadeTime(fade) {}
        };

        class AnimationManager {
        public:
            AnimationManager();
            ~AnimationManager() = default;

            void addAnimation(const std::string& name, const Animation& animation);
            Animation& getAnimation(const std::string& name);
            const Animation& getAnimation(const std::string& name) const;

            bool removeAnimation(const std::string& name);

            void play(const std::string& name);
            void stop();

            void update(float dt);

            std::string getCurrentAnimationName() const;
            bool hasAnimation(const std::string& name) const;

            void setDefaultAnimation(const std::string& name);
            std::string getDefaultAnimation() const;

            void addTransition(const std::string& from, const std::string& to,
                               bool immediate = true, float crossFadeTime = 0.f);
            bool canTransitionTo(const std::string& name) const;

            void playIfDifferent(const std::string& name);

            std::vector<std::string> getAnimationNames() const;
            void clear();

        private:
            std::unordered_map<std::string, Animation> m_animations;
            std::string m_currentAnimation;
            std::string m_previousAnimation;
            std::string m_defaultAnimation;
            float m_transitionTimer;
            float m_transitionDuration;
            bool m_transitioning;

            std::vector<TransitionCondition> m_transitions;

            Animation* findAnimation(const std::string& name);
            const Animation* findAnimation(const std::string& name) const;
        };

    }
}
