#include "RenderTexture.h"

namespace nebula {
    namespace graphics {

        RenderTexture::RenderTexture()
            : m_width(0)
            , m_height(0)
            , m_created(false) {}

        RenderTexture::RenderTexture(const std::string& debugName)
            : m_debugName(debugName)
            , m_width(0)
            , m_height(0)
            , m_created(false) {}

        RenderTexture::~RenderTexture() {}

        bool RenderTexture::create(unsigned int width, unsigned int height, bool depthBuffer) {
            sf::ContextSettings settings;
            settings.depthBits = depthBuffer ? 24 : 0;
            return create(width, height, settings);
        }

        bool RenderTexture::create(unsigned int width, unsigned int height, const sf::ContextSettings& settings) {
            m_width = width;
            m_height = height;

            if (!m_renderTexture.create(width, height, settings)) {
                m_created = false;
                return false;
            }

            m_created = true;
            return true;
        }

        sf::Texture& RenderTexture::getTexture() {
            return m_renderTexture.getTexture();
        }

        const sf::Texture& RenderTexture::getTexture() const {
            return m_renderTexture.getTexture();
        }

        void RenderTexture::display() {
            if (m_created) {
                m_renderTexture.display();
            }
        }

        void RenderTexture::clear(const Color& color) {
            clear(color.toSFML());
        }

        void RenderTexture::clear(const sf::Color& color) {
            if (m_created) {
                m_renderTexture.clear(color);
            }
        }

        bool RenderTexture::resize(unsigned int width, unsigned int height) {
            if (width == m_width && height == m_height) return true;
            return create(width, height);
        }

        void RenderTexture::setSmooth(bool smooth) {
            if (m_created) {
                m_renderTexture.setSmooth(smooth);
            }
        }

        bool RenderTexture::isSmooth() const {
            return m_created && m_renderTexture.isSmooth();
        }

        void RenderTexture::setRepeated(bool repeated) {
            if (m_created) {
                m_renderTexture.setRepeated(repeated);
            }
        }

        bool RenderTexture::isRepeated() const {
            return m_created && m_renderTexture.isRepeated();
        }

        void RenderTexture::setActive(bool active) {
            if (m_created) {
                m_renderTexture.setActive(active);
            }
        }

        sf::Vector2u RenderTexture::getSize() const {
            return sf::Vector2u(m_width, m_height);
        }

        std::string RenderTexture::getDebugName() const {
            return m_debugName;
        }

        sf::RenderTarget& RenderTexture::getSFMLTarget() {
            return m_renderTexture;
        }

        const sf::RenderTarget& RenderTexture::getSFMLTarget() const {
            return m_renderTexture;
        }

        RenderTexture RenderTexture::createHalfSize(const std::string& name, unsigned int width, unsigned int height, bool depthBuffer) {
            RenderTexture rt(name);
            rt.create(width / 2, height / 2, depthBuffer);
            return rt;
        }

        RenderTexture RenderTexture::createQuarterSize(const std::string& name, unsigned int width, unsigned int height, bool depthBuffer) {
            RenderTexture rt(name);
            rt.create(width / 4, height / 4, depthBuffer);
            return rt;
        }

        RenderTexture RenderTexture::createSquare(const std::string& name, unsigned int size, bool depthBuffer) {
            RenderTexture rt(name);
            rt.create(size, size, depthBuffer);
            return rt;
        }

    }
}
