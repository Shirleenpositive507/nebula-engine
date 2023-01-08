#pragma once
#include <cmath>
#include <iostream>
#include "core/Types.h"
#include "Vector3.h"
#include "Matrix3.h"
#include "Matrix4.h"

namespace nebula {

template<typename T>
class Quaternion {
public:
    T w, x, y, z;

    Quaternion() : w(1), x(0), y(0), z(0) {}
    Quaternion(T w, T x, T y, T z) : w(w), x(x), y(y), z(z) {}

    static Quaternion lookRotation(const Vector3<T>& forward, const Vector3<T>& up = Vector3<T>::Up) {
        Vector3<T> f = forward.normalized();
        Vector3<T> u = up.normalized();
        Vector3<T> r = u.cross(f).normalized();
        u = f.cross(r);

        Matrix3<T> rot(
            r.x, u.x, f.x,
            r.y, u.y, f.y,
            r.z, u.z, f.z
        );
        return fromRotationMatrix(rot);
    }

    static Quaternion fromToRotation(const Vector3<T>& from, const Vector3<T>& to) {
        Vector3<T> f = from.normalized();
        Vector3<T> t = to.normalized();
        T dot = f.dot(t);

        if (dot > static_cast<T>(0.99999)) {
            return Quaternion::Identity;
        }
        if (dot < static_cast<T>(-0.99999)) {
            Vector3<T> axis = Vector3<T>::Right.cross(f);
            if (axis.lengthSquared() < static_cast<T>(1e-6)) {
                axis = Vector3<T>::Up.cross(f);
            }
            axis.normalize();
            return Quaternion(0, axis.x, axis.y, axis.z);
        }

        Vector3<T> axis2 = f.cross(t);
        T w = std::sqrt(f.lengthSquared() * t.lengthSquared()) + dot;
        return Quaternion(w, axis2.x, axis2.y, axis2.z).normalized();
    }

    Vector3<T> getAxis() const {
        T s = static_cast<T>(1) - w * w;
        if (s < static_cast<T>(1e-8)) return Vector3<T>::Forward;
        T invS = static_cast<T>(1) / std::sqrt(s);
        return Vector3<T>(x * invS, y * invS, z * invS);
    }

    T getAngle() const {
        T cw = w < static_cast<T>(-1) ? static_cast<T>(-1) : (w > static_cast<T>(1) ? static_cast<T>(1) : w);
        return static_cast<T>(2) * std::acos(cw);
    }

    Quaternion exponential() const {
        T halfAngle = std::sqrt(x * x + y * y + z * z);
        if (halfAngle < static_cast<T>(1e-8)) return Quaternion::Identity;
        T sinHalf = std::sin(halfAngle);
        T factor = sinHalf / halfAngle;
        return Quaternion(std::cos(halfAngle), x * factor, y * factor, z * factor);
    }

    Quaternion logarithmic() const {
        T cw = w < static_cast<T>(-1) ? static_cast<T>(-1) : (w > static_cast<T>(1) ? static_cast<T>(1) : w);
        T angle = std::acos(cw);
        T sinAngle = std::sin(angle);
        if (sinAngle < static_cast<T>(1e-8)) return Quaternion(0, 0, 0, 0);
        T factor = angle / sinAngle;
        return Quaternion(0, x * factor, y * factor, z * factor);
    }

    T angularDistance(const Quaternion& other) const {
        T d = w * other.w + x * other.x + y * other.y + z * other.z;
        d = d < static_cast<T>(-1) ? static_cast<T>(-1) : (d > static_cast<T>(1) ? static_cast<T>(1) : d);
        return std::acos(d) * static_cast<T>(2);
    }

    static Quaternion fromAngleAxis(T angleRad, const Vector3<T>& axis) {
        Vector3<T> a = axis.normalized();
        T halfAngle = angleRad * static_cast<T>(0.5);
        T s = std::sin(halfAngle);
        return Quaternion(std::cos(halfAngle), a.x * s, a.y * s, a.z * s);
    }

    static Quaternion fromEulerAngles(const Vector3<T>& eulerRad) {
        T cy = std::cos(eulerRad.z * static_cast<T>(0.5));
        T sy = std::sin(eulerRad.z * static_cast<T>(0.5));
        T cp = std::cos(eulerRad.y * static_cast<T>(0.5));
        T sp = std::sin(eulerRad.y * static_cast<T>(0.5));
        T cr = std::cos(eulerRad.x * static_cast<T>(0.5));
        T sr = std::sin(eulerRad.x * static_cast<T>(0.5));

        return Quaternion(
            cr * cp * cy + sr * sp * sy,
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy
        );
    }

