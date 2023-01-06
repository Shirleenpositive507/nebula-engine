#pragma once
#include <cmath>
#include <cstring>
#include <iostream>
#include "core/Types.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix3.h"

namespace nebula {

template<typename T>
struct Plane {
    Vector3<T> normal;
    T distance;

    Plane() : normal(), distance(0) {}
    Plane(const Vector3<T>& n, T d) : normal(n), distance(d) {}
};

template<typename T>
struct TRSDecomposition {
    Vector3<T> position;
    Vector3<T> scale;
    Vector3<T> eulerAngles;
    // Rotation matrix extracted
    Matrix3<T> rotationMatrix;
};

template<typename T>
class Matrix4 {
public:
    T data[16];

    Matrix4() { setIdentity(); }
    explicit Matrix4(T value) {
        for (int i = 0; i < 16; ++i) data[i] = value;
    }
    Matrix4(T m00, T m01, T m02, T m03,
            T m10, T m11, T m12, T m13,
            T m20, T m21, T m22, T m23,
            T m30, T m31, T m32, T m33) {
        data[0]  = m00; data[1]  = m01; data[2]  = m02; data[3]  = m03;
        data[4]  = m10; data[5]  = m11; data[6]  = m12; data[7]  = m13;
        data[8]  = m20; data[9]  = m21; data[10] = m22; data[11] = m23;
        data[12] = m30; data[13] = m31; data[14] = m32; data[15] = m33;
    }

    T& operator()(int row, int col) { return data[row * 4 + col]; }
    const T& operator()(int row, int col) const { return data[row * 4 + col]; }

    T& operator[](int index) { return data[index]; }
    const T& operator[](int index) const { return data[index]; }

    Matrix4 operator+(const Matrix4& other) const {
        Matrix4 result;
        for (int i = 0; i < 16; ++i) result.data[i] = data[i] + other.data[i];
        return result;
    }
    Matrix4 operator-(const Matrix4& other) const {
        Matrix4 result;
        for (int i = 0; i < 16; ++i) result.data[i] = data[i] - other.data[i];
        return result;
    }
    Matrix4 operator*(T scalar) const {
        Matrix4 result;
        for (int i = 0; i < 16; ++i) result.data[i] = data[i] * scalar;
        return result;
    }
    Matrix4 operator*(const Matrix4& other) const {
        Matrix4 result(static_cast<T>(0));
        for (int row = 0; row < 4; ++row) {
            for (int col = 0; col < 4; ++col) {
                for (int k = 0; k < 4; ++k) {
                    result(row, col) += (*this)(row, k) * other(k, col);
                }
            }
        }
        return result;
    }
    Vector4<T> operator*(const Vector4<T>& vec) const {
        return Vector4<T>(
            data[0] * vec.x + data[1] * vec.y + data[2] * vec.z + data[3] * vec.w,
            data[4] * vec.x + data[5] * vec.y + data[6] * vec.z + data[7] * vec.w,
            data[8] * vec.x + data[9] * vec.y + data[10] * vec.z + data[11] * vec.w,
            data[12] * vec.x + data[13] * vec.y + data[14] * vec.z + data[15] * vec.w
        );
    }

    Vector3<T> transformPoint(const Vector3<T>& point) const {
        Vector4<T> result = *this * Vector4<T>(point.x, point.y, point.z, static_cast<T>(1));
        return Vector3<T>(result.x, result.y, result.z);
    }

    Vector3<T> transformDirection(const Vector3<T>& dir) const {
        Vector4<T> result = *this * Vector4<T>(dir.x, dir.y, dir.z, static_cast<T>(0));
        return Vector3<T>(result.x, result.y, result.z).normalized();
    }

    Matrix4& operator+=(const Matrix4& other) { for (int i = 0; i < 16; ++i) data[i] += other.data[i]; return *this; }
    Matrix4& operator-=(const Matrix4& other) { for (int i = 0; i < 16; ++i) data[i] -= other.data[i]; return *this; }
    Matrix4& operator*=(T scalar) { for (int i = 0; i < 16; ++i) data[i] *= scalar; return *this; }

