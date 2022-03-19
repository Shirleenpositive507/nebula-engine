#include "AnimationManager.h"
#include <algorithm>
#include <stdexcept>

namespace nebula {
    namespace graphics {

        AnimationManager::AnimationManager()
            : m_currentAnimation("")
            , m_previousAnimation("")
            , m_defaultAnimation("")
            , m_transitionTimer(0.f)
            , m_transitionDuration(0.f)
            , m_transitioning(false) {}

        void AnimationManager::addAnimation(const std::string& name, const Animation& animation) {
            Animation anim = animation;
            anim.setName(name);
            m_animations[name] = anim;

            if (m_defaultAnimation.empty()) {
                m_defaultAnimation = name;
            }
        }

        Animation& AnimationManager::getAnimation(const std::string& name) {
            auto it = m_animations.find(name);
            if (it == m_animations.end()) {
                static Animation emptyAnim;
                return emptyAnim;
            }
            return it->second;
        }

        const Animation& AnimationManager::getAnimation(const std::string& name) const {
            auto it = m_animations.find(name);
            if (it == m_animations.end()) {
                static Animation emptyAnim;
                return emptyAnim;
            }
            return it->second;
        }

        bool AnimationManager::removeAnimation(const std::string& name) {
            auto it = m_animations.find(name);
            if (it == m_animations.end()) return false;

            if (m_currentAnimation == name) {
                m_currentAnimation = "";
            }
            if (m_defaultAnimation == name) {
                m_defaultAnimation = m_animations.empty() ? "" : m_animations.begin()->first;
            }

            m_animations.erase(it);
            return true;
        }

        void AnimationManager::play(const std::string& name) {
            if (name == m_currentAnimation) return;

            auto it = m_animations.find(name);
            if (it == m_animations.end()) return;

            if (!m_currentAnimation.empty()) {
                m_animations[m_currentAnimation].stop();
            }

            m_previousAnimation = m_currentAnimation;
            m_currentAnimation = name;
            it->second.reset();
            it->second.play();

            for (const auto& trans : m_transitions) {
                if (trans.fromState == m_previousAnimation && trans.toState == name) {
                    if (!trans.immediate && trans.crossFadeTime > 0.f) {
                        m_transitioning = true;
                        m_transitionTimer = 0.f;
                        m_transitionDuration = trans.crossFadeTime;
                    } else {
                        m_transitioning = false;
                    }
                    return;
                }
            }

            m_transitioning = false;
        }

        void AnimationManager::stop() {
            if (!m_currentAnimation.empty()) {
                auto it = m_animations.find(m_currentAnimation);
                if (it != m_animations.end()) {
                    it->second.stop();
                }
            }
            m_currentAnimation = "";
            m_transitioning = false;
        }

        void AnimationManager::update(float dt) {
            if (m_transitioning) {
                m_transitionTimer += dt;
                if (m_transitionTimer >= m_transitionDuration) {
                    m_transitioning = false;
                }

                if (!m_previousAnimation.empty()) {
                    auto it = m_animations.find(m_previousAnimation);
                    if (it != m_animations.end()) {
                        it->second.update(dt);
                    }
                }
            }

            if (!m_currentAnimation.empty()) {
                auto it = m_animations.find(m_currentAnimation);
                if (it != m_animations.end()) {
                    it->second.update(dt);

                    if (it->second.isFinished() && !m_defaultAnimation.empty()) {
                        play(m_defaultAnimation);
                    }
                }
            }
        }

        std::string AnimationManager::getCurrentAnimationName() const {
            return m_currentAnimation;
        }

        bool AnimationManager::hasAnimation(const std::string& name) const {
            return m_animations.find(name) != m_animations.end();
        }

        void AnimationManager::setDefaultAnimation(const std::string& name) {
            if (m_animations.find(name) != m_animations.end()) {
                m_defaultAnimation = name;
            }
        }

        std::string AnimationManager::getDefaultAnimation() const {
            return m_defaultAnimation;
        }

        void AnimationManager::addTransition(const std::string& from, const std::string& to,
                                              bool immediate, float crossFadeTime) {
            m_transitions.emplace_back(from, to, immediate, crossFadeTime);
        }

        bool AnimationManager::canTransitionTo(const std::string& name) const {
            if (m_currentAnimation.empty()) return true;

            for (const auto& trans : m_transitions) {
                if (trans.fromState == m_currentAnimation && trans.toState == name) {
                    return true;
                }
            }
            return false;
        }

        void AnimationManager::playIfDifferent(const std::string& name) {
            if (m_currentAnimation != name) {
                play(name);
            }
        }

        std::vector<std::string> AnimationManager::getAnimationNames() const {
            std::vector<std::string> names;
            names.reserve(m_animations.size());
            for (const auto& pair : m_animations) {
                names.push_back(pair.first);
            }
            return names;
        }

        void AnimationManager::clear() {
            m_animations.clear();
            m_currentAnimation = "";
            m_previousAnimation = "";
            m_defaultAnimation = "";
            m_transitioning = false;
            m_transitions.clear();
        }

        Animation* AnimationManager::findAnimation(const std::string& name) {
            auto it = m_animations.find(name);
            return it != m_animations.end() ? &it->second : nullptr;
        }

        const Animation* AnimationManager::findAnimation(const std::string& name) const {
            auto it = m_animations.find(name);
            return it != m_animations.end() ? &it->second : nullptr;
        }

    }
}
