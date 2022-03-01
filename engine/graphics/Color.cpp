#include "Color.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <cctype>

namespace nebula {
    namespace graphics {

        const Color Color::White(255, 255, 255);
        const Color Color::Black(0, 0, 0);
        const Color Color::Red(255, 0, 0);
        const Color Color::Green(0, 255, 0);
        const Color Color::Blue(0, 0, 255);
        const Color Color::Yellow(255, 255, 0);
        const Color Color::Magenta(255, 0, 255);
        const Color Color::Cyan(0, 255, 255);
        const Color Color::Transparent(0, 0, 0, 0);
        const Color Color::Orange(255, 165, 0);
        const Color Color::Purple(128, 0, 128);
        const Color Color::Gray(128, 128, 128);
        const Color Color::DarkGray(64, 64, 64);
        const Color Color::LightGray(192, 192, 192);
        const Color Color::Brown(139, 69, 19);
        const Color Color::Pink(255, 192, 203);
        const Color Color::Lime(0, 255, 0);
        const Color Color::Teal(0, 128, 128);
        const Color Color::Lavender(230, 230, 250);
        const Color Color::Maroon(128, 0, 0);
        const Color Color::Navy(0, 0, 128);
        const Color Color::Olive(128, 128, 0);
        const Color Color::Coral(255, 127, 80);
        const Color Color::Gold(255, 215, 0);
        const Color Color::Silver(192, 192, 192);
        const Color Color::Indigo(75, 0, 130);
        const Color Color::Violet(238, 130, 238);
        const Color Color::Turquoise(64, 224, 208);
        const Color Color::Salmon(250, 128, 114);
        const Color Color::Beige(245, 245, 220);

        Color::Color() : r(255), g(255), b(255), a(255) {}

        Color::Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
            : r(red), g(green), b(blue), a(alpha) {}

        bool Color::operator==(const Color& other) const {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }

        bool Color::operator!=(const Color& other) const {
            return !(*this == other);
        }

        Color Color::operator+(const Color& other) const {
            return Color(
                static_cast<uint8_t>(std::min(255, static_cast<int>(r) + static_cast<int>(other.r))),
                static_cast<uint8_t>(std::min(255, static_cast<int>(g) + static_cast<int>(other.g))),
                static_cast<uint8_t>(std::min(255, static_cast<int>(b) + static_cast<int>(other.b))),
                static_cast<uint8_t>(std::min(255, static_cast<int>(a) + static_cast<int>(other.a)))
            );
        }

        Color Color::operator-(const Color& other) const {
            return Color(
                static_cast<uint8_t>(std::max(0, static_cast<int>(r) - static_cast<int>(other.r))),
                static_cast<uint8_t>(std::max(0, static_cast<int>(g) - static_cast<int>(other.g))),
                static_cast<uint8_t>(std::max(0, static_cast<int>(b) - static_cast<int>(other.b))),
                static_cast<uint8_t>(std::max(0, static_cast<int>(a) - static_cast<int>(other.a)))
            );
        }

        Color Color::operator*(float scalar) const {
            return Color(
                static_cast<uint8_t>(std::min(255.0f, std::round(static_cast<float>(r) * scalar))),
                static_cast<uint8_t>(std::min(255.0f, std::round(static_cast<float>(g) * scalar))),
                static_cast<uint8_t>(std::min(255.0f, std::round(static_cast<float>(b) * scalar))),
                static_cast<uint8_t>(std::min(255.0f, std::round(static_cast<float>(a) * scalar)))
            );
        }

        Color Color::operator/(float scalar) const {
            if (scalar == 0.0f) return *this;
            return Color(
                static_cast<uint8_t>(std::min(255.0f, std::round(static_cast<float>(r) / scalar))),
                static_cast<uint8_t>(std::min(255.0f, std::round(static_cast<float>(g) / scalar))),
                static_cast<uint8_t>(std::min(255.0f, std::round(static_cast<float>(b) / scalar))),
                static_cast<uint8_t>(std::min(255.0f, std::round(static_cast<float>(a) / scalar)))
            );
        }