    bool operator==(const Matrix4& other) const {
        for (int i = 0; i < 16; ++i)
            if (data[i] != other.data[i]) return false;
        return true;
    }
    bool operator!=(const Matrix4& other) const { return !(*this == other); }

    void setIdentity() {
        std::memset(data, 0, sizeof(data));
        data[0] = data[5] = data[10] = data[15] = static_cast<T>(1);
    }

    static Matrix4 identity() { return Matrix4(); }
    static Matrix4 zero() { return Matrix4(static_cast<T>(0)); }

    Matrix4 transposed() const {
        return Matrix4(
            data[0], data[4], data[8],  data[12],
            data[1], data[5], data[9],  data[13],
            data[2], data[6], data[10], data[14],
            data[3], data[7], data[11], data[15]
        );
    }

    void transpose() { *this = transposed(); }

    T determinant() const {
        T a00 = data[0], a01 = data[1], a02 = data[2], a03 = data[3];
        T a10 = data[4], a11 = data[5], a12 = data[6], a13 = data[7];
        T a20 = data[8], a21 = data[9], a22 = data[10], a23 = data[11];
        T a30 = data[12], a31 = data[13], a32 = data[14], a33 = data[15];

        T b00 = a00 * a11 - a01 * a10;
        T b01 = a00 * a12 - a02 * a10;
        T b02 = a00 * a13 - a03 * a10;
        T b03 = a01 * a12 - a02 * a11;
        T b04 = a01 * a13 - a03 * a11;
        T b05 = a02 * a13 - a03 * a12;
        T b06 = a20 * a31 - a21 * a30;
        T b07 = a20 * a32 - a22 * a30;
        T b08 = a20 * a33 - a23 * a30;
        T b09 = a21 * a32 - a22 * a31;
        T b10 = a21 * a33 - a23 * a31;
        T b11 = a22 * a33 - a23 * a32;

        return b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;
    }

    Matrix4 inverse() const {
        T det = determinant();
        if (std::abs(det) < static_cast<T>(1e-8)) return identity();

        T a00 = data[0], a01 = data[1], a02 = data[2], a03 = data[3];
        T a10 = data[4], a11 = data[5], a12 = data[6], a13 = data[7];
        T a20 = data[8], a21 = data[9], a22 = data[10], a23 = data[11];
        T a30 = data[12], a31 = data[13], a32 = data[14], a33 = data[15];

        T b00 = a00 * a11 - a01 * a10;
        T b01 = a00 * a12 - a02 * a10;
        T b02 = a00 * a13 - a03 * a10;
        T b03 = a01 * a12 - a02 * a11;
        T b04 = a01 * a13 - a03 * a11;
        T b05 = a02 * a13 - a03 * a12;
        T b06 = a20 * a31 - a21 * a30;
        T b07 = a20 * a32 - a22 * a30;
        T b08 = a20 * a33 - a23 * a30;
        T b09 = a21 * a32 - a22 * a31;
        T b10 = a21 * a33 - a23 * a31;
        T b11 = a22 * a33 - a23 * a32;

        T invDet = static_cast<T>(1) / det;

        return Matrix4(
            ( a11 * b11 - a12 * b10 + a13 * b09) * invDet,
            (-a01 * b11 + a02 * b10 - a03 * b09) * invDet,
            ( a31 * b05 - a32 * b04 + a33 * b03) * invDet,
            (-a21 * b05 + a22 * b04 - a23 * b03) * invDet,
            (-a10 * b11 + a12 * b08 - a13 * b07) * invDet,
            ( a00 * b11 - a02 * b08 + a03 * b07) * invDet,
            (-a30 * b05 + a32 * b02 - a33 * b01) * invDet,
            ( a20 * b05 - a22 * b02 + a23 * b01) * invDet,
            ( a10 * b10 - a11 * b08 + a13 * b06) * invDet,
            (-a00 * b10 + a01 * b08 - a03 * b06) * invDet,
            ( a30 * b04 - a31 * b02 + a33 * b00) * invDet,
            (-a20 * b04 + a21 * b02 - a23 * b00) * invDet,
            (-a10 * b09 + a11 * b07 - a12 * b06) * invDet,
            ( a00 * b09 - a01 * b07 + a02 * b06) * invDet,
            (-a30 * b03 + a31 * b01 - a32 * b00) * invDet,
            ( a20 * b03 - a21 * b01 + a22 * b00) * invDet
        );
    }

