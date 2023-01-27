#pragma once

#include "Animation.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include <map>

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

        struct AnimationEvent {
            std::string name;
            float time;
            std::function<void()> callback;

            AnimationEvent() : time(0) {}
            AnimationEvent(const std::string& n, float t, std::function<void()> cb)
                : name(n), time(t), callback(cb) {}
        };

        struct BlendTree1DNode {
            float position;
            std::string animationName;
            float weight;

            BlendTree1DNode() : position(0), weight(1) {}
            BlendTree1DNode(float pos, const std::string& anim)
                : position(pos), animationName(anim), weight(1) {}
        };

        struct BlendTree2DNode {
            sf::Vector2f position;
            std::string animationName;
            float weight;

            BlendTree2DNode() : weight(1) {}
            BlendTree2DNode(const sf::Vector2f& pos, const std::string& anim)
                : position(pos), animationName(anim), weight(1) {}
        };

        class BlendTree1D {
        public:
            void addNode(float position, const std::string& animationName);
            void removeNode(const std::string& animationName);
            std::vector<BlendTree1DNode> evaluate(float inputPosition) const;
            void clear();

        private:
            std::vector<BlendTree1DNode> m_nodes;
        };

        class BlendTree2D {
        public:
            void addNode(const sf::Vector2f& position, const std::string& animationName);
            void removeNode(const std::string& animationName);
            std::vector<BlendTree2DNode> evaluate(const sf::Vector2f& inputPosition) const;
            void clear();

        private:
            std::vector<BlendTree2DNode> m_nodes;
        };

        struct AnimationLayer {
            std::string name;
            float weight;
            bool additive;
            bool enabled;
            std::string currentAnimation;
            float time;
            float speed;

            AnimationLayer()
                : name("base"), weight(1.0f), additive(false)
                , enabled(true), time(0), speed(1.0f) {}
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

            // Animation events
            void addEvent(const std::string& animationName, const AnimationEvent& event);
            void removeEvent(const std::string& animationName, const std::string& eventName);
            void clearEvents(const std::string& animationName);
            std::vector<AnimationEvent> getEvents(const std::string& animationName) const;

            // Blend trees
            void setBlendTree1D(const std::string& name, const BlendTree1D& tree);
            void setBlendTree2D(const std::string& name, const BlendTree2D& tree);
            BlendTree1D* getBlendTree1D(const std::string& name);
            BlendTree2D* getBlendTree2D(const std::string& name);
            void evaluateBlendTree1D(const std::string& name, float input);
            void evaluateBlendTree2D(const std::string& name, const sf::Vector2f& input);

            // Animation layers
            int addLayer(const std::string& name, float weight = 1.0f, bool additive = false);
            bool removeLayer(const std::string& name);
            AnimationLayer* getLayer(const std::string& name);
            void setLayerWeight(const std::string& name, float weight);
            float getLayerWeight(const std::string& name) const;
            void setLayerAdditive(const std::string& name, bool additive);
            void playOnLayer(const std::string& layerName, const std::string& animationName);
            std::vector<std::string> getLayerNames() const;

        private:
            std::unordered_map<std::string, Animation> m_animations;
            std::string m_currentAnimation;
            std::string m_previousAnimation;
            std::string m_defaultAnimation;
            float m_transitionTimer;
            float m_transitionDuration;
            bool m_transitioning;

            std::vector<TransitionCondition> m_transitions;

            std::unordered_map<std::string, std::vector<AnimationEvent>> m_events;
            std::unordered_map<std::string, BlendTree1D> m_blendTrees1D;
            std::unordered_map<std::string, BlendTree2D> m_blendTrees2D;
            std::unordered_map<std::string, AnimationLayer> m_layers;
            std::map<float, std::string> m_playedEvents;

            Animation* findAnimation(const std::string& name);
            const Animation* findAnimation(const std::string& name) const;

            void checkEvents(const std::string& animationName, float currentTime, float previousTime);
        };

    }
}
