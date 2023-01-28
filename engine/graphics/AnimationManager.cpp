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

        // --- Blend Tree 1D ---

        void BlendTree1D::addNode(float position, const std::string& animationName) {
            m_nodes.push_back(BlendTree1DNode(position, animationName));
            std::sort(m_nodes.begin(), m_nodes.end(),
                [](const BlendTree1DNode& a, const BlendTree1DNode& b) {
                    return a.position < b.position;
                });
        }

        void BlendTree1D::removeNode(const std::string& animationName) {
            m_nodes.erase(std::remove_if(m_nodes.begin(), m_nodes.end(),
                [&](const BlendTree1DNode& n) { return n.animationName == animationName; }),
                m_nodes.end());
        }

        std::vector<BlendTree1DNode> BlendTree1D::evaluate(float inputPosition) const {
            if (m_nodes.empty()) return {};
            if (m_nodes.size() == 1) {
                auto result = m_nodes;
                result[0].weight = 1.0f;
                return result;
            }

            if (inputPosition <= m_nodes.front().position) {
                auto result = m_nodes;
                result[0].weight = 1.0f;
                for (std::size_t i = 1; i < result.size(); ++i) result[i].weight = 0;
                return result;
            }
            if (inputPosition >= m_nodes.back().position) {
                auto result = m_nodes;
                result.back().weight = 1.0f;
                for (std::size_t i = 0; i < result.size() - 1; ++i) result[i].weight = 0;
                return result;
            }

            for (std::size_t i = 0; i < m_nodes.size() - 1; ++i) {
                if (inputPosition >= m_nodes[i].position && inputPosition <= m_nodes[i + 1].position) {
                    float range = m_nodes[i + 1].position - m_nodes[i].position;
                    float t = (inputPosition - m_nodes[i].position) / range;
                    auto result = m_nodes;
                    for (std::size_t j = 0; j < result.size(); ++j) result[j].weight = 0;
                    result[i].weight = 1.0f - t;
                    result[i + 1].weight = t;
                    return result;
                }
            }
            return {};
        }

        void BlendTree1D::clear() { m_nodes.clear(); }

        // --- Blend Tree 2D ---

        void BlendTree2D::addNode(const sf::Vector2f& position, const std::string& animationName) {
            m_nodes.push_back(BlendTree2DNode(position, animationName));
        }

        void BlendTree2D::removeNode(const std::string& animationName) {
            m_nodes.erase(std::remove_if(m_nodes.begin(), m_nodes.end(),
                [&](const BlendTree2DNode& n) { return n.animationName == animationName; }),
                m_nodes.end());
        }

        std::vector<BlendTree2DNode> BlendTree2D::evaluate(const sf::Vector2f& inputPosition) const {
            if (m_nodes.empty()) return {};
            if (m_nodes.size() == 1) {
                auto result = m_nodes;
                result[0].weight = 1.0f;
                return result;
            }

            auto result = m_nodes;
            for (auto& node : result) {
                sf::Vector2f diff = inputPosition - node.position;
                float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
                node.weight = (dist < 0.001f) ? 1.0f : 1.0f / dist;
            }

            float totalWeight = 0;
            for (const auto& node : result) totalWeight += node.weight;
            if (totalWeight > 0) {
                for (auto& node : result) node.weight /= totalWeight;
            }
            return result;
        }

        void BlendTree2D::clear() { m_nodes.clear(); }

        // --- Animation events ---

        void AnimationManager::addEvent(const std::string& animationName, const AnimationEvent& event) {
            m_events[animationName].push_back(event);
            std::sort(m_events[animationName].begin(), m_events[animationName].end(),
                [](const AnimationEvent& a, const AnimationEvent& b) { return a.time < b.time; });
        }

        void AnimationManager::removeEvent(const std::string& animationName, const std::string& eventName) {
            auto it = m_events.find(animationName);
            if (it != m_events.end()) {
                it->second.erase(std::remove_if(it->second.begin(), it->second.end(),
                    [&](const AnimationEvent& e) { return e.name == eventName; }), it->second.end());
            }
        }

        void AnimationManager::clearEvents(const std::string& animationName) {
            m_events.erase(animationName);
        }

        std::vector<AnimationEvent> AnimationManager::getEvents(const std::string& animationName) const {
            auto it = m_events.find(animationName);
            return (it != m_events.end()) ? it->second : std::vector<AnimationEvent>();
        }

        void AnimationManager::checkEvents(const std::string& animationName, float currentTime, float previousTime) {
            auto it = m_events.find(animationName);
            if (it == m_events.end()) return;

            for (const auto& event : it->second) {
                if (event.time >= previousTime && event.time < currentTime) {
                    if (event.callback) event.callback();
                }
            }
        }

        // --- Blend trees (manager) ---

        void AnimationManager::setBlendTree1D(const std::string& name, const BlendTree1D& tree) {
            m_blendTrees1D[name] = tree;
        }

        void AnimationManager::setBlendTree2D(const std::string& name, const BlendTree2D& tree) {
            m_blendTrees2D[name] = tree;
        }

        BlendTree1D* AnimationManager::getBlendTree1D(const std::string& name) {
            auto it = m_blendTrees1D.find(name);
            return (it != m_blendTrees1D.end()) ? &it->second : nullptr;
        }

        BlendTree2D* AnimationManager::getBlendTree2D(const std::string& name) {
            auto it = m_blendTrees2D.find(name);
            return (it != m_blendTrees2D.end()) ? &it->second : nullptr;
        }

        void AnimationManager::evaluateBlendTree1D(const std::string& name, float input) {
            auto it = m_blendTrees1D.find(name);
            if (it == m_blendTrees1D.end()) return;
            auto nodes = it->second.evaluate(input);
            for (const auto& node : nodes) {
                auto animIt = m_animations.find(node.animationName);
                if (animIt != m_animations.end()) {
                    animIt->second.setWeight(node.weight);
                }
            }
        }

        void AnimationManager::evaluateBlendTree2D(const std::string& name, const sf::Vector2f& input) {
            auto it = m_blendTrees2D.find(name);
            if (it == m_blendTrees2D.end()) return;
            auto nodes = it->second.evaluate(input);
            for (const auto& node : nodes) {
                auto animIt = m_animations.find(node.animationName);
                if (animIt != m_animations.end()) {
                    animIt->second.setWeight(node.weight);
                }
            }
        }

        // --- Animation layers ---

        int AnimationManager::addLayer(const std::string& name, float weight, bool additive) {
            AnimationLayer layer;
            layer.name = name;
            layer.weight = weight;
            layer.additive = additive;
            m_layers[name] = layer;
            return static_cast<int>(m_layers.size());
        }

        bool AnimationManager::removeLayer(const std::string& name) {
            return m_layers.erase(name) > 0;
        }

        AnimationLayer* AnimationManager::getLayer(const std::string& name) {
            auto it = m_layers.find(name);
            return (it != m_layers.end()) ? &it->second : nullptr;
        }

        void AnimationManager::setLayerWeight(const std::string& name, float weight) {
            auto it = m_layers.find(name);
            if (it != m_layers.end()) it->second.weight = weight;
        }

        float AnimationManager::getLayerWeight(const std::string& name) const {
            auto it = m_layers.find(name);
            return (it != m_layers.end()) ? it->second.weight : 0.0f;
        }

        void AnimationManager::setLayerAdditive(const std::string& name, bool additive) {
            auto it = m_layers.find(name);
            if (it != m_layers.end()) it->second.additive = additive;
        }

        void AnimationManager::playOnLayer(const std::string& layerName, const std::string& animationName) {
            auto it = m_layers.find(layerName);
            if (it != m_layers.end() && m_animations.find(animationName) != m_animations.end()) {
                it->second.currentAnimation = animationName;
                it->second.time = 0;
            }
        }

        std::vector<std::string> AnimationManager::getLayerNames() const {
            std::vector<std::string> names;
            names.reserve(m_layers.size());
            for (const auto& pair : m_layers) names.push_back(pair.first);
            return names;
        }

    }
}
