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
#include <vector>
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

        struct BoxShadow {
            sf::Vector2f offset;
            float blurRadius;
            sf::Color color;

            BoxShadow() : offset(0.f, 0.f), blurRadius(0.f), color(0, 0, 0, 128) {}
            BoxShadow(const sf::Vector2f& off, float blur, const sf::Color& col)
                : offset(off), blurRadius(blur), color(col) {}
        };

        struct TextShadow {
            sf::Vector2f offset;
            sf::Color color;

            TextShadow() : offset(1.f, 1.f), color(0, 0, 0, 128) {}
            TextShadow(const sf::Vector2f& off, const sf::Color& col)
                : offset(off), color(col) {}
        };

        enum class GradientType {
            None,
            Linear,
            Radial
        };

        struct GradientBackground {
            GradientType type;
            sf::Color colorA;
            sf::Color colorB;
            sf::Vector2f pointA;
            sf::Vector2f pointB;

            GradientBackground() : type(GradientType::None), colorA(255, 255, 255), colorB(0, 0, 0),
                pointA(0.f, 0.f), pointB(1.f, 1.f) {}
        };

        struct CornerRadii {
            float topLeft;
            float topRight;
            float bottomRight;
            float bottomLeft;

            CornerRadii() : topLeft(0.f), topRight(0.f), bottomRight(0.f), bottomLeft(0.f) {}
            explicit CornerRadii(float all) : topLeft(all), topRight(all), bottomRight(all), bottomLeft(all) {}
            CornerRadii(float tl, float tr, float br, float bl)
                : topLeft(tl), topRight(tr), bottomRight(br), bottomLeft(bl) {}
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
            CornerRadii cornerRadii;
            std::string font;
            unsigned int fontSize;
            float padding;
            float margin;

            BoxShadow boxShadow;
            TextShadow textShadow;
            GradientBackground gradient;

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

            void setBoxShadow(const sf::Vector2f& offset, float blur, const sf::Color& color);
            void setTextShadow(const sf::Vector2f& offset, const sf::Color& color);
            void setGradient(GradientType type, const sf::Color& a, const sf::Color& b,
                             const sf::Vector2f& pointA = sf::Vector2f(0.f, 0.f),
                             const sf::Vector2f& pointB = sf::Vector2f(1.f, 1.f));
            void setCornerRadius(float tl, float tr, float br, float bl);

            static UIStyle Dark();
            static UIStyle Light();
            static UIStyle Blue();
            static UIStyle Custom();
        };

    }
}
