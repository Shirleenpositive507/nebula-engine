#pragma once

#include "RenderTarget.h"
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <string>

namespace nebula {
    namespace graphics {

        class RenderTexture : public RenderTarget {
        public:
            RenderTexture();
            explicit RenderTexture(const std::string& debugName);
            virtual ~RenderTexture();

            bool create(unsigned int width, unsigned int height, bool depthBuffer = false);
            bool create(unsigned int width, unsigned int height, const sf::ContextSettings& settings);

            sf::Texture& getTexture();
            const sf::Texture& getTexture() const;

            void display();

            void clear(const Color& color = Color::Black) override;
            void clear(const sf::Color& color) override;

            bool resize(unsigned int width, unsigned int height);

            void setSmooth(bool smooth);
            bool isSmooth() const;

            void setRepeated(bool repeated);
            bool isRepeated() const;

            void setActive(bool active = true) override;

            sf::Vector2u getSize() const override;
            std::string getDebugName() const;

            sf::RenderTarget& getSFMLTarget() override;
            const sf::RenderTarget& getSFMLTarget() const override;

            static RenderTexture createHalfSize(const std::string& name, unsigned int width, unsigned int height, bool depthBuffer = false);
            static RenderTexture createQuarterSize(const std::string& name, unsigned int width, unsigned int height, bool depthBuffer = false);
            static RenderTexture createSquare(const std::string& name, unsigned int size, bool depthBuffer = false);

        private:
            sf::RenderTexture m_renderTexture;
            std::string m_debugName;
            unsigned int m_width;
            unsigned int m_height;
            bool m_created;
        };

    }
}
