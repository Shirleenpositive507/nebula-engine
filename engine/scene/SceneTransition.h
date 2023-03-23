#pragma once

#include <memory>
#include <functional>

#include "engine/scene/Scene.h"
#include "engine/core/math/Vector2.h"
#include "engine/core/math/Color.h"
#include "engine/renderer/RenderTarget.h"

namespace engine {
namespace scene {

enum class TransitionDirection {
    In,
    Out,
    InOut
};

enum class EaseType {
    Linear,
    Quad,
    Cubic,
    Quart,
    Quint,
    Expo,
    Elastic,
    Bounce,
    Back
};

struct TransitionOverlay {
    Color color;
    float alpha;

    TransitionOverlay() : color(0, 0, 0), alpha(0.0f) {}
    TransitionOverlay(const Color& c, float a) : color(c), alpha(a) {}
};

class SceneTransition {
public:
    using Ptr = std::shared_ptr<SceneTransition>;
    using OnCompleteCallback = std::function<void()>;
    using EasingFunction = std::function<float(float)>;

    enum class Type {
        Fade,
        SlideLeft,
        SlideRight,
        SlideUp,
        SlideDown,
        CircleReveal,
        Checkerboard,
        CrossFade
    };

    SceneTransition();
    explicit SceneTransition(float duration);
    virtual ~SceneTransition();

    SceneTransition(const SceneTransition&) = delete;
    SceneTransition& operator=(const SceneTransition&) = delete;
    SceneTransition(SceneTransition&&) = default;
    SceneTransition& operator=(SceneTransition&&) = default;

    void start(Scene::Ptr fromScene, Scene::Ptr toScene);
    virtual void update(float dt);
    virtual void render(std::shared_ptr<renderer::RenderTarget> renderTarget) = 0;
    bool isComplete() const { return m_complete; }

    void setDuration(float duration) { m_duration = duration; }
    float getDuration() const { return m_duration; }
    float getProgress() const { return m_progress; }

    void setEasingFunction(EasingFunction func) { m_easing = func; }
    float applyEasing(float t) const;

    void setEaseType(EaseType type);
    EaseType getEaseType() const { return m_easeType; }

    void setDirection(TransitionDirection dir) { m_direction = dir; }
    TransitionDirection getDirection() const { return m_direction; }

    void setOverlay(const TransitionOverlay& overlay) { m_overlay = overlay; }
    const TransitionOverlay& getOverlay() const { return m_overlay; }

    void setOnComplete(OnCompleteCallback callback) { m_onComplete = callback; }

    Type getType() const { return m_type; }

    static float easeLinear(float t);
    static float easeInQuad(float t);
    static float easeOutQuad(float t);
    static float easeInOutQuad(float t);
    static float easeInCubic(float t);
    static float easeOutCubic(float t);
    static float easeInOutCubic(float t);
    static float easeInQuart(float t);
    static float easeOutQuart(float t);
    static float easeInOutQuart(float t);
    static float easeInQuint(float t);
    static float easeOutQuint(float t);
    static float easeInOutQuint(float t);
    static float easeInExpo(float t);
    static float easeOutExpo(float t);
    static float easeInOutExpo(float t);
    static float easeInElastic(float t);
    static float easeOutElastic(float t);
    static float easeInOutElastic(float t);
    static float easeInBounce(float t);
    static float easeOutBounce(float t);
    static float easeInOutBounce(float t);
    static float easeInBack(float t);
    static float easeOutBack(float t);
    static float easeInOutBack(float t);

    void setParticleEffect(const std::string& effectName) { m_particleEffect = effectName; }
    const std::string& getParticleEffect() const { return m_particleEffect; }
    bool hasParticleEffect() const { return !m_particleEffect.empty(); }

protected:
    Scene::Ptr m_fromScene;
    Scene::Ptr m_toScene;
    float m_duration;
    float m_elapsed;
    float m_progress;
    bool m_complete;
    Type m_type;
    EaseType m_easeType;
    TransitionDirection m_direction;
    TransitionOverlay m_overlay;
    EasingFunction m_easing;
    OnCompleteCallback m_onComplete;
    std::string m_particleEffect;
};

} // namespace scene
} // namespace engine
