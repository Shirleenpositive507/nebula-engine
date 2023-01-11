#include "Transform.h"

namespace nebula {

void transform3DLookAtTarget(Transform3D& transform, const Vector3f& target, const Vector3f& up) {
    transform.lookAt(target, up);
}

void transform3DLookAtDirection(Transform3D& transform, const Vector3f& direction, const Vector3f& up) {
    Vector3f dir = direction.normalized();
    if (dir.lengthSquared() < 1e-8f) return;
    Vector3f right = up.cross(dir).normalized();
    if (right.lengthSquared() < 1e-8f) return;
    Vector3f realUp = dir.cross(right).normalized();
    Matrix3f rotMat(
        right.x, realUp.x, dir.x,
        right.y, realUp.y, dir.y,
        right.z, realUp.z, dir.z
    );
    transform.rotation = Quaternionf::fromRotationMatrix(rotMat);
}

void transform3DOrbit(Transform3D& transform, const Vector3f& pivot, f32 horizontalAngle, f32 verticalAngle, f32 distance) {
    f32 cx = std::cos(verticalAngle) * std::cos(horizontalAngle);
    f32 cy = std::sin(verticalAngle);
    f32 cz = std::cos(verticalAngle) * std::sin(horizontalAngle);
    transform.position = pivot + Vector3f(cx, cy, cz) * distance;
    transform.lookAt(pivot);
}

f32 transform3DDistanceToPoint(const Transform3D& transform, const Vector3f& point) {
    return transform.position.distance(point);
}

Vector3f transform3DWorldToLocalDirection(const Transform3D& transform, const Vector3f& worldDir) {
    return transform.rotation.conjugated().rotateVector(worldDir);
}

Vector3f transform3DLocalToWorldDirection(const Transform3D& transform, const Vector3f& localDir) {
    return transform.rotation.rotateVector(localDir);
}

} // namespace nebula
