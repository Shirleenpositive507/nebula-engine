#include "Renderer.h"
#include <chrono>

namespace nebula {
    namespace graphics {

        Renderer::Renderer()
            : m_window(nullptr)
            , m_customTarget(nullptr)
            , m_activeTarget(nullptr)
            , m_frameBegan(false)
            , m_clearColor(Color::Black)
            , m_viewport(0.f, 0.f, 1.f, 1.f) {}

        Renderer::Renderer(sf::RenderWindow& window)
            : m_window(&window)
            , m_customTarget(nullptr)
            , m_activeTarget(&window)
            , m_frameBegan(false)
            , m_clearColor(Color::Black)
            , m_viewport(0.f, 0.f, 1.f, 1.f) {}

        Renderer::~Renderer() {}

        void Renderer::setWindow(sf::RenderWindow& window) {
            m_window = &window;
            if (!m_customTarget) {
                m_activeTarget = &window;
            }
        }

        sf::RenderWindow* Renderer::getWindow() const {
            return m_window;
        }

        void Renderer::beginFrame() {
            if (m_frameBegan) return;
            m_frameBegan = true;

            if (m_activeTarget) {
                m_activeTarget->clear(m_clearColor.toSFML());
            }
        }

        void Renderer::endFrame() {
            if (!m_frameBegan) return;
            m_frameBegan = false;

            if (m_customTarget) {
                m_customTarget->display();
            }

            applyState();
        }

        void Renderer::present() {
            if (m_window && !m_customTarget) {
                m_window->display();
            }
        }

        void Renderer::clear(const Color& color) {
            clear(color.toSFML());
        }

        void Renderer::clear(const sf::Color& color) {
            if (m_activeTarget) {
                m_activeTarget->clear(color);
            }
        }

        void Renderer::draw(const sf::Drawable& drawable, const sf::RenderStates& states) {
            if (!m_activeTarget) return;

            sf::RenderStates s = states;
            s.blendMode = m_blendState.toSFML();
            m_activeTarget->draw(drawable, s);
            ++m_stats.drawCalls;
        }

        void Renderer::draw(const sf::VertexArray& vertexArray, const sf::RenderStates& states) {
            if (!m_activeTarget) return;

            sf::RenderStates s = states;
            s.blendMode = m_blendState.toSFML();
            m_activeTarget->draw(vertexArray, s);
            ++m_stats.drawCalls;
            m_stats.vertices += vertexArray.getVertexCount();
        }

        void Renderer::draw(const sf::Vertex* vertices, std::size_t count,
                            sf::PrimitiveType type, const sf::RenderStates& states)
        {
            if (!m_activeTarget || !vertices || count == 0) return;

            sf::RenderStates s = states;
            s.blendMode = m_blendState.toSFML();
            m_activeTarget->draw(vertices, count, type, s);
            ++m_stats.drawCalls;
            m_stats.vertices += count;
        }

        void Renderer::draw(SpriteBatch& batch) {
            if (!m_activeTarget) return;

            batch.setTarget(*m_activeTarget);
            batch.begin(m_blendState);

            batch.end();

            m_stats.drawCalls += batch.getBatchCount();
            m_stats.vertices += batch.getVertexCount();
            m_stats.batches += batch.getBatchCount();

            batch.resetStats();
        }

        void Renderer::setBlendMode(const BlendState& state) {
            m_blendState = state;
        }

        BlendState Renderer::getBlendMode() const {
            return m_blendState;
        }

        void Renderer::setViewport(const sf::FloatRect& viewport) {
            m_viewport = viewport;
            if (m_activeTarget) {
                sf::View view = m_activeTarget->getView();
                view.setViewport(viewport);
                m_activeTarget->setView(view);
            }
        }

        sf::FloatRect Renderer::getViewport() const {
            return m_viewport;
        }

        void Renderer::setView(const sf::View& view) {
            if (m_activeTarget) {
                m_activeTarget->setView(view);
            }
        }

        sf::View Renderer::getView() const {
            if (m_activeTarget) {
                return m_activeTarget->getView();
            }
            return sf::View();
        }

        sf::View Renderer::getDefaultView() const {
            if (m_activeTarget) {
                return m_activeTarget->getDefaultView();
            }
            return sf::View();
        }

        void Renderer::pushView(const sf::View& view) {
            if (m_activeTarget) {
                m_viewStack.push(m_activeTarget->getView());
                m_activeTarget->setView(view);
            }
        }

        void Renderer::popView() {
            if (m_activeTarget && !m_viewStack.empty()) {
                m_activeTarget->setView(m_viewStack.top());
                m_viewStack.pop();
            }
        }

        std::size_t Renderer::getViewStackSize() const {
            return m_viewStack.size();
        }

        void Renderer::setRenderTarget(RenderTexture* target) {
            m_customTarget = target;
            if (target) {
                m_activeTarget = &target->getSFMLTarget();
            }
        }

        void Renderer::resetRenderTarget() {
            m_customTarget = nullptr;
            m_activeTarget = m_window;
        }

        FrameStats Renderer::getFrameStats() const {
            return m_stats;
        }

        std::size_t Renderer::getDrawCalls() const {
            return m_stats.drawCalls;
        }

        void Renderer::resetStats() {
            m_stats = FrameStats();
        }

        void Renderer::setClearColor(const Color& color) {
            m_clearColor = color;
        }

        Color Renderer::getClearColor() const {
            return m_clearColor;
        }

        void Renderer::setVSync(bool enabled) {
            if (m_window) {
                m_window->setVerticalSyncEnabled(enabled);
            }
        }

        bool Renderer::isVSync() const {
            return m_window && m_window->getVerticalSyncEnabled();
        }

        void Renderer::applyState() {
        }

    }
}