        Color& Color::operator+=(const Color& other) {
            *this = *this + other;
            return *this;
        }

        Color& Color::operator-=(const Color& other) {
            *this = *this - other;
            return *this;
        }

        Color& Color::operator*=(float scalar) {
            *this = *this * scalar;
            return *this;
        }

        Color& Color::operator/=(float scalar) {
            *this = *this / scalar;
            return *this;
        }

        std::string Color::toHex(bool withAlpha) const {
            std::stringstream ss;
            ss << "#" << std::hex << std::setfill('0')
               << std::setw(2) << static_cast<int>(r)
               << std::setw(2) << static_cast<int>(g)
               << std::setw(2) << static_cast<int>(b);
            if (withAlpha) {
                ss << std::setw(2) << static_cast<int>(a);
            }
            return ss.str();
        }

        Color Color::fromHex(const std::string& hex) {
            std::string h = hex;
            if (h.size() > 0 && h[0] == '#') {
                h = h.substr(1);
            }
            for (auto& c : h) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

            auto hexToVal = [](char c) -> uint8_t {
                if (c >= '0' && c <= '9') return static_cast<uint8_t>(c - '0');
                if (c >= 'A' && c <= 'F') return static_cast<uint8_t>(c - 'A' + 10);
                return 0;
            };

            if (h.size() >= 6) {
                uint8_t red = static_cast<uint8_t>((hexToVal(h[0]) << 4) | hexToVal(h[1]));
                uint8_t green = static_cast<uint8_t>((hexToVal(h[2]) << 4) | hexToVal(h[3]));
                uint8_t blue = static_cast<uint8_t>((hexToVal(h[4]) << 4) | hexToVal(h[5]));
                uint8_t alpha = 255;
                if (h.size() >= 8) {
                    alpha = static_cast<uint8_t>((hexToVal(h[6]) << 4) | hexToVal(h[7]));
                }
                return Color(red, green, blue, alpha);
            }
            if (h.size() >= 3) {
                uint8_t red = static_cast<uint8_t>((hexToVal(h[0]) << 4) | hexToVal(h[0]));
                uint8_t green = static_cast<uint8_t>((hexToVal(h[1]) << 4) | hexToVal(h[1]));
                uint8_t blue = static_cast<uint8_t>((hexToVal(h[2]) << 4) | hexToVal(h[2]));
                return Color(red, green, blue);
            }
            return White;
        }

        sf::Color Color::toSFML() const {
            return sf::Color(r, g, b, a);
        }

        Color Color::fromSFML(const sf::Color& color) {
            return Color(color.r, color.g, color.b, color.a);
        }

        Color Color::lerp(const Color& a, const Color& b, float t) {
            t = std::max(0.0f, std::min(1.0f, t));
            return Color(
                static_cast<uint8_t>(std::round(static_cast<float>(a.r) + (static_cast<float>(b.r) - static_cast<float>(a.r)) * t)),
                static_cast<uint8_t>(std::round(static_cast<float>(a.g) + (static_cast<float>(b.g) - static_cast<float>(a.g)) * t)),
                static_cast<uint8_t>(std::round(static_cast<float>(a.b) + (static_cast<float>(b.b) - static_cast<float>(a.b)) * t)),
                static_cast<uint8_t>(std::round(static_cast<float>(a.a) + (static_cast<float>(b.a) - static_cast<float>(a.a)) * t))
            );
        }

        Color Color::withAlpha(uint8_t alpha) const {
            return Color(r, g, b, alpha);
        }

        float Color::brightness() const {
            return 0.299f * static_cast<float>(r) / 255.0f +
                   0.587f * static_cast<float>(g) / 255.0f +
                   0.114f * static_cast<float>(b) / 255.0f;
        }

        Color Color::invert() const {
            return Color(
                static_cast<uint8_t>(255 - r),
                static_cast<uint8_t>(255 - g),
                static_cast<uint8_t>(255 - b),
                a
            );
        }

    }
}
