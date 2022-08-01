#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <string>
#include <memory>
#include "graphics/Color.h"

namespace nebula {
    namespace ui {

        struct UIStyle {
            graphics::Color normal;
            graphics::Color hover;
            graphics::Color pressed;
            graphics::Color disabled;
            graphics::Color text;
            graphics::Color border;

            float borderSize;
            float borderRadius;
            std::string font;
            unsigned int fontSize;
            float padding;
            float margin;

            std::shared_ptr<sf::Texture> backgroundTexture;
            sf::IntRect backgroundImageRect;
            std::string icon;

            UIStyle();

            UIStyle clone() const;
            void merge(const UIStyle& other);
            void apply(sf::Shape& shape) const;
            void apply(sf::Text& textObj) const;

            static UIStyle Dark();
            static UIStyle Light();
            static UIStyle Blue();
            static UIStyle Custom();
        };

    }
}
