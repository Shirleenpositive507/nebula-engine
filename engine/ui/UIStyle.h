#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include "graphics/Color.h"

namespace nebula {
    namespace ui {

        enum class StyleState {
            Normal,
            Hover,
            Pressed,
            Focused,
            Disabled
        };

        struct StyleTransition {
            float duration;
            std::function<float(float)> easing;
            bool enabled;

            StyleTransition() : duration(0.2f), enabled(false) {
                easing = [](float t) { return t * t * (3.f - 2.f * t); };
            }
        };

        struct StyleSheet {
            std::unordered_map<std::string, std::string> properties;
            std::unordered_map<std::string, UIStyle> styleCache;

            void setProperty(const std::string& key, const std::string& value);
            std::string getProperty(const std::string& key, const std::string& defaultValue = "") const;
            UIStyle resolve(const std::string& selector) const;
        };

        struct UIStyle {
            graphics::Color normal;
            graphics::Color hover;
            graphics::Color pressed;
            graphics::Color focused;
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

            StyleTransition transition;
            std::weak_ptr<UIStyle> parentStyle;
            bool inheritFromParent;

            std::shared_ptr<StyleSheet> styleSheet;

            UIStyle();

            UIStyle clone() const;
            void merge(const UIStyle& other);
            void apply(sf::Shape& shape) const;
            void apply(sf::Text& textObj) const;

            UIStyle getVariant(StyleState state) const;
            void applyStyleSheet(const StyleSheet& sheet, const std::string& selector);
            void setTransition(float duration);
            void inheritFrom(std::shared_ptr<UIStyle> parent);
            UIStyle resolveFinal() const;

            static UIStyle Dark();
            static UIStyle Light();
            static UIStyle Blue();
            static UIStyle Custom();
        };

    }
}
