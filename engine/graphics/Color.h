#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <SFML/Graphics/Color.hpp>

namespace nebula {
    namespace graphics {

        struct HSL {
            float h, s, l, a;
            HSL() : h(0), s(0), l(0), a(1) {}
            HSL(float hue, float sat, float light, float alpha = 1)
                : h(hue), s(sat), l(light), a(alpha) {}
        };

        struct HSV {
            float h, s, v, a;
            HSV() : h(0), s(0), v(0), a(1) {}
            HSV(float hue, float sat, float val, float alpha = 1)
                : h(hue), s(sat), v(val), a(alpha) {}
        };

        enum class BlendOp {
            Normal,
            Multiply,
            Screen,
            Overlay,
            Add,
            Subtract
        };

        class ColorStop {
        public:
            float position;
            Color color;

            ColorStop() : position(0) {}
            ColorStop(float pos, const Color& col) : position(pos), color(col) {}
        };

        class Gradient {
        public:
            Gradient();
            explicit Gradient(const std::vector<ColorStop>& stops);

            void addStop(float position, const Color& color);
            void removeStop(float position);
            void clearStops();
            void sortStops();

            Color evaluate(float t) const;
            std::vector<Color> evaluateMany(const std::vector<float>& positions) const;

            std::size_t getNumStops() const;
            const std::vector<ColorStop>& getStops() const;

        private:
            std::vector<ColorStop> m_stops;
        };

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
            float luminance() const;
            Color invert() const;

            HSL toHSL() const;
            HSV toHSV() const;
            static Color fromHSL(const HSL& hsl);
            static Color fromHSV(const HSV& hsv);

            Color blend(const Color& other, BlendOp op = BlendOp::Normal) const;
            static Color blend(const Color& a, const Color& b, BlendOp op);
            Color multiply(const Color& other) const;
            Color screen(const Color& other) const;
            Color overlay(const Color& other) const;

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
