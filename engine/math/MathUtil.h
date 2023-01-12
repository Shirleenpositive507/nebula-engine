#pragma once
#include <cmath>
#include <algorithm>
#include <limits>
#include <type_traits>
#include "core/Types.h"

namespace nebula {

namespace Math {

    constexpr f32 PI = 3.14159265358979323846f;
    constexpr f64 PI_D = 3.14159265358979323846;
    constexpr f32 TWO_PI = 2.0f * PI;
    constexpr f32 HALF_PI = PI / 2.0f;
    constexpr f32 QUARTER_PI = PI / 4.0f;
    constexpr f32 E = 2.71828182845904523536f;
    constexpr f32 EPSILON = 1e-6f;
    constexpr f64 EPSILON_D = 1e-12;

    constexpr f32 DEG2RAD = PI / 180.0f;
    constexpr f32 RAD2DEG = 180.0f / PI;

    inline f32 degToRad(f32 degrees) { return degrees * DEG2RAD; }
    inline f64 degToRad(f64 degrees) { return degrees * (PI_D / 180.0); }
    inline f32 radToDeg(f32 radians) { return radians * RAD2DEG; }
    inline f64 radToDeg(f64 radians) { return radians * (180.0 / PI_D); }

    template<typename T>
    constexpr T min(T a, T b) { return (a < b) ? a : b; }

    template<typename T>
    constexpr T max(T a, T b) { return (a > b) ? a : b; }

    template<typename T>
    constexpr T clamp(T value, T minVal, T maxVal) {
        return (value < minVal) ? minVal : ((value > maxVal) ? maxVal : value);
    }

    template<typename T>
    constexpr T lerp(T a, T b, T t) {
        return a + (b - a) * t;
    }

    template<typename T>
    constexpr T smoothstep(T edge0, T edge1, T x) {
        T t = clamp((x - edge0) / (edge1 - edge0), static_cast<T>(0), static_cast<T>(1));
        return t * t * (static_cast<T>(3) - static_cast<T>(2) * t);
    }

    template<typename T>
    constexpr T smootherstep(T edge0, T edge1, T x) {
        T t = clamp((x - edge0) / (edge1 - edge0), static_cast<T>(0), static_cast<T>(1));
        return t * t * t * (t * (t * static_cast<T>(6) - static_cast<T>(15)) + static_cast<T>(10));
    }

    template<typename T>
    constexpr T map(T value, T inMin, T inMax, T outMin, T outMax) {
        return outMin + (value - inMin) * (outMax - outMin) / (inMax - inMin);
    }

    template<typename T>
    constexpr bool isPowerOfTwo(T value) {
        return value > 0 && (value & (value - 1)) == 0;
    }

    template<typename T>
    constexpr T nextPowerOfTwo(T value) {
        if (value <= 1) return static_cast<T>(1);
        T power = 1;
        while (power < value) power <<= 1;
        return power;
    }

    template<typename T>
    constexpr T sign(T value) {
        return (value > static_cast<T>(0)) ? static_cast<T>(1)
             : (value < static_cast<T>(0)) ? static_cast<T>(-1)
             : static_cast<T>(0);
    }

    template<typename T>
    constexpr T abs(T value) {
        return (value < static_cast<T>(0)) ? -value : value;
    }

    template<typename T>
    T floor(T value) {
        return std::floor(value);
    }

    template<typename T>
    T ceil(T value) {
        return std::ceil(value);
    }

    template<typename T>
    T round(T value) {
        return std::round(value);
    }

    template<typename T>
    T fract(T value) {
        return value - std::floor(value);
    }

    inline f32 wrapAngle(f32 angle) {
        angle = std::fmod(angle, TWO_PI);
        if (angle < 0) angle += TWO_PI;
        return angle;
    }

    inline f32 wrapAngleDeg(f32 angle) {
        angle = std::fmod(angle, 360.0f);
        if (angle < 0) angle += 360.0f;
        return angle;
    }

    template<typename T>
    T approach(T current, T target, T maxDelta) {
        T diff = target - current;
        if (abs(diff) <= maxDelta) return target;
        return current + sign(diff) * maxDelta;
    }

    template<typename T>
    T moveToward(T current, T target, T maxDelta) {
        return approach(current, target, maxDelta);
    }

    template<typename T>
    constexpr T easeIn(T t) {
        return t * t;
    }

    template<typename T>
    constexpr T easeOut(T t) {
        return static_cast<T>(1) - (static_cast<T>(1) - t) * (static_cast<T>(1) - t);
    }

