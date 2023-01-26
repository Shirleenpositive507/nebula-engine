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

        float Color::luminance() const {
            float rNorm = static_cast<float>(r) / 255.0f;
            float gNorm = static_cast<float>(g) / 255.0f;
            float bNorm = static_cast<float>(b) / 255.0f;

            auto linearize = [](float c) {
                return (c <= 0.04045f) ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
            };

            return 0.2126f * linearize(rNorm) + 0.7152f * linearize(gNorm) + 0.0722f * linearize(bNorm);
        }

        HSL Color::toHSL() const {
            float rNorm = static_cast<float>(r) / 255.0f;
            float gNorm = static_cast<float>(g) / 255.0f;
            float bNorm = static_cast<float>(b) / 255.0f;

            float max = std::max({rNorm, gNorm, bNorm});
            float min = std::min({rNorm, gNorm, bNorm});
            float diff = max - min;

            HSL hsl;
            hsl.l = (max + min) / 2.0f;
            hsl.a = static_cast<float>(a) / 255.0f;

            if (diff < 0.0001f) {
                hsl.h = 0;
                hsl.s = 0;
            } else {
                hsl.s = hsl.l > 0.5f ? diff / (2.0f - max - min) : diff / (max + min);

                if (max == rNorm) {
                    hsl.h = std::fmod((gNorm - bNorm) / diff + (gNorm < bNorm ? 6.0f : 0.0f), 6.0f);
                } else if (max == gNorm) {
                    hsl.h = (bNorm - rNorm) / diff + 2.0f;
                } else {
                    hsl.h = (rNorm - gNorm) / diff + 4.0f;
                }
                hsl.h *= 60.0f;
            }
            return hsl;
        }

        HSV Color::toHSV() const {
            float rNorm = static_cast<float>(r) / 255.0f;
            float gNorm = static_cast<float>(g) / 255.0f;
            float bNorm = static_cast<float>(b) / 255.0f;

            float max = std::max({rNorm, gNorm, bNorm});
            float min = std::min({rNorm, gNorm, bNorm});
            float diff = max - min;

            HSV hsv;
            hsv.v = max;
            hsv.a = static_cast<float>(a) / 255.0f;

            if (diff < 0.0001f) {
                hsv.h = 0;
                hsv.s = 0;
            } else {
                hsv.s = diff / max;
                if (max == rNorm) {
                    hsv.h = std::fmod((gNorm - bNorm) / diff + (gNorm < bNorm ? 6.0f : 0.0f), 6.0f);
                } else if (max == gNorm) {
                    hsv.h = (bNorm - rNorm) / diff + 2.0f;
                } else {
                    hsv.h = (rNorm - gNorm) / diff + 4.0f;
                }
                hsv.h *= 60.0f;
            }
            return hsv;
        }

        Color Color::fromHSL(const HSL& hsl) {
            if (hsl.s < 0.0001f) {
                uint8_t v = static_cast<uint8_t>(std::round(hsl.l * 255.0f));
                return Color(v, v, v, static_cast<uint8_t>(std::round(hsl.a * 255.0f)));
            }

            float h = hsl.h / 60.0f;
            float c = (1.0f - std::abs(2.0f * hsl.l - 1.0f)) * hsl.s;
            float x = c * (1.0f - std::abs(std::fmod(h, 2.0f) - 1.0f));
            float m = hsl.l - c / 2.0f;

            float r, g, b;
            if (h < 1) { r = c; g = x; b = 0; }
            else if (h < 2) { r = x; g = c; b = 0; }
            else if (h < 3) { r = 0; g = c; b = x; }
            else if (h < 4) { r = 0; g = x; b = c; }
            else if (h < 5) { r = x; g = 0; b = c; }
            else { r = c; g = 0; b = x; }

            return Color(
                static_cast<uint8_t>(std::round((r + m) * 255.0f)),
                static_cast<uint8_t>(std::round((g + m) * 255.0f)),
                static_cast<uint8_t>(std::round((b + m) * 255.0f)),
                static_cast<uint8_t>(std::round(hsl.a * 255.0f))
            );
        }

        Color Color::fromHSV(const HSV& hsv) {
            if (hsv.s < 0.0001f) {
                uint8_t v = static_cast<uint8_t>(std::round(hsv.v * 255.0f));
                return Color(v, v, v, static_cast<uint8_t>(std::round(hsv.a * 255.0f)));
            }

            float h = hsv.h / 60.0f;
            float c = hsv.v * hsv.s;
            float x = c * (1.0f - std::abs(std::fmod(h, 2.0f) - 1.0f));
            float m = hsv.v - c;

            float r, g, b;
            if (h < 1) { r = c; g = x; b = 0; }
            else if (h < 2) { r = x; g = c; b = 0; }
            else if (h < 3) { r = 0; g = c; b = x; }
            else if (h < 4) { r = 0; g = x; b = c; }
            else if (h < 5) { r = x; g = 0; b = c; }
            else { r = c; g = 0; b = x; }

            return Color(
                static_cast<uint8_t>(std::round((r + m) * 255.0f)),
                static_cast<uint8_t>(std::round((g + m) * 255.0f)),
                static_cast<uint8_t>(std::round((b + m) * 255.0f)),
                static_cast<uint8_t>(std::round(hsv.a * 255.0f))
            );
        }

        Color Color::blend(const Color& other, BlendOp op) const {
            return blend(*this, other, op);
        }

        Color Color::blend(const Color& a, const Color& b, BlendOp op) {
            float ar = static_cast<float>(a.r) / 255.0f;
            float ag = static_cast<float>(a.g) / 255.0f;
            float ab = static_cast<float>(a.b) / 255.0f;
            float br = static_cast<float>(b.r) / 255.0f;
            float bg = static_cast<float>(b.g) / 255.0f;
            float bb = static_cast<float>(b.b) / 255.0f;

            float rr, rg, rb;
            switch (op) {
                case BlendOp::Multiply:
                    rr = ar * br; rg = ag * bg; rb = ab * bb;
                    break;
                case BlendOp::Screen:
                    rr = 1.0f - (1.0f - ar) * (1.0f - br);
                    rg = 1.0f - (1.0f - ag) * (1.0f - bg);
                    rb = 1.0f - (1.0f - ab) * (1.0f - bb);
                    break;
                case BlendOp::Overlay: {
                    auto overlay = [](float base, float blend) {
                        return (base < 0.5f) ? 2.0f * base * blend : 1.0f - 2.0f * (1.0f - base) * (1.0f - blend);
                    };
                    rr = overlay(ar, br); rg = overlay(ag, bg); rb = overlay(ab, bb);
                    break;
                }
                case BlendOp::Add:
                    rr = std::min(1.0f, ar + br);
                    rg = std::min(1.0f, ag + bg);
                    rb = std::min(1.0f, ab + bb);
                    break;
                case BlendOp::Subtract:
                    rr = std::max(0.0f, ar - br);
                    rg = std::max(0.0f, ag - bg);
                    rb = std::max(0.0f, ab - bb);
                    break;
                default:
                    rr = br; rg = bg; rb = bb;
                    break;
            }

            return Color(
                static_cast<uint8_t>(std::round(rr * 255.0f)),
                static_cast<uint8_t>(std::round(rg * 255.0f)),
                static_cast<uint8_t>(std::round(rb * 255.0f)),
                a.a
            );
        }

        Color Color::multiply(const Color& other) const {
            return blend(other, BlendOp::Multiply);
        }

        Color Color::screen(const Color& other) const {
            return blend(other, BlendOp::Screen);
        }

        Color Color::overlay(const Color& other) const {
            return blend(other, BlendOp::Overlay);
        }

        // --- Gradient ---

        Gradient::Gradient() {}

        Gradient::Gradient(const std::vector<ColorStop>& stops) : m_stops(stops) {
            sortStops();
        }

        void Gradient::addStop(float position, const Color& color) {
            m_stops.push_back(ColorStop(position, color));
            sortStops();
        }

        void Gradient::removeStop(float position) {
            for (auto it = m_stops.begin(); it != m_stops.end(); ) {
                if (std::abs(it->position - position) < 0.001f) {
                    it = m_stops.erase(it);
                } else {
                    ++it;
                }
            }
        }

        void Gradient::clearStops() {
            m_stops.clear();
        }

        void Gradient::sortStops() {
            std::sort(m_stops.begin(), m_stops.end(),
                [](const ColorStop& a, const ColorStop& b) {
                    return a.position < b.position;
                });
        }

        Color Gradient::evaluate(float t) const {
            if (m_stops.empty()) return Color::Black;
            if (m_stops.size() == 1) return m_stops[0].color;

            t = std::max(0.0f, std::min(1.0f, t));

            if (t <= m_stops.front().position) return m_stops.front().color;
            if (t >= m_stops.back().position) return m_stops.back().color;

            for (std::size_t i = 0; i < m_stops.size() - 1; ++i) {
                if (t >= m_stops[i].position && t <= m_stops[i + 1].position) {
                    float localT = (t - m_stops[i].position) / (m_stops[i + 1].position - m_stops[i].position);
                    return Color::lerp(m_stops[i].color, m_stops[i + 1].color, localT);
                }
            }
            return m_stops.back().color;
        }

        std::vector<Color> Gradient::evaluateMany(const std::vector<float>& positions) const {
            std::vector<Color> colors;
            colors.reserve(positions.size());
            for (float p : positions) {
                colors.push_back(evaluate(p));
            }
            return colors;
        }

        std::size_t Gradient::getNumStops() const { return m_stops.size(); }
        const std::vector<ColorStop>& Gradient::getStops() const { return m_stops; }

    }
}
