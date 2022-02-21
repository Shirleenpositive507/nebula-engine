#pragma once
#include <cstdint>
#include <random>
#include <limits>
#include "core/Types.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "MathUtil.h"

namespace nebula {

class Random {
public:
    Random() : m_rng(std::random_device{}()) {}

    explicit Random(u64 seed) : m_rng(static_cast<u32>(seed)) {}

    void setSeed(u64 seed) {
        m_rng.seed(static_cast<u32>(seed));
    }

    i32 nextInt() {
        return static_cast<i32>(m_rng());
    }

    i32 nextInt(i32 min, i32 max) {
        if (min >= max) return min;
        std::uniform_int_distribution<i32> dist(min, max);
        return dist(m_rng);
    }

    u32 nextUInt() {
        return m_rng();
    }

    u32 nextUInt(u32 min, u32 max) {
        if (min >= max) return min;
        std::uniform_int_distribution<u32> dist(min, max);
        return dist(m_rng);
    }

    f32 nextFloat() {
        std::uniform_real_distribution<f32> dist(0.0f, 1.0f);
        return dist(m_rng);
    }

    f32 nextFloat(f32 min, f32 max) {
        if (min >= max) return min;
        std::uniform_real_distribution<f32> dist(min, max);
        return dist(m_rng);
    }

    f64 nextDouble() {
        std::uniform_real_distribution<f64> dist(0.0, 1.0);
        return dist(m_rng);
    }

    f64 nextDouble(f64 min, f64 max) {
        if (min >= max) return min;
        std::uniform_real_distribution<f64> dist(min, max);
        return dist(m_rng);
    }

    bool nextBool() {
        return (m_rng() & 1) != 0;
    }

    bool nextBool(f32 probability) {
        return nextFloat() < probability;
    }

    Vector2f nextVector2() {
        return Vector2f(nextFloat(-1.0f, 1.0f), nextFloat(-1.0f, 1.0f));
    }

    Vector2f nextUnitVector2() {
        f32 angle = nextFloat(0.0f, Math::TWO_PI);
        return Vector2f(std::cos(angle), std::sin(angle));
    }

    Vector2f nextPointInCircle(f32 radius = 1.0f) {
        f32 angle = nextFloat(0.0f, Math::TWO_PI);
        f32 r = radius * std::sqrt(nextFloat());
        return Vector2f(r * std::cos(angle), r * std::sin(angle));
    }

    Vector2f nextPointInRect(const Vector2f& min, const Vector2f& max) {
        return Vector2f(
            nextFloat(min.x, max.x),
            nextFloat(min.y, max.y)
        );
    }

    Vector2f nextPointInRect(f32 left, f32 top, f32 right, f32 bottom) {
        return Vector2f(nextFloat(left, right), nextFloat(top, bottom));
    }

    Vector3f nextVector3() {
        return Vector3f(
            nextFloat(-1.0f, 1.0f),
            nextFloat(-1.0f, 1.0f),
            nextFloat(-1.0f, 1.0f)
        );
    }

    Vector3f nextUnitVector3() {
        f32 theta = nextFloat(0.0f, Math::TWO_PI);
        f32 phi = std::acos(nextFloat(-1.0f, 1.0f));
        return Vector3f(
            std::sin(phi) * std::cos(theta),
            std::sin(phi) * std::sin(theta),
            std::cos(phi)
        );
    }

    Vector3f nextPointInSphere(f32 radius = 1.0f) {
        return nextUnitVector3() * radius * std::pow(nextFloat(), 1.0f / 3.0f);
    }

    Vector4f nextColor() {
        return Vector4f(nextFloat(), nextFloat(), nextFloat(), 1.0f);
    }

    Vector4f nextColorRGBA() {
        return Vector4f(nextFloat(), nextFloat(), nextFloat(), nextFloat());
    }

