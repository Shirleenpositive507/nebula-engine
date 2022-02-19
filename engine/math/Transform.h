#pragma once
#include <cmath>
#include "core/Types.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Matrix3.h"
#include "Matrix4.h"
#include "Quaternion.h"

namespace nebula {

class Transform2D {
public:
    Vector2f position;
    f32 rotation;
    Vector2f scale;

    Transform2D()
        : position(0.0f, 0.0f), rotation(0.0f), scale(1.0f, 1.0f) {}

    Transform2D(const Vector2f& pos, f32 rot, const Vector2f& scl)
        : position(pos), rotation(rot), scale(scl) {}

    void translate(const Vector2f& delta) { position += delta; }
    void translate(f32 dx, f32 dy) { position.x += dx; position.y += dy; }

    void rotate(f32 angleRad) { rotation += angleRad; }

    void scaleBy(const Vector2f& factor) { scale *= factor; }
    void scaleBy(f32 sx, f32 sy) { scale.x *= sx; scale.y *= sy; }

    void setPosition(const Vector2f& pos) { position = pos; }
    void setRotation(f32 rot) { rotation = rot; }
    void setScale(const Vector2f& scl) { scale = scl; }

    Vector2f getPosition() const { return position; }
    f32 getRotation() const { return rotation; }
    Vector2f getScale() const { return scale; }

    Matrix3f getLocalMatrix() const {
        return Matrix3f::TRS(position, rotation, scale);
    }

    Matrix3f getMatrix() const { return getLocalMatrix(); }

    Vector2f getForward() const {
        return Vector2f(std::cos(rotation), std::sin(rotation));
    }

    Vector2f getRight() const {
        return Vector2f(std::cos(rotation), std::sin(rotation));
    }

    Vector2f getUp() const {
        return Vector2f(-std::sin(rotation), std::cos(rotation));
    }

    void lookAt(const Vector2f& target) {
        Vector2f dir = target - position;
        if (dir.lengthSquared() > 1e-8f) {
            rotation = std::atan2(dir.y, dir.x);
        }
    }

    f32 getDistance(const Vector2f& point) const {
        return position.distance(point);
    }

    f32 getAngleTo(const Vector2f& point) const {
        Vector2f dir = point - position;
        return std::atan2(dir.y, dir.x);
    }

    static Transform2D interpolate(const Transform2D& a, const Transform2D& b, f32 t) {
        return Transform2D(
            Vector2f::lerp(a.position, b.position, t),
            a.rotation + (b.rotation - a.rotation) * t,
            Vector2f::lerp(a.scale, b.scale, t)
        );
    }

    bool operator==(const Transform2D& other) const {
        return position == other.position && std::abs(rotation - other.rotation) < 1e-6f &&
               scale == other.scale;
    }
    bool operator!=(const Transform2D& other) const { return !(*this == other); }
};

class Transform3D {
public:
    Vector3f position;
    Quaternionf rotation;
    Vector3f scale;

    Transform3D()
        : position(0.0f, 0.0f, 0.0f), rotation(), scale(1.0f, 1.0f, 1.0f) {}

    Transform3D(const Vector3f& pos, const Quaternionf& rot, const Vector3f& scl)
        : position(pos), rotation(rot), scale(scl) {}

    void translate(const Vector3f& delta) { position += delta; }
    void translate(f32 dx, f32 dy, f32 dz) { position.x += dx; position.y += dy; position.z += dz; }

    void rotate(const Quaternionf& delta) { rotation = delta * rotation; }
    void rotate(f32 angleRad, const Vector3f& axis) {
        rotation = Quaternionf::fromAngleAxis(angleRad, axis) * rotation;
    }

    void scaleBy(const Vector3f& factor) { scale *= factor; }
    void scaleBy(f32 sx, f32 sy, f32 sz) { scale.x *= sx; scale.y *= sy; scale.z *= sz; }

    void setPosition(const Vector3f& pos) { position = pos; }
    void setRotation(const Quaternionf& rot) { rotation = rot; }
    void setScale(const Vector3f& scl) { scale = scl; }

    void setEulerAngles(const Vector3f& eulerRad) {
        rotation = Quaternionf::fromEulerAngles(eulerRad);
    }

    Vector3f getPosition() const { return position; }
    Quaternionf getRotation() const { return rotation; }
    Vector3f getScale() const { return scale; }
    Vector3f getEulerAngles() const { return rotation.toEulerAngles(); }

    Matrix4f getLocalMatrix() const {
        return Matrix4f::TRS(position, getEulerAngles(), scale);
    }

    Matrix4f getMatrix() const { return getLocalMatrix(); }

    void setFromMatrix(const Matrix4f& matrix) {
        position = matrix.getTranslation();
        scale = matrix.getScale();
        Matrix3f rotMat = matrix.getRotationMatrix();
        rotation = Quaternionf::fromRotationMatrix(rotMat);
    }

    Vector3f getForward() const {
        return rotation.rotateVector(Vector3f(0.0f, 0.0f, -1.0f));
    }

    Vector3f getRight() const {
        return rotation.rotateVector(Vector3f(1.0f, 0.0f, 0.0f));
    }

    Vector3f getUp() const {
        return rotation.rotateVector(Vector3f(0.0f, 1.0f, 0.0f));
    }

    void lookAt(const Vector3f& target, const Vector3f& up = Vector3f::Up) {
        Vector3f dir = (target - position).normalized();
        if (dir.lengthSquared() < 1e-8f) return;

        Vector3f right = up.cross(dir).normalized();
        if (right.lengthSquared() < 1e-8f) return;

        Vector3f realUp = dir.cross(right).normalized();

        Matrix3f rotMat(
            right.x, realUp.x, dir.x,
            right.y, realUp.y, dir.y,
            right.z, realUp.z, dir.z
        );
        rotation = Quaternionf::fromRotationMatrix(rotMat);
    }

    f32 getDistance(const Vector3f& point) const {
        return position.distance(point);
    }

    static Transform3D interpolate(const Transform3D& a, const Transform3D& b, f32 t) {
        return Transform3D(
            Vector3f::lerp(a.position, b.position, t),
            Quaternionf::slerp(a.rotation, b.rotation, t),
            Vector3f::lerp(a.scale, b.scale, t)
        );
    }

    bool operator==(const Transform3D& other) const {
        return position == other.position && rotation == other.rotation && scale == other.scale;
    }
    bool operator!=(const Transform3D& other) const { return !(*this == other); }
};

} // namespace nebula
