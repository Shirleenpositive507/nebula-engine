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

    // 3D Perlin noise
    f32 perlinNoise3D(f32 x, f32 y, f32 z) const {
        i32 xi = static_cast<i32>(std::floor(x)) & 255;
        i32 yi = static_cast<i32>(std::floor(y)) & 255;
        i32 zi = static_cast<i32>(std::floor(z)) & 255;

        f32 xf = x - std::floor(x);
        f32 yf = y - std::floor(y);
        f32 zf = z - std::floor(z);

        f32 u = xf * xf * (3.0f - 2.0f * xf);
        f32 v = yf * yf * (3.0f - 2.0f * yf);
        f32 w = zf * zf * (3.0f - 2.0f * zf);

        i32 a = m_perm[xi] + yi;
        i32 aa = m_perm[a] + zi;
        i32 ab = m_perm[a + 1] + zi;
        i32 b = m_perm[xi + 1] + yi;
        i32 ba = m_perm[b] + zi;
        i32 bb = m_perm[b + 1] + zi;

        f32 x1 = lerp(grad3D(aa, xf, yf, zf), grad3D(ba, xf - 1, yf, zf), u);
        f32 x2 = lerp(grad3D(ab, xf, yf - 1, zf), grad3D(bb, xf - 1, yf - 1, zf), u);
        f32 y1 = lerp(x1, x2, v);
        x1 = lerp(grad3D(aa + 1, xf, yf, zf - 1), grad3D(ba + 1, xf - 1, yf, zf - 1), u);
        x2 = lerp(grad3D(ab + 1, xf, yf - 1, zf - 1), grad3D(bb + 1, xf - 1, yf - 1, zf - 1), u);
        f32 y2 = lerp(x1, x2, v);

        return (lerp(y1, y2, w) + 1.0f) * 0.5f;
    }

    f32 perlinNoise3D(f32 x, f32 y, f32 z, i32 octaves, f32 persistence = 0.5f) {
        f32 total = 0.0f;
        f32 frequency = 1.0f;
        f32 amplitude = 1.0f;
        f32 maxAmplitude = 0.0f;
        for (i32 i = 0; i < octaves; ++i) {
            total += perlinNoise3D(x * frequency, y * frequency, z * frequency) * amplitude;
            maxAmplitude += amplitude;
            amplitude *= persistence;
            frequency *= 2.0f;
        }
        return total / maxAmplitude;
    }

    // 2D simplex noise
    f32 simplexNoise2D(f32 x, f32 y) const {
        const f32 F2 = 0.5f * (std::sqrt(3.0f) - 1.0f);
        const f32 G2 = (3.0f - std::sqrt(3.0f)) / 6.0f;

        f32 s = (x + y) * F2;
        i32 i = fastFloor(x + s);
        i32 j = fastFloor(y + s);
        f32 t = static_cast<f32>(i + j) * G2;
        f32 X0 = i - t;
        f32 Y0 = j - t;
        f32 x0 = x - X0;
        f32 y0 = y - Y0;

        i32 i1, j1;
        if (x0 > y0) { i1 = 1; j1 = 0; }
        else { i1 = 0; j1 = 1; }

        f32 x1 = x0 - i1 + G2;
        f32 y1 = y0 - j1 + G2;
        f32 x2 = x0 - 1.0f + 2.0f * G2;
        f32 y2 = y0 - 1.0f + 2.0f * G2;

        i32 ii = i & 255;
        i32 jj = j & 255;
        i32 gi0 = m_perm[ii + m_perm[jj]] % 12;
        i32 gi1 = m_perm[ii + i1 + m_perm[jj + j1]] % 12;
        i32 gi2 = m_perm[ii + 1 + m_perm[jj + 1]] % 12;

        f32 n0 = 0.0f, n1 = 0.0f, n2 = 0.0f;
        f32 t0 = 0.5f - x0 * x0 - y0 * y0;
        if (t0 > 0) { t0 *= t0; n0 = t0 * t0 * dot2Grad(gi0, x0, y0); }
        f32 t1 = 0.5f - x1 * x1 - y1 * y1;
        if (t1 > 0) { t1 *= t1; n1 = t1 * t1 * dot2Grad(gi1, x1, y1); }
        f32 t2 = 0.5f - x2 * x2 - y2 * y2;
        if (t2 > 0) { t2 *= t2; n2 = t2 * t2 * dot2Grad(gi2, x2, y2); }

        return 70.0f * (n0 + n1 + n2);
    }

    // 3D simplex noise
    f32 simplexNoise3D(f32 x, f32 y, f32 z) const {
        const f32 F3 = 1.0f / 3.0f;
        const f32 G3 = 1.0f / 6.0f;

        f32 s = (x + y + z) * F3;
        i32 i = fastFloor(x + s);
        i32 j = fastFloor(y + s);
        i32 k = fastFloor(z + s);
        f32 t = static_cast<f32>(i + j + k) * G3;
        f32 X0 = i - t;
        f32 Y0 = j - t;
        f32 Z0 = k - t;
        f32 x0 = x - X0;
        f32 y0 = y - Y0;
        f32 z0 = z - Z0;

        i32 i1, j1, k1, i2, j2, k2;
        if (x0 >= y0) {
            if (y0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; }
            else if (x0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; }
            else { i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; }
        } else {
            if (y0 < z0) { i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; }
            else if (x0 < z0) { i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; }
            else { i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; }
        }

        f32 x1 = x0 - i1 + G3;
        f32 y1 = y0 - j1 + G3;
        f32 z1 = z0 - k1 + G3;
        f32 x2 = x0 - i2 + 2.0f * G3;
        f32 y2 = y0 - j2 + 2.0f * G3;
        f32 z2 = z0 - k2 + 2.0f * G3;
        f32 x3 = x0 - 1.0f + 3.0f * G3;
        f32 y3 = y0 - 1.0f + 3.0f * G3;
        f32 z3 = z0 - 1.0f + 3.0f * G3;

        i32 ii = i & 255;
        i32 jj = j & 255;
        i32 kk = k & 255;
        i32 gi0 = m_perm[ii + m_perm[jj + m_perm[kk]]] % 12;
        i32 gi1 = m_perm[ii + i1 + m_perm[jj + j1 + m_perm[kk + k1]]] % 12;
        i32 gi2 = m_perm[ii + i2 + m_perm[jj + j2 + m_perm[kk + k2]]] % 12;
        i32 gi3 = m_perm[ii + 1 + m_perm[jj + 1 + m_perm[kk + 1]]] % 12;

        f32 n0 = 0.0f, n1 = 0.0f, n2 = 0.0f, n3 = 0.0f;
        f32 t0 = 0.6f - x0 * x0 - y0 * y0 - z0 * z0;
        if (t0 > 0) { t0 *= t0; n0 = t0 * t0 * dot3Grad(gi0, x0, y0, z0); }
        f32 t1 = 0.6f - x1 * x1 - y1 * y1 - z1 * z1;
        if (t1 > 0) { t1 *= t1; n1 = t1 * t1 * dot3Grad(gi1, x1, y1, z1); }
        f32 t2 = 0.6f - x2 * x2 - y2 * y2 - z2 * z2;
        if (t2 > 0) { t2 *= t2; n2 = t2 * t2 * dot3Grad(gi2, x2, y2, z2); }
        f32 t3 = 0.6f - x3 * x3 - y3 * y3 - z3 * z3;
        if (t3 > 0) { t3 *= t3; n3 = t3 * t3 * dot3Grad(gi3, x3, y3, z3); }

        return 32.0f * (n0 + n1 + n2 + n3);
    }

    f32 fBm2D(f32 x, f32 y, i32 octaves, f32 lacunarity = 2.0f, f32 gain = 0.5f) const {
        f32 value = 0.0f;
        f32 amplitude = 1.0f;
        f32 frequency = 1.0f;
        f32 maxValue = 0.0f;
        for (i32 i = 0; i < octaves; ++i) {
            value += simplexNoise2D(x * frequency, y * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= gain;
            frequency *= lacunarity;
        }
        return value / maxValue;
    }

    f32 fBm3D(f32 x, f32 y, f32 z, i32 octaves, f32 lacunarity = 2.0f, f32 gain = 0.5f) const {
        f32 value = 0.0f;
        f32 amplitude = 1.0f;
        f32 frequency = 1.0f;
        f32 maxValue = 0.0f;
        for (i32 i = 0; i < octaves; ++i) {
            value += simplexNoise3D(x * frequency, y * frequency, z * frequency) * amplitude;
            maxValue += amplitude;
            amplitude *= gain;
            frequency *= lacunarity;
        }
        return value / maxValue;
    }

    // Voronoi (Worley) noise
    f32 voronoi2D(f32 x, f32 y, f32 frequency = 1.0f) const {
        x *= frequency; y *= frequency;
        i32 ix = fastFloor(x); i32 iy = fastFloor(y);
        f32 fx = x - ix; f32 fy = y - iy;

        f32 minDist = 8.0f;
        for (i32 ox = -1; ox <= 1; ++ox) {
            for (i32 oy = -1; oy <= 1; ++oy) {
                i32 cx = ix + ox; i32 cy = iy + oy;
                i32 h = hash2(cx, cy);
                f32 vx = ox + (h & 255) / 255.0f;
                f32 vy = oy + ((h >> 8) & 255) / 255.0f;
                f32 dx = vx - fx;
                f32 dy = vy - fy;
                f32 d = dx * dx + dy * dy;
                if (d < minDist) minDist = d;
            }
        }
        return std::sqrt(minDist);
    }

    f32 voronoi3D(f32 x, f32 y, f32 z, f32 frequency = 1.0f) const {
        x *= frequency; y *= frequency; z *= frequency;
        i32 ix = fastFloor(x); i32 iy = fastFloor(y); i32 iz = fastFloor(z);
        f32 fx = x - ix; f32 fy = y - iy; f32 fz = z - iz;

        f32 minDist = 8.0f;
        for (i32 ox = -1; ox <= 1; ++ox) {
            for (i32 oy = -1; oy <= 1; ++oy) {
                for (i32 oz = -1; oz <= 1; ++oz) {
                    i32 cx = ix + ox; i32 cy = iy + oy; i32 cz = iz + oz;
                    i32 h = hash3(cx, cy, cz);
                    f32 vx = ox + (h & 255) / 255.0f;
                    f32 vy = oy + ((h >> 8) & 255) / 255.0f;
                    f32 vz = oz + ((h >> 16) & 255) / 255.0f;
                    f32 dx = vx - fx;
                    f32 dy = vy - fy;
                    f32 dz = vz - fz;
                    f32 d = dx * dx + dy * dy + dz * dz;
                    if (d < minDist) minDist = d;
                }
            }
        }
        return std::sqrt(minDist);
    }

    // Seeded deterministic random sequence
    u32 seededRandom(i32 seed, i32 index) const {
        i32 h = seed * 374761393 + index * 668265263;
        h = (h ^ (h >> 13)) * 1274126177;
        return h ^ (h >> 16);
    }

    f32 seededRandomFloat(i32 seed, i32 index) const {
        return (seededRandom(seed, index) & 0x7FFFFFFF) / 2147483648.0f;
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

    static i32 fastFloor(f32 x) {
        return x > 0 ? static_cast<i32>(x) : static_cast<i32>(x) - 1;
    }

    static i32 hash2(i32 x, i32 y) {
        i32 h = x * 374761393 + y * 668265263;
        h = (h ^ (h >> 13)) * 1274126177;
        return h ^ (h >> 16);
    }

    static i32 hash3(i32 x, i32 y, i32 z) {
        i32 h = x * 374761393 + y * 668265263 + z * 955166017;
        h = (h ^ (h >> 13)) * 1274126177;
        return h ^ (h >> 16);
    }

    static f32 grad(i32 hash, f32 x, f32 y) {
        i32 h = hash & 3;
        f32 u = (h < 2) ? x : y;
        f32 v = (h < 2) ? y : x;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

    static f32 grad3D(i32 hash, f32 x, f32 y, f32 z) {
        i32 h = hash & 15;
        f32 u = (h < 8) ? x : y;
        f32 v = (h < 4) ? y : ((h == 12 || h == 14) ? x : z);
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }

    static f32 dot2Grad(i32 hash, f32 x, f32 y) {
        switch (hash & 7) {
            case 0: return x + y;
            case 1: return -x + y;
            case 2: return x - y;
            case 3: return -x - y;
            case 4: return x;
            case 5: return -x;
            case 6: return y;
            case 7: return -y;
            default: return 0;
        }
    }

    static f32 dot3Grad(i32 hash, f32 x, f32 y, f32 z) {
        switch (hash & 15) {
            case 0: return x + y;
            case 1: return -x + y;
            case 2: return x - y;
            case 3: return -x - y;
            case 4: return x + z;
            case 5: return -x + z;
            case 6: return x - z;
            case 7: return -x - z;
            case 8: return y + z;
            case 9: return -y + z;
            case 10: return y - z;
            case 11: return -y - z;
            case 12: return x + y;
            case 13: return -x + y;
            case 14: return -y + z;
            case 15: return -x - z;
            default: return 0;
        }
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
