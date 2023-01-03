#pragma once
#include <cmath>
#include <iostream>
#include "core/Types.h"

namespace nebula {

template<typename T>
class Vector2 {
public:
    T x, y;

    Vector2() : x(0), y(0) {}
    Vector2(T x, T y) : x(x), y(y) {}
    explicit Vector2(T value) : x(value), y(value) {}

    template<typename U>
    explicit Vector2(const Vector2<U>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}

    Vector2 operator+(const Vector2& other) const { return Vector2(x + other.x, y + other.y); }
    Vector2 operator-(const Vector2& other) const { return Vector2(x - other.x, y - other.y); }
    Vector2 operator*(T scalar) const { return Vector2(x * scalar, y * scalar); }
    Vector2 operator/(T scalar) const { return Vector2(x / scalar, y / scalar); }
    Vector2 operator*(const Vector2& other) const { return Vector2(x * other.x, y * other.y); }
    Vector2 operator/(const Vector2& other) const { return Vector2(x / other.x, y / other.y); }

    Vector2& operator+=(const Vector2& other) { x += other.x; y += other.y; return *this; }
    Vector2& operator-=(const Vector2& other) { x -= other.x; y -= other.y; return *this; }
    Vector2& operator*=(T scalar) { x *= scalar; y *= scalar; return *this; }
    Vector2& operator/=(T scalar) { x /= scalar; y /= scalar; return *this; }

    Vector2 operator-() const { return Vector2(-x, -y); }

    bool operator==(const Vector2& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vector2& other) const { return x != other.x || y != other.y; }

    T dot(const Vector2& other) const { return x * other.x + y * other.y; }
    T cross(const Vector2& other) const { return x * other.y - y * other.x; }
    T lengthSquared() const { return x * x + y * y; }
    T length() const { return std::sqrt(lengthSquared()); }

    Vector2 normalized() const {
        T len = length();
        if (len < static_cast<T>(1e-8)) return Vector2::Zero;
        return *this / len;
    }

    void normalize() {
        T len = length();
        if (len < static_cast<T>(1e-8)) { x = 0; y = 0; return; }
        x /= len; y /= len;
    }

    Vector2 rotated(T angleRad) const {
        T c = std::cos(angleRad);
        T s = std::sin(angleRad);
        return Vector2(x * c - y * s, x * s + y * c);
    }

    void rotate(T angleRad) {
        T c = std::cos(angleRad);
        T s = std::sin(angleRad);
        T nx = x * c - y * s;
        y = x * s + y * c;
        x = nx;
    }

    T angle(const Vector2& other) const {
        T d = dot(other);
        T lenProd = length() * other.length();
        if (lenProd < static_cast<T>(1e-8)) return 0;
        return std::acos(d / lenProd);
    }

    static Vector2 lerp(const Vector2& a, const Vector2& b, T t) {
        return a + (b - a) * t;
    }

    T distance(const Vector2& other) const {
        return (*this - other).length();
    }

    T distanceSquared(const Vector2& other) const {
        return (*this - other).lengthSquared();
    }

    Vector2 reflect(const Vector2& normal) const {
        return *this - normal * static_cast<T>(2) * dot(normal);
    }

    Vector2 project(const Vector2& onto) const {
        T d = onto.dot(onto);
        if (d < static_cast<T>(1e-8)) return Vector2::Zero;
        return onto * (dot(onto) / d);
    }

    Vector2 perpendicular() const {
        return Vector2(-y, x);
    }

    Vector2 swizzleXX() const { return Vector2(x, x); }
    Vector2 swizzleXY() const { return Vector2(x, y); }
    Vector2 swizzleYX() const { return Vector2(y, x); }
    Vector2 swizzleYY() const { return Vector2(y, y); }

    Vector2 reject(const Vector2& onto) const {
        return *this - project(onto);
    }

    T angleBetween(const Vector2& other) const {
        T d = dot(other);
        T lenProd = length() * other.length();
        if (lenProd < static_cast<T>(1e-8)) return 0;
        return std::acos(d / lenProd);
    }

    bool isUnit(T epsilon = static_cast<T>(1e-6)) const {
        return std::abs(lengthSquared() - static_cast<T>(1)) < epsilon;
    }

    bool isZero(T epsilon = static_cast<T>(1e-8)) const {
        return std::abs(x) < epsilon && std::abs(y) < epsilon;
    }

    friend std::ostream& operator<<(std::ostream& os, const Vector2& v) {
        os << "Vector2(" << v.x << ", " << v.y << ")";
        return os;
    }

    static const Vector2 Zero;
    static const Vector2 One;
    static const Vector2 Up;
    static const Vector2 Down;
    static const Vector2 Left;
    static const Vector2 Right;
};

template<typename T>
const Vector2<T> Vector2<T>::Zero = Vector2<T>(0, 0);

template<typename T>
const Vector2<T> Vector2<T>::One = Vector2<T>(1, 1);

template<typename T>
const Vector2<T> Vector2<T>::Up = Vector2<T>(0, 1);

template<typename T>
const Vector2<T> Vector2<T>::Down = Vector2<T>(0, -1);

template<typename T>
const Vector2<T> Vector2<T>::Left = Vector2<T>(-1, 0);

template<typename T>
const Vector2<T> Vector2<T>::Right = Vector2<T>(1, 0);

template<typename T>
Vector2<T> operator*(T scalar, const Vector2<T>& vec) {
    return vec * scalar;
}

using Vector2f = Vector2<f32>;
using Vector2i = Vector2<i32>;
using Vector2u = Vector2<u32>;
using Vector2d = Vector2<f64>;

} // namespace nebula
