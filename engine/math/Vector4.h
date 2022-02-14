#pragma once
#include <cmath>
#include <iostream>
#include "core/Types.h"
#include "Vector3.h"

namespace nebula {

template<typename T>
class Vector4 {
public:
    T x, y, z, w;

    Vector4() : x(0), y(0), z(0), w(0) {}
    Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
    explicit Vector4(T value) : x(value), y(value), z(value), w(value) {}
    Vector4(const Vector3<T>& xyz, T w) : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}

    template<typename U>
    explicit Vector4(const Vector4<U>& other)
        : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)),
          z(static_cast<T>(other.z)), w(static_cast<T>(other.w)) {}

    Vector3<T> xyz() const { return Vector3<T>(x, y, z); }
    void setXYZ(const Vector3<T>& v) { x = v.x; y = v.y; z = v.z; }

    Vector4 operator+(const Vector4& other) const { return Vector4(x + other.x, y + other.y, z + other.z, w + other.w); }
    Vector4 operator-(const Vector4& other) const { return Vector4(x - other.x, y - other.y, z - other.z, w - other.w); }
    Vector4 operator*(T scalar) const { return Vector4(x * scalar, y * scalar, z * scalar, w * scalar); }
    Vector4 operator/(T scalar) const { return Vector4(x / scalar, y / scalar, z / scalar, w / scalar); }
    Vector4 operator*(const Vector4& other) const { return Vector4(x * other.x, y * other.y, z * other.z, w * other.w); }
    Vector4 operator/(const Vector4& other) const { return Vector4(x / other.x, y / other.y, z / other.z, w / other.w); }

    Vector4& operator+=(const Vector4& other) { x += other.x; y += other.y; z += other.z; w += other.w; return *this; }
    Vector4& operator-=(const Vector4& other) { x -= other.x; y -= other.y; z -= other.z; w -= other.w; return *this; }
    Vector4& operator*=(T scalar) { x *= scalar; y *= scalar; z *= scalar; w *= scalar; return *this; }
    Vector4& operator/=(T scalar) { x /= scalar; y /= scalar; z /= scalar; w /= scalar; return *this; }

    Vector4 operator-() const { return Vector4(-x, -y, -z, -w); }

    bool operator==(const Vector4& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
    bool operator!=(const Vector4& other) const { return x != other.x || y != other.y || z != other.z || w != other.w; }

    T dot(const Vector4& other) const { return x * other.x + y * other.y + z * other.z + w * other.w; }
    T lengthSquared() const { return x * x + y * y + z * z + w * w; }
    T length() const { return std::sqrt(lengthSquared()); }

    Vector4 normalized() const {
        T len = length();
        if (len < static_cast<T>(1e-8)) return Vector4::Zero;
        return *this / len;
    }

    void normalize() {
        T len = length();
        if (len < static_cast<T>(1e-8)) { x = 0; y = 0; z = 0; w = 0; return; }
        x /= len; y /= len; z /= len; w /= len;
    }

    Vector4 toHomogeneous() const {
        if (std::abs(w) < static_cast<T>(1e-8)) return Vector4::Zero;
        return Vector4(x / w, y / w, z / w, static_cast<T>(1));
    }

    Vector3<T> toVec3() const { return Vector3<T>(x, y, z); }

    static Vector4 lerp(const Vector4& a, const Vector4& b, T t) {
        return a + (b - a) * t;
    }

    T distance(const Vector4& other) const { return (*this - other).length(); }

    friend std::ostream& operator<<(std::ostream& os, const Vector4& v) {
        os << "Vector4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return os;
    }

    static const Vector4 Zero;
    static const Vector4 One;
};

template<typename T>
const Vector4<T> Vector4<T>::Zero = Vector4<T>(0, 0, 0, 0);

template<typename T>
const Vector4<T> Vector4<T>::One = Vector4<T>(1, 1, 1, 1);

template<typename T>
Vector4<T> operator*(T scalar, const Vector4<T>& vec) {
    return vec * scalar;
}

using Vector4f = Vector4<f32>;
using Vector4i = Vector4<i32>;
using Vector4u = Vector4<u32>;
using Vector4d = Vector4<f64>;

} // namespace nebula
