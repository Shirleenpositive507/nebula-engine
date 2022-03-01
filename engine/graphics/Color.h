#pragma once

#include <cstdint>
#include <string>
#include <SFML/Graphics/Color.hpp>

namespace nebula {
    namespace graphics {

        class Color {
        public:
            uint8_t r, g, b, a;

            Color();
            Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);

            bool operator==(const Color& other) const;
            bool operator!=(const Color& other) const;
            Color operator+(const Color& other) const;
            Color operator-(const Color& other) const;
            Color operator*(float scalar) const;
            Color operator/(float scalar) const;
            Color& operator+=(const Color& other);
            Color& operator-=(const Color& other);
            Color& operator*=(float scalar);
            Color& operator/=(float scalar);

            std::string toHex(bool withAlpha = false) const;
            static Color fromHex(const std::string& hex);

            sf::Color toSFML() const;
            static Color fromSFML(const sf::Color& color);

            static Color lerp(const Color& a, const Color& b, float t);
            Color withAlpha(uint8_t alpha) const;
            float brightness() const;
            Color invert() const;

            static const Color White;
            static const Color Black;
            static const Color Red;
            static const Color Green;
            static const Color Blue;
            static const Color Yellow;
            static const Color Magenta;
            static const Color Cyan;
            static const Color Transparent;
            static const Color Orange;
            static const Color Purple;
            static const Color Gray;
            static const Color DarkGray;
            static const Color LightGray;
            static const Color Brown;
            static const Color Pink;
            static const Color Lime;
            static const Color Teal;
            static const Color Lavender;
            static const Color Maroon;
            static const Color Navy;
            static const Color Olive;
            static const Color Coral;
            static const Color Gold;
            static const Color Silver;
            static const Color Indigo;
            static const Color Violet;
            static const Color Turquoise;
            static const Color Salmon;
            static const Color Beige;
        };

    }
}