    template<typename T>
    constexpr T easeInOut(T t) {
        return (t < static_cast<T>(0.5))
            ? static_cast<T>(2) * t * t
            : static_cast<T>(1) - static_cast<T>(-2) * t * static_cast<T>(1) * (static_cast<T>(1) - t);
    }

    template<typename T>
    bool almostEqual(T a, T b, T epsilon = static_cast<T>(1e-6)) {
        if constexpr (std::is_floating_point_v<T>) {
            return abs(a - b) <= epsilon;
        } else {
            return a == b;
        }
    }

    template<typename T>
    bool almostZero(T value, T epsilon = static_cast<T>(1e-6)) {
        return abs(value) <= epsilon;
    }

    inline f32 normalizeAngle(f32 angle) {
        return wrapAngle(angle);
    }

    inline f32 deltaAngle(f32 from, f32 to) {
        f32 diff = to - from;
        while (diff > PI) diff -= TWO_PI;
        while (diff < -PI) diff += TWO_PI;
        return diff;
    }

    template<typename T>
    T lerpAngle(T from, T to, T t) {
        return from + deltaAngle(static_cast<f32>(from), static_cast<f32>(to)) * t;
    }

    template<typename T>
    T inverseLerp(T a, T b, T value) {
        T denom = b - a;
        if (abs(denom) < static_cast<T>(1e-10)) return static_cast<T>(0);
        return (value - a) / denom;
    }

    template<typename T>
    T remap(T value, T inMin, T inMax, T outMin, T outMax) {
        return map(value, inMin, inMax, outMin, outMax);
    }

    template<typename T>
    T saturate(T value) {
        return clamp(value, static_cast<T>(0), static_cast<T>(1));
    }

    template<typename T>
    T gaussian(T x, T mean = static_cast<T>(0), T sigma = static_cast<T>(1)) {
        T diff = x - mean;
        return std::exp(-(diff * diff) / (static_cast<T>(2) * sigma * sigma));
    }

    template<typename T>
    T sigmoid(T x, T gain = static_cast<T>(1)) {
        return static_cast<T>(1) / (static_cast<T>(1) + std::exp(-x * gain));
    }

    template<typename T>
    T bezier(T p0, T p1, T p2, T p3, T t) {
        T u = static_cast<T>(1) - t;
        T uu = u * u;
        T uuu = uu * u;
        T tt = t * t;
        T ttt = tt * t;
        return uuu * p0 + static_cast<T>(3) * uu * t * p1 + static_cast<T>(3) * u * tt * p2 + ttt * p3;
    }

    template<typename T>
    T catmullRom(T p0, T p1, T p2, T p3, T t) {
        T tt = t * t;
        T ttt = tt * t;
        return static_cast<T>(0.5) * (
            (static_cast<T>(2) * p1) +
            (-p0 + p2) * t +
            (static_cast<T>(2) * p0 - static_cast<T>(5) * p1 + static_cast<T>(4) * p2 - p3) * tt +
            (-p0 + static_cast<T>(3) * p1 - static_cast<T>(3) * p2 + p3) * ttt
        );
    }

    template<typename T>
    T damp(T current, T target, T smoothTime, T deltaTime, T& velocity) {
        T omega = static_cast<T>(2) / smoothTime;
        T x = omega * deltaTime;
        T exp = static_cast<T>(1) / (static_cast<T>(1) + x + static_cast<T>(0.48) * x * x + static_cast<T>(0.235) * x * x * x);
        T change = current - target;
        T temp = (velocity + omega * change) * deltaTime;
        velocity = (velocity - omega * temp) * exp;
        return target + (change + temp) * exp;
    }

    template<typename T>
    T smoothDamp(T current, T target, T& currentVelocity, T smoothTime, T maxSpeed, T deltaTime) {
        T omega = static_cast<T>(2) / smoothTime;
        T x = omega * deltaTime;
        T exp = static_cast<T>(1) / (static_cast<T>(1) + x + static_cast<T>(0.48) * x * x + static_cast<T>(0.235) * x * x * x);
        T change = current - target;
        T maxChange = maxSpeed * smoothTime;
        change = clamp(change, -maxChange, maxChange);
        T targetTo = target;
        T temp = (currentVelocity + omega * change) * deltaTime;
        currentVelocity = (currentVelocity - omega * temp) * exp;
        return targetTo + (change + temp) * exp;
    }

    template<typename T>
    T repeat(T t, T length) {
        return t - std::floor(t / length) * length;
    }

    template<typename T>
    T pingPong(T t, T length) {
        t = repeat(t, length * static_cast<T>(2));
        return length - std::abs(t - length);
    }

} // namespace Math

} // namespace nebula
