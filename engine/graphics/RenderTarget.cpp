#include "RenderTarget.h"

namespace nebula {
    namespace graphics {

        RenderTarget::RenderTarget()
            : m_blendState(BlendMode::Alpha)
            , m_viewport(0.f, 0.f, 1.f, 1.f) {}

        RenderTarget::~RenderTarget() {}

        void RenderTarget::clear(const Color& color) {
            clear(color.toSFML());
        }

        void RenderTarget::clear(const sf::Color& color) {
            getSFMLTarget().clear(color);
        }

        void RenderTarget::draw(const sf::Drawable& drawable, const sf::RenderStates& states) {
            sf::RenderStates s = states;
            s.blendMode = m_blendState.toSFML();
            getSFMLTarget().draw(drawable, s);
        }

        void RenderTarget::draw(const sf::VertexArray& vertexArray, const sf::RenderStates& states) {
            sf::RenderStates s = states;
            s.blendMode = m_blendState.toSFML();
            getSFMLTarget().draw(vertexArray, s);
        }

        void RenderTarget::draw(const sf::Vertex* vertices, std::size_t count,
                                sf::PrimitiveType type, const sf::RenderStates& states)
        {
            sf::RenderStates s = states;
            s.blendMode = m_blendState.toSFML();
            getSFMLTarget().draw(vertices, count, type, s);
        }

        void RenderTarget::setView(const sf::View& view) {
            getSFMLTarget().setView(view);
        }

        void RenderTarget::setView(const Viewport& viewport) {
            getSFMLTarget().setView(viewport.toSFML());
        }

        sf::View RenderTarget::getView() const {
            return getSFMLTarget().getView();
        }

        sf::View RenderTarget::getDefaultView() const {
            return getSFMLTarget().getDefaultView();
        }

        void RenderTarget::setActive(bool active) {
            getSFMLTarget().setActive(active);
        }

        void RenderTarget::pushGLStates() {
            getSFMLTarget().pushGLStates();
        }

        void RenderTarget::popGLStates() {
            getSFMLTarget().popGLStates();
        }

        void RenderTarget::resetGLStates() {
            getSFMLTarget().resetGLStates();
        }

        void RenderTarget::pushView(const sf::View& view) {
            m_viewStack.push(getSFMLTarget().getView());
            getSFMLTarget().setView(view);
        }

        void RenderTarget::popView() {
            if (!m_viewStack.empty()) {
                getSFMLTarget().setView(m_viewStack.top());
                m_viewStack.pop();
            }
        }

        std::size_t RenderTarget::getViewStackSize() const {
            return m_viewStack.size();
        }

        void RenderTarget::setBlendMode(const BlendState& state) {
            m_blendState = state;
        }

        BlendState RenderTarget::getBlendMode() const {
            return m_blendState;
        }

        void RenderTarget::setViewport(const sf::FloatRect& viewport) {
            m_viewport = viewport;
            sf::View view = getSFMLTarget().getView();
            view.setViewport(viewport);
            getSFMLTarget().setView(view);
        }

        sf::FloatRect RenderTarget::getViewport() const {
            return m_viewport;
        }

        void RenderTarget::applyBlendMode() {
        }

    }
}