    static Matrix4 perspective(T fovRad, T aspect, T nearZ, T farZ) {
        T f = static_cast<T>(1) / std::tan(fovRad / static_cast<T>(2));
        T rangeInv = static_cast<T>(1) / (nearZ - farZ);
        return Matrix4(
            f / aspect, 0, 0, 0,
            0, f, 0, 0,
            0, 0, (farZ + nearZ) * rangeInv, 2 * farZ * nearZ * rangeInv,
            0, 0, -1, 0
        );
    }

    static Matrix4 orthographic(T left, T right, T bottom, T top, T nearZ, T farZ) {
        T rml = right - left;
        T tmb = top - bottom;
        T fmn = farZ - nearZ;
        return Matrix4(
            static_cast<T>(2) / rml, 0, 0, -(right + left) / rml,
            0, static_cast<T>(2) / tmb, 0, -(top + bottom) / tmb,
            0, 0, static_cast<T>(-2) / fmn, -(farZ + nearZ) / fmn,
            0, 0, 0, 1
        );
    }

    static Matrix4 lookAt(const Vector3<T>& eye, const Vector3<T>& center, const Vector3<T>& up) {
        Vector3<T> f = (center - eye).normalized();
        Vector3<T> s = f.cross(up).normalized();
        Vector3<T> u = s.cross(f);

        return Matrix4(
            s.x, s.y, s.z, -s.dot(eye),
            u.x, u.y, u.z, -u.dot(eye),
            -f.x, -f.y, -f.z, f.dot(eye),
            0, 0, 0, 1
        );
    }

    static Matrix4 rotationX(T angleRad) {
        T c = std::cos(angleRad);
        T s = std::sin(angleRad);
        return Matrix4(
            1, 0, 0, 0,
            0, c, -s, 0,
            0, s, c, 0,
            0, 0, 0, 1
        );
    }

    static Matrix4 rotationY(T angleRad) {
        T c = std::cos(angleRad);
        T s = std::sin(angleRad);
        return Matrix4(
            c, 0, s, 0,
            0, 1, 0, 0,
            -s, 0, c, 0,
            0, 0, 0, 1
        );
    }