    Vector4f nextColorHSLA() {
        f32 hue = nextFloat(0.0f, 360.0f);
        f32 saturation = nextFloat(0.0f, 1.0f);
        f32 lightness = nextFloat(0.0f, 1.0f);
        return hslaToRgba(hue, saturation, lightness, 1.0f);
    }

    f64 gaussian(f64 mean = 0.0, f64 stddev = 1.0) {
        std::normal_distribution<f64> dist(mean, stddev);
        return dist(m_rng);
    }

    f32 gaussianFloat(f32 mean = 0.0f, f32 stddev = 1.0f) {
        return static_cast<f32>(gaussian(static_cast<f64>(mean), static_cast<f64>(stddev)));
    }

    f32 perlinNoise2D(f32 x, f32 y) const {
        i32 xi = static_cast<i32>(std::floor(x)) & 255;
        i32 yi = static_cast<i32>(std::floor(y)) & 255;

        f32 xf = x - std::floor(x);
        f32 yf = y - std::floor(y);

        f32 u = xf * xf * (3.0f - 2.0f * xf);
        f32 v = yf * yf * (3.0f - 2.0f * yf);

        i32 aa = m_perm[m_perm[xi] + yi];
        i32 ab = m_perm[m_perm[xi] + yi + 1];
        i32 ba = m_perm[m_perm[xi + 1] + yi];
        i32 bb = m_perm[m_perm[xi + 1] + yi + 1];

        f32 x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1.0f, yf), u);
        f32 x2 = lerp(grad(ab, xf, yf - 1.0f), grad(bb, xf - 1.0f, yf - 1.0f), u);

        return (lerp(x1, x2, v) + 1.0f) * 0.5f;
    }

    f32 perlinNoise2D(f32 x, f32 y, i32 octaves, f32 persistence = 0.5f) {
        f32 total = 0.0f;
        f32 frequency = 1.0f;
        f32 amplitude = 1.0f;
        f32 maxAmplitude = 0.0f;

        for (i32 i = 0; i < octaves; ++i) {
            total += perlinNoise2D(x * frequency, y * frequency) * amplitude;
            maxAmplitude += amplitude;
            amplitude *= persistence;
            frequency *= 2.0f;
        }

        return total / maxAmplitude;
    }

    void shuffle(auto& container) {
        std::shuffle(container.begin(), container.end(), m_rng);
    }

    template<typename T>
    T choose(const std::vector<T>& items) {
        if (items.empty()) return T{};
        return items[nextInt(0, static_cast<i32>(items.size()) - 1)];
    }

private:
    std::mt19937 m_rng;

    static constexpr i32 PERM_SIZE = 512;

    i32 m_perm[PERM_SIZE] = {
        151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,
        140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,
        247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,
        57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,
        74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,
        60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,
        65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,
        200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,
        52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,
        207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,
        119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,
        129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,
        218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,
        81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,
        184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,
        222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
    };

    static f32 grad(i32 hash, f32 x, f32 y) {
        i32 h = hash & 3;
        f32 u = (h < 2) ? x : y;
        f32 v = (h < 2) ? y : x;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

    static f32 lerp(f32 a, f32 b, f32 t) {
        return a + t * (b - a);
    }

    static Vector4f hslaToRgba(f32 hue, f32 saturation, f32 lightness, f32 alpha) {
        f32 c = (1.0f - std::abs(2.0f * lightness - 1.0f)) * saturation;
        f32 x = c * (1.0f - std::abs(std::fmod(hue / 60.0f, 2.0f) - 1.0f));
        f32 m = lightness - c / 2.0f;

        f32 r, g, b;
        if (hue < 60) { r = c; g = x; b = 0; }
        else if (hue < 120) { r = x; g = c; b = 0; }
        else if (hue < 180) { r = 0; g = c; b = x; }
        else if (hue < 240) { r = 0; g = x; b = c; }
        else if (hue < 300) { r = x; g = 0; b = c; }
        else { r = c; g = 0; b = x; }

        return Vector4f(r + m, g + m, b + m, alpha);
    }
};

} // namespace nebula