    static Quaternion fromRotationMatrix(const Matrix3<T>& m) {
        T trace = m(0, 0) + m(1, 1) + m(2, 2);
        if (trace > static_cast<T>(0)) {
            T s = std::sqrt(trace + static_cast<T>(1)) * static_cast<T>(2);
            return Quaternion(
                s * static_cast<T>(0.25),
                (m(2, 1) - m(1, 2)) / s,
                (m(0, 2) - m(2, 0)) / s,
                (m(1, 0) - m(0, 1)) / s
            );
        } else if (m(0, 0) > m(1, 1) && m(0, 0) > m(2, 2)) {
            T s = std::sqrt(static_cast<T>(1) + m(0, 0) - m(1, 1) - m(2, 2)) * static_cast<T>(2);
            return Quaternion(
                (m(2, 1) - m(1, 2)) / s,
                s * static_cast<T>(0.25),
                (m(0, 1) + m(1, 0)) / s,
                (m(0, 2) + m(2, 0)) / s
            );
        } else if (m(1, 1) > m(2, 2)) {
            T s = std::sqrt(static_cast<T>(1) + m(1, 1) - m(0, 0) - m(2, 2)) * static_cast<T>(2);
            return Quaternion(
                (m(0, 2) - m(2, 0)) / s,
                (m(0, 1) + m(1, 0)) / s,
                s * static_cast<T>(0.25),
                (m(1, 2) + m(2, 1)) / s
            );
        } else {
            T s = std::sqrt(static_cast<T>(1) + m(2, 2) - m(0, 0) - m(1, 1)) * static_cast<T>(2);
            return Quaternion(
                (m(1, 0) - m(0, 1)) / s,
                (m(0, 2) + m(2, 0)) / s,
                (m(1, 2) + m(2, 1)) / s,
                s * static_cast<T>(0.25)
            );
        }
    }

    T lengthSquared() const { return w * w + x * x + y * y + z * z; }
    T length() const { return std::sqrt(lengthSquared()); }

    Quaternion normalized() const {
        T len = length();
        if (len < static_cast<T>(1e-8)) return Quaternion::Identity;
        return Quaternion(w / len, x / len, y / len, z / len);
    }

    void normalize() {
        T len = length();
        if (len < static_cast<T>(1e-8)) { w = 1; x = 0; y = 0; z = 0; return; }
        w /= len; x /= len; y /= len; z /= len;
    }

    Quaternion conjugated() const { return Quaternion(w, -x, -y, -z); }
    void conjugate() { x = -x; y = -y; z = -z; }

    Quaternion inverse() const { return conjugated() / lengthSquared(); }

    Quaternion operator*(const Quaternion& other) const {
        return Quaternion(
            w * other.w - x * other.x - y * other.y - z * other.z,
            w * other.x + x * other.w + y * other.z - z * other.y,
            w * other.y - x * other.z + y * other.w + z * other.x,
            w * other.z + x * other.y - y * other.x + z * other.w
        );
    }

    Quaternion operator*(T scalar) const {
        return Quaternion(w * scalar, x * scalar, y * scalar, z * scalar);
    }

    Quaternion operator+(const Quaternion& other) const {
        return Quaternion(w + other.w, x + other.x, y + other.y, z + other.z);
    }

    Quaternion operator-(const Quaternion& other) const {
        return Quaternion(w - other.w, x - other.x, y - other.y, z - other.z);
    }

    Quaternion operator/(T scalar) const {
        return Quaternion(w / scalar, x / scalar, y / scalar, z / scalar);
    }

    Quaternion& operator*=(const Quaternion& other) { *this = *this * other; return *this; }
    Quaternion& operator+=(const Quaternion& other) { w += other.w; x += other.x; y += other.y; z += other.z; return *this; }
    Quaternion& operator*=(T scalar) { w *= scalar; x *= scalar; y *= scalar; z *= scalar; return *this; }

    bool operator==(const Quaternion& other) const {
        return w == other.w && x == other.x && y == other.y && z == other.z;
    }
    bool operator!=(const Quaternion& other) const { return !(*this == other); }

