#pragma once
#include <cmath>
#include <cstring>
#include <iostream>
#include "core/Types.h"
#include "Vector2.h"
#include "Vector3.h"

namespace nebula {

template<typename T>
class Matrix3 {
public:
    T data[9];

    Matrix3() { setIdentity(); }
    explicit Matrix3(T value) {
        for (int i = 0; i < 9; ++i) data[i] = value;
    }
    Matrix3(T m00, T m01, T m02,
            T m10, T m11, T m12,
            T m20, T m21, T m22) {
        data[0] = m00; data[1] = m01; data[2] = m02;
        data[3] = m10; data[4] = m11; data[5] = m12;
        data[6] = m20; data[7] = m21; data[8] = m22;
    }

    T& operator()(int row, int col) { return data[row * 3 + col]; }
    const T& operator()(int row, int col) const { return data[row * 3 + col]; }

    T& operator[](int index) { return data[index]; }
    const T& operator[](int index) const { return data[index]; }

    Matrix3 operator+(const Matrix3& other) const {
        Matrix3 result;
        for (int i = 0; i < 9; ++i) result.data[i] = data[i] + other.data[i];
        return result;
    }
    Matrix3 operator-(const Matrix3& other) const {
        Matrix3 result;
        for (int i = 0; i < 9; ++i) result.data[i] = data[i] - other.data[i];
        return result;
    }
    Matrix3 operator*(T scalar) const {
        Matrix3 result;
        for (int i = 0; i < 9; ++i) result.data[i] = data[i] * scalar;
        return result;
    }
    Matrix3 operator*(const Matrix3& other) const {
        Matrix3 result(static_cast<T>(0));
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                for (int k = 0; k < 3; ++k) {
                    result(row, col) += (*this)(row, k) * other(k, col);
                }
            }
        }
        return result;
    }
    Vector3<T> operator*(const Vector3<T>& vec) const {
        return Vector3<T>(
            data[0] * vec.x + data[1] * vec.y + data[2] * vec.z,
            data[3] * vec.x + data[4] * vec.y + data[5] * vec.z,
            data[6] * vec.x + data[7] * vec.y + data[8] * vec.z
        );
    }
    Vector2<T> transformPoint(const Vector2<T>& point) const {
        Vector3<T> result = *this * Vector3<T>(point.x, point.y, static_cast<T>(1));
        return Vector2<T>(result.x, result.y);
    }

    Matrix3& operator+=(const Matrix3& other) { for (int i = 0; i < 9; ++i) data[i] += other.data[i]; return *this; }
    Matrix3& operator-=(const Matrix3& other) { for (int i = 0; i < 9; ++i) data[i] -= other.data[i]; return *this; }
    Matrix3& operator*=(T scalar) { for (int i = 0; i < 9; ++i) data[i] *= scalar; return *this; }

    bool operator==(const Matrix3& other) const {
        for (int i = 0; i < 9; ++i)
            if (data[i] != other.data[i]) return false;
        return true;
    }
    bool operator!=(const Matrix3& other) const { return !(*this == other); }

    void setIdentity() {
        std::memset(data, 0, sizeof(data));
        data[0] = data[4] = data[8] = static_cast<T>(1);
    }

    static Matrix3 identity() { return Matrix3(); }
    static Matrix3 zero() { return Matrix3(static_cast<T>(0)); }

    Matrix3 transposed() const {
        return Matrix3(
            data[0], data[3], data[6],
            data[1], data[4], data[7],
            data[2], data[5], data[8]
        );
    }

    void transpose() { *this = transposed(); }

    T determinant() const {
        return data[0] * (data[4] * data[8] - data[5] * data[7])
             - data[1] * (data[3] * data[8] - data[5] * data[6])
             + data[2] * (data[3] * data[7] - data[4] * data[6]);
    }

    Matrix3 inverse() const {
        T det = determinant();
        if (std::abs(det) < static_cast<T>(1e-8)) return identity();

        T invDet = static_cast<T>(1) / det;
        return Matrix3(
            (data[4] * data[8] - data[5] * data[7]) * invDet,
            (data[2] * data[7] - data[1] * data[8]) * invDet,
            (data[1] * data[5] - data[2] * data[4]) * invDet,
            (data[5] * data[6] - data[3] * data[8]) * invDet,
            (data[0] * data[8] - data[2] * data[6]) * invDet,
            (data[2] * data[3] - data[0] * data[5]) * invDet,
            (data[3] * data[7] - data[4] * data[6]) * invDet,
            (data[1] * data[6] - data[0] * data[7]) * invDet,
            (data[0] * data[4] - data[1] * data[3]) * invDet
        );
    }

    static Matrix3 rotation(T angleRad) {
        T c = std::cos(angleRad);
        T s = std::sin(angleRad);
        return Matrix3(
            c, -s, 0,
            s,  c, 0,
            0,  0, 1
        );
    }

    static Matrix3 scaling(T sx, T sy) {
        return Matrix3(
            sx, 0,  0,
            0,  sy, 0,
            0,  0,  1
        );
    }

    static Matrix3 translation(T tx, T ty) {
        return Matrix3(
            1,  0,  tx,
            0,  1,  ty,
            0,  0,  1
        );
    }

    static Matrix3 TRS(const Vector2<T>& translation, T rotationRad, const Vector2<T>& scale) {
        Matrix3 t = translation(translation.x, translation.y);
        Matrix3 r = rotation(rotationRad);
        Matrix3 s = scaling(scale.x, scale.y);
        return t * r * s;
    }

    friend std::ostream& operator<<(std::ostream& os, const Matrix3& m) {
        os << "Matrix3(\n";
        for (int row = 0; row < 3; ++row) {
            os << "  ";
            for (int col = 0; col < 3; ++col) {
                os << m(row, col) << (col < 2 ? ", " : "");
            }
            os << (row < 2 ? ",\n" : "\n");
        }
        os << ")";
        return os;
    }

    static const Matrix3 Identity;
};

template<typename T>
const Matrix3<T> Matrix3<T>::Identity = Matrix3<T>();

template<typename T>
Matrix3<T> operator*(T scalar, const Matrix3<T>& mat) {
    return mat * scalar;
}

using Matrix3f = Matrix3<f32>;
using Matrix3d = Matrix3<f64>;

} // namespace nebula
