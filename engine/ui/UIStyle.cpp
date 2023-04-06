#include "UIStyle.h"

namespace nebula {
    namespace ui {

        UIStyle::UIStyle()
            : normal(255, 255, 255)
            , hover(230, 230, 230)
            , pressed(200, 200, 200)
            , focused(210, 210, 240)
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
            , icon("")
            , inheritFromParent(true) {}

        UIStyle UIStyle::clone() const {
            UIStyle s;
            s.normal = normal;
            s.hover = hover;
            s.pressed = pressed;
            s.focused = focused;
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
            s.transition = transition;
            s.inheritFromParent = inheritFromParent;
            s.styleSheet = styleSheet;
            return s;
        }

        void UIStyle::merge(const UIStyle& other) {
            if (other.normal.r != 255 || other.normal.g != 255 || other.normal.b != 255 || other.normal.a != 255)
                normal = other.normal;
            if (other.hover.r != 230 || other.hover.g != 230 || other.hover.b != 230 || other.hover.a != 255)
                hover = other.hover;
            if (other.pressed.r != 200 || other.pressed.g != 200 || other.pressed.b != 200 || other.pressed.a != 255)
                pressed = other.pressed;
            if (other.focused.r != 210 || other.focused.g != 210 || other.focused.b != 240 || other.focused.a != 255)
                focused = other.focused;
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
            if (other.transition.enabled) transition = other.transition;
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

        UIStyle UIStyle::getVariant(StyleState state) const {
            UIStyle variant = clone();
            switch (state) {
                case StyleState::Normal:
                    variant.normal = normal;
                    break;
                case StyleState::Hover:
                    variant.normal = hover;
                    break;
                case StyleState::Pressed:
                    variant.normal = pressed;
                    break;
                case StyleState::Focused:
                    variant.normal = focused;
                    break;
                case StyleState::Disabled:
                    variant.normal = disabled;
                    break;
            }
            return variant;
        }

        void UIStyle::applyStyleSheet(const StyleSheet& sheet, const std::string& selector) {
            styleSheet = std::make_shared<StyleSheet>(sheet);
            UIStyle resolved = sheet.resolve(selector);
            merge(resolved);
        }

        void UIStyle::setTransition(float duration) {
            transition.enabled = true;
            transition.duration = duration;
        }

        void UIStyle::inheritFrom(std::shared_ptr<UIStyle> parent) {
            parentStyle = parent;
            inheritFromParent = true;
        }

        UIStyle UIStyle::resolveFinal() const {
            if (!inheritFromParent) {
                return clone();
            }

            UIStyle result = clone();

            auto parent = parentStyle.lock();
            if (parent) {
                UIStyle parentResolved = parent->resolveFinal();
                if (result.normal.r == 255 && result.normal.g == 255 && result.normal.b == 255 && result.normal.a == 255)
                    result.normal = parentResolved.normal;
                if (result.hover.r == 230 && result.hover.g == 230 && result.hover.b == 230 && result.hover.a == 255)
                    result.hover = parentResolved.hover;
                if (result.pressed.r == 200 && result.pressed.g == 200 && result.pressed.b == 200 && result.pressed.a == 255)
                    result.pressed = parentResolved.pressed;
                if (result.disabled.r == 128 && result.disabled.g == 128 && result.disabled.b == 128 && result.disabled.a == 255)
                    result.disabled = parentResolved.disabled;
                if (result.text.r == 0 && result.text.g == 0 && result.text.b == 0 && result.text.a == 255)
                    result.text = parentResolved.text;
                if (result.border.r == 0 && result.border.g == 0 && result.border.b == 0 && result.border.a == 255)
                    result.border = parentResolved.border;
                if (result.borderSize == 1.0f) result.borderSize = parentResolved.borderSize;
                if (result.borderRadius == 0.0f) result.borderRadius = parentResolved.borderRadius;
                if (result.font.empty()) result.font = parentResolved.font;
                if (result.fontSize == 14) result.fontSize = parentResolved.fontSize;
                if (result.padding == 4.0f) result.padding = parentResolved.padding;
                if (result.margin == 0.0f) result.margin = parentResolved.margin;
            }

            return result;
        }

        void StyleSheet::setProperty(const std::string& key, const std::string& value) {
            properties[key] = value;
        }

        std::string StyleSheet::getProperty(const std::string& key, const std::string& defaultValue) const {
            auto it = properties.find(key);
            if (it != properties.end()) {
                return it->second;
            }
            return defaultValue;
        }

        UIStyle StyleSheet::resolve(const std::string& selector) const {
            auto it = styleCache.find(selector);
            if (it != styleCache.end()) {
                return it->second;
            }

            UIStyle result;
            auto propIt = properties.find(selector);
            if (propIt != properties.end()) {
            }

            return result;
        }

        UIStyle UIStyle::Dark() {
            UIStyle s;
            s.normal = graphics::Color(40, 40, 40);
            s.hover = graphics::Color(60, 60, 60);
            s.pressed = graphics::Color(30, 30, 30);
            s.focused = graphics::Color(50, 50, 80);
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
            s.focused = graphics::Color(220, 220, 250);
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
            s.focused = graphics::Color(60, 140, 210);
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
            s.focused = graphics::Color(190, 190, 220);
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