    Vector3<T> rotateVector(const Vector3<T>& vec) const {
        Quaternion p(0, vec.x, vec.y, vec.z);
        Quaternion qInv = inverse();
        Quaternion rotated = *this * p * qInv;
        return Vector3<T>(rotated.x, rotated.y, rotated.z);
    }

    static Quaternion lerp(const Quaternion& a, const Quaternion& b, T t) {
        return (a * (static_cast<T>(1) - t) + b * t).normalized();
    }

    static Quaternion slerp(const Quaternion& a, const Quaternion& b, T t) {
        T dot = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
        Quaternion qb = b;

        if (dot < 0) {
            qb = -qb;
            dot = -dot;
        }

        const T DOT_THRESHOLD = static_cast<T>(0.9995);
        if (dot > DOT_THRESHOLD) {
            return lerp(a, qb, t);
        }

        T theta = std::acos(dot);
        T sinTheta = std::sin(theta);
        T scaleA = std::sin((static_cast<T>(1) - t) * theta) / sinTheta;
        T scaleB = std::sin(t * theta) / sinTheta;

        return Quaternion(
            scaleA * a.w + scaleB * qb.w,
            scaleA * a.x + scaleB * qb.x,
            scaleA * a.y + scaleB * qb.y,
            scaleA * a.z + scaleB * qb.z
        );
    }

    Vector3<T> toEulerAngles() const {
        T sinr_cosp = static_cast<T>(2) * (w * x + y * z);
        T cosr_cosp = static_cast<T>(1) - static_cast<T>(2) * (x * x + y * y);
        T roll = std::atan2(sinr_cosp, cosr_cosp);

        T sinp = static_cast<T>(2) * (w * y - z * x);
        T pitch;
        if (std::abs(sinp) >= 1)
            pitch = std::copysign(static_cast<T>(3.14159265358979323846) / static_cast<T>(2), sinp);
        else
            pitch = std::asin(sinp);

        T siny_cosp = static_cast<T>(2) * (w * z + x * y);
        T cosy_cosp = static_cast<T>(1) - static_cast<T>(2) * (y * y + z * z);
        T yaw = std::atan2(siny_cosp, cosy_cosp);

        return Vector3<T>(roll, pitch, yaw);
    }

    Matrix3<T> toRotationMatrix() const {
        T tx = x + x, ty = y + y, tz = z + z;
        T twx = w * tx, twy = w * ty, twz = w * tz;
        T txx = x * tx, txy = x * ty, txz = x * tz;
        T tyy = y * ty, tyz = y * tz, tzz = z * tz;

        return Matrix3<T>(
            static_cast<T>(1) - (tyy + tzz), txy - twz, txz + twy,
            txy + twz, static_cast<T>(1) - (txx + tzz), tyz - twx,
            txz - twy, tyz + twx, static_cast<T>(1) - (txx + tyy)
        );
    }

    Matrix4<T> toMatrix4() const {
        Matrix3<T> rot = toRotationMatrix();
        return Matrix4<T>(
            rot(0, 0), rot(0, 1), rot(0, 2), 0,
            rot(1, 0), rot(1, 1), rot(1, 2), 0,
            rot(2, 0), rot(2, 1), rot(2, 2), 0,
            0, 0, 0, 1
        );
    }

    T angle(const Quaternion& other) const {
        T d = w * other.w + x * other.x + y * other.y + z * other.z;
        T v = d * d * static_cast<T>(2) - static_cast<T>(1);
        v = v < static_cast<T>(-1) ? static_cast<T>(-1) : (v > static_cast<T>(1) ? static_cast<T>(1) : v);
        return std::acos(v);
    }

    Quaternion operator-() const { return Quaternion(-w, -x, -y, -z); }

    friend std::ostream& operator<<(std::ostream& os, const Quaternion& q) {
        os << "Quaternion(" << q.w << ", " << q.x << ", " << q.y << ", " << q.z << ")";
        return os;
    }

    static const Quaternion Identity;
};

template<typename T>
const Quaternion<T> Quaternion<T>::Identity = Quaternion<T>();

template<typename T>
Quaternion<T> operator*(T scalar, const Quaternion<T>& q) {
    return q * scalar;
}

using Quaternionf = Quaternion<f32>;
using Quaterniond = Quaternion<f64>;

} // namespace nebula
