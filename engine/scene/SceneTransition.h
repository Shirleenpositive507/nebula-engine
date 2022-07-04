#pragma once

#include <memory>
#include <functional>

#include "engine/scene/Scene.h"
#include "engine/core/math/Vector2.h"
#include "engine/renderer/RenderTarget.h"

namespace engine {
namespace scene {

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

    void setOnComplete(OnCompleteCallback callback) { m_onComplete = callback; }

    Type getType() const { return m_type; }

    static float easeLinear(float t);
    static float easeInQuad(float t);
    static float easeOutQuad(float t);
    static float easeInOutQuad(float t);
    static float easeInCubic(float t);
    static float easeOutCubic(float t);
    static float easeInOutCubic(float t);

protected:
    Scene::Ptr m_fromScene;
    Scene::Ptr m_toScene;
    float m_duration;
    float m_elapsed;
    float m_progress;
    bool m_complete;
    Type m_type;
    EasingFunction m_easing;
    OnCompleteCallback m_onComplete;
};

class FadeTransition : public SceneTransition {
public:
    FadeTransition();
    explicit FadeTransition(float duration);
    void render(std::shared_ptr<renderer::RenderTarget> renderTarget) override;
};

class SlideTransition : public SceneTransition {
public:
    SlideTransition();
    SlideTransition(float duration, Type direction);
    void render(std::shared_ptr<renderer::RenderTarget> renderTarget) override;
};

class CrossFadeTransition : public SceneTransition {
public:
    CrossFadeTransition();
    explicit CrossFadeTransition(float duration);
    void render(std::shared_ptr<renderer::RenderTarget> renderTarget) override;
};

} // namespace scene
} // namespace engine
