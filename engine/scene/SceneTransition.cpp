#include "SceneTransition.h"

namespace engine {
namespace scene {

SceneTransition::SceneTransition()
    : m_duration(1.0f)
    , m_elapsed(0.0f)
    , m_progress(0.0f)
    , m_complete(false)
    , m_type(Type::Fade)
    , m_easing(easeInOutQuad)
{
}

SceneTransition::SceneTransition(float duration)
    : m_duration(duration)
    , m_elapsed(0.0f)
    , m_progress(0.0f)
    , m_complete(false)
    , m_type(Type::Fade)
    , m_easing(easeInOutQuad)
{
}

SceneTransition::~SceneTransition()
{
}

void SceneTransition::start(Scene::Ptr fromScene, Scene::Ptr toScene)
{
    m_fromScene = fromScene;
    m_toScene = toScene;
    m_elapsed = 0.0f;
    m_progress = 0.0f;
    m_complete = false;
}

void SceneTransition::update(float dt)
{
    if (m_complete) return;

    m_elapsed += dt;
    m_progress = std::min(m_elapsed / m_duration, 1.0f);

    if (m_progress >= 1.0f) {
        m_complete = true;
        if (m_onComplete) {
            m_onComplete();
        }
    }
}

float SceneTransition::applyEasing(float t) const
{
    if (m_easing) {
        return m_easing(t);
    }
    return t;
}

float SceneTransition::easeLinear(float t) { return t; }
float SceneTransition::easeInQuad(float t) { return t * t; }
float SceneTransition::easeOutQuad(float t) { return t * (2.0f - t); }
float SceneTransition::easeInOutQuad(float t) { return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t; }
float SceneTransition::easeInCubic(float t) { return t * t * t; }
float SceneTransition::easeOutCubic(float t) { float u = t - 1.0f; return u * u * u + 1.0f; }
float SceneTransition::easeInOutCubic(float t) { return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f; }

FadeTransition::FadeTransition()
    : SceneTransition()
{
    m_type = Type::Fade;
}

FadeTransition::FadeTransition(float duration)
    : SceneTransition(duration)
{
    m_type = Type::Fade;
}

void FadeTransition::render(std::shared_ptr<renderer::RenderTarget> renderTarget)
{
    float alpha = applyEasing(m_progress);
    renderTarget->clear();
    if (m_fromScene) {
        m_fromScene->render(0.0f);
    }
    if (m_toScene) {
        m_toScene->render(0.0f);
    }
    renderTarget->drawOverlay(0, 0, 0, alpha);
}

SlideTransition::SlideTransition()
    : SceneTransition()
{
    m_type = Type::SlideLeft;
}

SlideTransition::SlideTransition(float duration, Type direction)
    : SceneTransition(duration)
{
    m_type = direction;
}

void SlideTransition::render(std::shared_ptr<renderer::RenderTarget> renderTarget)
{
    float t = applyEasing(m_progress);
    float width = static_cast<float>(renderTarget->getWidth());

    float offsetFrom = 0.0f;
    float offsetTo = width;

    switch (m_type) {
        case Type::SlideLeft:
            offsetFrom = -t * width;
            offsetTo = (1.0f - t) * width;
            break;
        case Type::SlideRight:
            offsetFrom = t * width;
            offsetTo = -(1.0f - t) * width;
            break;
        case Type::SlideUp:
            offsetFrom = 0.0f;
            offsetTo = 0.0f;
            break;
        case Type::SlideDown:
            offsetFrom = 0.0f;
            offsetTo = 0.0f;
            break;
        default:
            break;
    }

    renderTarget->clear();
    renderTarget->pushTransform(offsetFrom, 0.0f);
    if (m_fromScene) m_fromScene->render(0.0f);
    renderTarget->popTransform();

    renderTarget->pushTransform(offsetTo, 0.0f);
    if (m_toScene) m_toScene->render(0.0f);
    renderTarget->popTransform();
}

CrossFadeTransition::CrossFadeTransition()
    : SceneTransition()
{
    m_type = Type::CrossFade;
}

CrossFadeTransition::CrossFadeTransition(float duration)
    : SceneTransition(duration)
{
    m_type = Type::CrossFade;
}

void CrossFadeTransition::render(std::shared_ptr<renderer::RenderTarget> renderTarget)
{
    float alpha = applyEasing(m_progress);
    renderTarget->clear();
    if (m_fromScene) {
        m_fromScene->render(0.0f);
        renderTarget->drawOverlay(0, 0, 0, alpha);
    }
    if (m_toScene) {
        m_toScene->render(0.0f);
    }
}

} // namespace scene
} // namespace engine