    static Matrix4 rotationZ(T angleRad) {
        T c = std::cos(angleRad);
        T s = std::sin(angleRad);
        return Matrix4(
            c, -s, 0, 0,
            s, c, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    static Matrix4 rotation(T angleRad, const Vector3<T>& axis) {
        T c = std::cos(angleRad);
        T s = std::sin(angleRad);
        T oneMinusC = static_cast<T>(1) - c;
        Vector3<T> a = axis.normalized();

        return Matrix4(
            c + a.x * a.x * oneMinusC, a.x * a.y * oneMinusC - a.z * s, a.x * a.z * oneMinusC + a.y * s, 0,
            a.y * a.x * oneMinusC + a.z * s, c + a.y * a.y * oneMinusC, a.y * a.z * oneMinusC - a.x * s, 0,
            a.z * a.x * oneMinusC - a.y * s, a.z * a.y * oneMinusC + a.x * s, c + a.z * a.z * oneMinusC, 0,
            0, 0, 0, 1
        );
    }

    static Matrix4 scaling(T sx, T sy, T sz) {
        return Matrix4(
            sx, 0, 0, 0,
            0, sy, 0, 0,
            0, 0, sz, 0,
            0, 0, 0, 1
        );
    }

    static Matrix4 translation(T tx, T ty, T tz) {
        return Matrix4(
            1, 0, 0, tx,
            0, 1, 0, ty,
            0, 0, 1, tz,
            0, 0, 0, 1
        );
    }

    static Matrix4 TRS(const Vector3<T>& translation, const Vector3<T>& eulerRad, const Vector3<T>& scale) {
        Matrix4 t = translation(translation.x, translation.y, translation.z);
        Matrix4 rx = rotationX(eulerRad.x);
        Matrix4 ry = rotationY(eulerRad.y);
        Matrix4 rz = rotationZ(eulerRad.z);
        Matrix4 s = scaling(scale.x, scale.y, scale.z);
        return t * rz * ry * rx * s;
    }

    Vector3<T> getTranslation() const {
        return Vector3<T>(data[3], data[7], data[11]);
    }

    void setTranslation(const Vector3<T>& t) {
        data[3] = t.x; data[7] = t.y; data[11] = t.z;
    }

    Matrix3<T> getRotationMatrix() const {
        T invSx = static_cast<T>(1) / std::sqrt(data[0] * data[0] + data[1] * data[1] + data[2] * data[2]);
        T invSy = static_cast<T>(1) / std::sqrt(data[4] * data[4] + data[5] * data[5] + data[6] * data[6]);
        T invSz = static_cast<T>(1) / std::sqrt(data[8] * data[8] + data[9] * data[9] + data[10] * data[10]);
        return Matrix3<T>(
            data[0] * invSx, data[1] * invSx, data[2] * invSx,
            data[4] * invSy, data[5] * invSy, data[6] * invSy,
            data[8] * invSz, data[9] * invSz, data[10] * invSz
        );
    }

    Vector3<T> getScale() const {
        return Vector3<T>(
            std::sqrt(data[0] * data[0] + data[1] * data[1] + data[2] * data[2]),
            std::sqrt(data[4] * data[4] + data[5] * data[5] + data[6] * data[6]),
            std::sqrt(data[8] * data[8] + data[9] * data[9] + data[10] * data[10])
        );
    }

    // Frustum plane extraction from combined projection*view matrix
    // Returns 6 planes: left, right, bottom, top, near, far
    void extractPlanes(Plane<T> planes[6]) const {
        // Left
        planes[0].normal = Vector3<T>(
            data[3] + data[0],
            data[7] + data[4],
            data[11] + data[8]
        );
        T len0 = planes[0].normal.length();
        planes[0].distance = (data[15] + data[12]) / len0;
        planes[0].normal /= len0;

        // Right
        planes[1].normal = Vector3<T>(
            data[3] - data[0],
            data[7] - data[4],
            data[11] - data[8]
        );
        T len1 = planes[1].normal.length();
        planes[1].distance = (data[15] - data[12]) / len1;
        planes[1].normal /= len1;

        // Bottom
        planes[2].normal = Vector3<T>(
            data[3] + data[1],
            data[7] + data[5],
            data[11] + data[9]
        );
        T len2 = planes[2].normal.length();
        planes[2].distance = (data[15] + data[13]) / len2;
        planes[2].normal /= len2;

        // Top
        planes[3].normal = Vector3<T>(
            data[3] - data[1],
            data[7] - data[5],
            data[11] - data[9]
        );
        T len3 = planes[3].normal.length();
        planes[3].distance = (data[15] - data[13]) / len3;
        planes[3].normal /= len3;

        // Near
        planes[4].normal = Vector3<T>(
            data[3] + data[2],
            data[7] + data[6],
            data[11] + data[10]
        );
        T len4 = planes[4].normal.length();
        planes[4].distance = (data[15] + data[14]) / len4;
        planes[4].normal /= len4;

        // Far
        planes[5].normal = Vector3<T>(
            data[3] - data[2],
            data[7] - data[6],
            data[11] - data[10]
        );
        T len5 = planes[5].normal.length();
        planes[5].distance = (data[15] - data[14]) / len5;
        planes[5].normal /= len5;
    }

    // Decompose into position, scale, and rotation
    TRSDecomposition<T> decompose() const {
        TRSDecomposition<T> result;
        result.position = Vector3<T>(data[3], data[7], data[11]);

        Vector3<T> scale;
        scale.x = std::sqrt(data[0] * data[0] + data[1] * data[1] + data[2] * data[2]);
        scale.y = std::sqrt(data[4] * data[4] + data[5] * data[5] + data[6] * data[6]);
        scale.z = std::sqrt(data[8] * data[8] + data[9] * data[9] + data[10] * data[10]);
        result.scale = scale;

        T invSx = static_cast<T>(1) / scale.x;
        T invSy = static_cast<T>(1) / scale.y;
        T invSz = static_cast<T>(1) / scale.z;

        result.rotationMatrix = Matrix3<T>(
            data[0] * invSx, data[1] * invSx, data[2] * invSx,
            data[4] * invSy, data[5] * invSy, data[6] * invSy,
            data[8] * invSz, data[9] * invSz, data[10] * invSz
        );

        // Extract euler angles from rotation matrix (XYZ order)
        const Matrix3<T>& r = result.rotationMatrix;
        T sy = std::sqrt(r(0,0) * r(0,0) + r(1,0) * r(1,0));
        bool singular = sy < static_cast<T>(1e-6);
        if (!singular) {
            result.eulerAngles.x = std::atan2(r(2,1), r(2,2));
            result.eulerAngles.y = std::atan2(-r(2,0), sy);
            result.eulerAngles.z = std::atan2(r(1,0), r(0,0));
        } else {
            result.eulerAngles.x = std::atan2(-r(1,2), r(1,1));
            result.eulerAngles.y = std::atan2(-r(2,0), sy);
            result.eulerAngles.z = 0;
        }

        return result;
    }

    // Interpolation
    static Matrix4 lerp(const Matrix4& a, const Matrix4& b, T t) {
        Matrix4 result;
        for (int i = 0; i < 16; ++i) {
            result.data[i] = a.data[i] + (b.data[i] - a.data[i]) * t;
        }
        return result;
    }

    // Slerp for rotation component, lerp for position/scale
    static Matrix4 slerp(const Matrix4& a, const Matrix4& b, T t) {
        TRSDecomposition<T> da = a.decompose();
        TRSDecomposition<T> db = b.decompose();

        Vector3<T> pos = Vector3<T>::lerp(da.position, db.position, t);
        Vector3<T> scl = Vector3<T>::lerp(da.scale, db.scale, t);

        // Simple nlerp-based rotation interpolation via matrix blending
        Matrix4 result = translation(pos.x, pos.y, pos.z);
        Matrix4 rotA = a;
        Matrix4 rotB = b;
        // Remove translation components
        rotA(0,3) = rotA(1,3) = rotA(2,3) = 0;
        rotB(0,3) = rotB(1,3) = rotB(2,3) = 0;
        // Normalize scale
        Vector3<T> scaleA = a.getScale();
        Vector3<T> scaleB = b.getScale();
        T sa = scaleA.x > 0 ? scaleA.x : 1;
        T sb = scaleB.x > 0 ? scaleB.x : 1;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result(i,j) = (rotA(i,j) / sa) * (1-t) + (rotB(i,j) / sb) * t;
            }
        }
        result(0,0) *= scl.x; result(1,0) *= scl.x; result(2,0) *= scl.x;
        result(0,1) *= scl.y; result(1,1) *= scl.y; result(2,1) *= scl.y;
        result(0,2) *= scl.z; result(1,2) *= scl.z; result(2,2) *= scl.z;

        return result;
    }

    friend std::ostream& operator<<(std::ostream& os, const Matrix4& m) {
        os << "Matrix4(\n";
        for (int row = 0; row < 4; ++row) {
            os << "  ";
            for (int col = 0; col < 4; ++col) {
                os << m(row, col) << (col < 3 ? ", " : "");
            }
            os << (row < 3 ? ",\n" : "\n");
        }
        os << ")";
        return os;
    }

    static const Matrix4 Identity;
};

template<typename T>
const Matrix4<T> Matrix4<T>::Identity = Matrix4<T>();

template<typename T>
Matrix4<T> operator*(T scalar, const Matrix4<T>& mat) {
    return mat * scalar;
}

using Matrix4f = Matrix4<f32>;
using Matrix4d = Matrix4<f64>;

} // namespace nebula
