#pragma once

#include "UIWidget.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <memory>

namespace nebula {
    namespace ui {

        enum class ScaleType {
            Fill,
            Fit,
            Stretch,
            Tile,
            Center
        };

        class UIImage : public UIWidget {
        public:
            UIImage();

            void setTexture(const std::shared_ptr<sf::Texture>& texture);
            void setSourceRect(const sf::IntRect& rect);
            void setColor(const graphics::Color& color);
            void setScaleType(ScaleType type);
            void setOpacity(float opacity);

            void onRender(sf::RenderTarget& target, sf::RenderStates states) override;
            void onLayout() override;

            bool flipX;
            bool flipY;
            bool preserveAspectRatio;

        private:
            std::shared_ptr<sf::Texture> m_texture;
            sf::Sprite m_sprite;
            sf::IntRect m_sourceRect;
            graphics::Color m_color;
            ScaleType m_scaleType;
            float m_opacity;
            bool m_dirty;

            void rebuildSprite();
        };

    }
}
