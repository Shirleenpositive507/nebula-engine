#pragma once
#include <cmath>
#include <iostream>
#include "core/Types.h"
#include "Vector2.h"

namespace nebula {

template<typename T>
class Vector3 {
public:
    T x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(T x, T y, T z) : x(x), y(y), z(z) {}
    explicit Vector3(T value) : x(value), y(value), z(value) {}
    Vector3(const Vector2<T>& xy, T z) : x(xy.x), y(xy.y), z(z) {}

    template<typename U>
    explicit Vector3(const Vector3<U>& other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)), z(static_cast<T>(other.z)) {}

    Vector2<T> xy() const { return Vector2<T>(x, y); }
    void setXY(const Vector2<T>& v) { x = v.x; y = v.y; }

    Vector3 operator+(const Vector3& other) const { return Vector3(x + other.x, y + other.y, z + other.z); }
    Vector3 operator-(const Vector3& other) const { return Vector3(x - other.x, y - other.y, z - other.z); }
    Vector3 operator*(T scalar) const { return Vector3(x * scalar, y * scalar, z * scalar); }
    Vector3 operator/(T scalar) const { return Vector3(x / scalar, y / scalar, z / scalar); }
    Vector3 operator*(const Vector3& other) const { return Vector3(x * other.x, y * other.y, z * other.z); }
    Vector3 operator/(const Vector3& other) const { return Vector3(x / other.x, y / other.y, z / other.z); }

    Vector3& operator+=(const Vector3& other) { x += other.x; y += other.y; z += other.z; return *this; }
    Vector3& operator-=(const Vector3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
    Vector3& operator*=(T scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
    Vector3& operator/=(T scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }

    Vector3 operator-() const { return Vector3(-x, -y, -z); }

    bool operator==(const Vector3& other) const { return x == other.x && y == other.y && z == other.z; }
    bool operator!=(const Vector3& other) const { return x != other.x || y != other.y || z != other.z; }

    T dot(const Vector3& other) const { return x * other.x + y * other.y + z * other.z; }

    // cross product with SSE-friendly ordering hints
    Vector3 cross(const Vector3& other) const {
        return Vector3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    Vector3 crossFast(const Vector3& other) const {
        // Optimized cross with reduced dependency chain
        T cx = y * other.z - z * other.y;
        T cy = z * other.x - x * other.z;
        T cz = x * other.y - y * other.x;
        return Vector3(cx, cy, cz);
    }

    T lengthSquared() const { return x * x + y * y + z * z; }
    T length() const { return std::sqrt(lengthSquared()); }

    Vector3 normalized() const {
        T len = length();
        if (len < static_cast<T>(1e-8)) return Vector3::Zero;
        return *this / len;
    }

    void normalize() {
        T len = length();
        if (len < static_cast<T>(1e-8)) { x = 0; y = 0; z = 0; return; }
        x /= len; y /= len; z /= len;
    }

    T angle(const Vector3& other) const {
        T d = dot(other);
        T lenProd = length() * other.length();
        if (lenProd < static_cast<T>(1e-8)) return 0;
        return std::acos(d / lenProd);
    }

    T distance(const Vector3& other) const { return (*this - other).length(); }
    T distanceSquared(const Vector3& other) const { return (*this - other).lengthSquared(); }

    Vector3 reflect(const Vector3& normal) const {
        return *this - normal * static_cast<T>(2) * dot(normal);
    }

    Vector3 refract(const Vector3& normal, T eta) const {
        T d = dot(normal);
        T k = static_cast<T>(1) - eta * eta * (static_cast<T>(1) - d * d);
        if (k < static_cast<T>(0)) return Vector3::Zero;
        return *this * eta - normal * (eta * d + std::sqrt(k));
    }

    Vector3 project(const Vector3& onto) const {
        T d = onto.dot(onto);
        if (d < static_cast<T>(1e-8)) return Vector3::Zero;
        return onto * (dot(onto) / d);
    }

    Vector3 projectOnPlane(const Vector3& planeNormal) const {
        return *this - planeNormal * dot(planeNormal);
    }

    static Vector3 lerp(const Vector3& a, const Vector3& b, T t) {
        return a + (b - a) * t;
    }

    static Vector3 lerpUnclamped(const Vector3& a, const Vector3& b, T t) {
        return a + (b - a) * t;
    }

    static Vector3 nlerp(const Vector3& a, const Vector3& b, T t) {
        return lerp(a, b, t).normalized();
    }

    static Vector3 slerp(const Vector3& a, const Vector3& b, T t) {
        T dotProduct = a.dot(b);
        dotProduct = std::max(static_cast<T>(-1), std::min(static_cast<T>(1), dotProduct));
        T theta = std::acos(dotProduct) * t;
        Vector3 relative = (b - a * dotProduct).normalized();
        return a * std::cos(theta) + relative * std::sin(theta);
    }

    friend std::ostream& operator<<(std::ostream& os, const Vector3& v) {
        os << "Vector3(" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

    static const Vector3 Zero;
    static const Vector3 One;
    static const Vector3 Up;
    static const Vector3 Down;
    static const Vector3 Left;
    static const Vector3 Right;
    static const Vector3 Forward;
    static const Vector3 Backward;
};

template<typename T>
const Vector3<T> Vector3<T>::Zero = Vector3<T>(0, 0, 0);

template<typename T>
const Vector3<T> Vector3<T>::One = Vector3<T>(1, 1, 1);

template<typename T>
const Vector3<T> Vector3<T>::Up = Vector3<T>(0, 1, 0);

template<typename T>
const Vector3<T> Vector3<T>::Down = Vector3<T>(0, -1, 0);

template<typename T>
const Vector3<T> Vector3<T>::Left = Vector3<T>(-1, 0, 0);

template<typename T>
const Vector3<T> Vector3<T>::Right = Vector3<T>(1, 0, 0);

template<typename T>
const Vector3<T> Vector3<T>::Forward = Vector3<T>(0, 0, -1);

template<typename T>
const Vector3<T> Vector3<T>::Backward = Vector3<T>(0, 0, 1);

template<typename T>
Vector3<T> operator*(T scalar, const Vector3<T>& vec) {
    return vec * scalar;
}

using Vector3f = Vector3<f32>;
using Vector3i = Vector3<i32>;
using Vector3u = Vector3<u32>;
using Vector3d = Vector3<f64>;

} // namespace nebula
