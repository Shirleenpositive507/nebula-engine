#include "UIStyle.h"

namespace nebula {
    namespace ui {

        UIStyle::UIStyle()
            : normal(255, 255, 255)
            , hover(230, 230, 230)
            , pressed(200, 200, 200)
            , disabled(128, 128, 128)
            , text(0, 0, 0)
            , border(0, 0, 0)
            , borderSize(1.0f)
            , borderRadius(0.0f)
            , font("")
            , fontSize(14)
            , padding(4.0f)
            , margin(0.0f)
            , backgroundTexture(nullptr)
            , backgroundImageRect(0, 0, 0, 0)
            , icon("") {}

        UIStyle UIStyle::clone() const {
            UIStyle s;
            s.normal = normal;
            s.hover = hover;
            s.pressed = pressed;
            s.disabled = disabled;
            s.text = text;
            s.border = border;
            s.borderSize = borderSize;
            s.borderRadius = borderRadius;
            s.font = font;
            s.fontSize = fontSize;
            s.padding = padding;
            s.margin = margin;
            s.backgroundTexture = backgroundTexture;
            s.backgroundImageRect = backgroundImageRect;
            s.icon = icon;
            return s;
        }

        void UIStyle::merge(const UIStyle& other) {
            if (other.normal.r != 255 || other.normal.g != 255 || other.normal.b != 255 || other.normal.a != 255)
                normal = other.normal;
            if (other.hover.r != 230 || other.hover.g != 230 || other.hover.b != 230 || other.hover.a != 255)
                hover = other.hover;
            if (other.pressed.r != 200 || other.pressed.g != 200 || other.pressed.b != 200 || other.pressed.a != 255)
                pressed = other.pressed;
            if (other.disabled.r != 128 || other.disabled.g != 128 || other.disabled.b != 128 || other.disabled.a != 255)
                disabled = other.disabled;
            if (other.text.r != 0 || other.text.g != 0 || other.text.b != 0 || other.text.a != 255)
                text = other.text;
            if (other.border.r != 0 || other.border.g != 0 || other.border.b != 0 || other.border.a != 255)
                border = other.border;
            if (other.borderSize != 1.0f) borderSize = other.borderSize;
            if (other.borderRadius != 0.0f) borderRadius = other.borderRadius;
            if (!other.font.empty()) font = other.font;
            if (other.fontSize != 14) fontSize = other.fontSize;
            if (other.padding != 4.0f) padding = other.padding;
            if (other.margin != 0.0f) margin = other.margin;
            if (other.backgroundTexture) backgroundTexture = other.backgroundTexture;
            if (other.backgroundImageRect.width > 0 || other.backgroundImageRect.height > 0)
                backgroundImageRect = other.backgroundImageRect;
            if (!other.icon.empty()) icon = other.icon;
        }

        void UIStyle::apply(sf::Shape& shape) const {
            shape.setFillColor(normal.toSFML());
            if (borderSize > 0.0f) {
                shape.setOutlineThickness(borderSize);
                shape.setOutlineColor(border.toSFML());
            }
        }

        void UIStyle::apply(sf::Text& textObj) const {
            textObj.setFillColor(text.toSFML());
            textObj.setCharacterSize(fontSize);
        }

        UIStyle UIStyle::Dark() {
            UIStyle s;
            s.normal = graphics::Color(40, 40, 40);
            s.hover = graphics::Color(60, 60, 60);
            s.pressed = graphics::Color(30, 30, 30);
            s.disabled = graphics::Color(80, 80, 80);
            s.text = graphics::Color(220, 220, 220);
            s.border = graphics::Color(80, 80, 80);
            s.borderSize = 1.0f;
            s.borderRadius = 4.0f;
            s.fontSize = 14;
            s.padding = 6.0f;
            return s;
        }

        UIStyle UIStyle::Light() {
            UIStyle s;
            s.normal = graphics::Color(245, 245, 245);
            s.hover = graphics::Color(230, 230, 230);
            s.pressed = graphics::Color(200, 200, 200);
            s.disabled = graphics::Color(180, 180, 180);
            s.text = graphics::Color(30, 30, 30);
            s.border = graphics::Color(180, 180, 180);
            s.borderSize = 1.0f;
            s.borderRadius = 4.0f;
            s.fontSize = 14;
            s.padding = 6.0f;
            return s;
        }

        UIStyle UIStyle::Blue() {
            UIStyle s;
            s.normal = graphics::Color(41, 128, 185);
            s.hover = graphics::Color(52, 152, 219);
            s.pressed = graphics::Color(33, 97, 140);
            s.disabled = graphics::Color(130, 170, 200);
            s.text = graphics::Color(255, 255, 255);
            s.border = graphics::Color(33, 97, 140);
            s.borderSize = 1.0f;
            s.borderRadius = 4.0f;
            s.fontSize = 14;
            s.padding = 6.0f;
            return s;
        }

        UIStyle UIStyle::Custom() {
            UIStyle s;
            s.normal = graphics::Color(200, 200, 200);
            s.hover = graphics::Color(180, 180, 180);
            s.pressed = graphics::Color(150, 150, 150);
            s.disabled = graphics::Color(100, 100, 100);
            s.text = graphics::Color(255, 255, 255);
            s.border = graphics::Color(100, 100, 100);
            s.borderSize = 2.0f;
            s.borderRadius = 8.0f;
            s.fontSize = 16;
            s.padding = 10.0f;
            return s;
        }

    }
}
