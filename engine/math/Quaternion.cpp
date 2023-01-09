#include "Quaternion.h"

namespace nebula {

template class Quaternion<f32>;
template class Quaternion<f64>;

template Quaternion<f32> operator*<f32>(f32, const Quaternion<f32>&);
template Quaternion<f64> operator*<f64>(f64, const Quaternion<f64>&);

template<typename T>
Quaternion<T> quaternionBatchSlerp(const Quaternion<T>* a, const Quaternion<T>* b, T t, size_t count) {
    Quaternion<T> result(0, 0, 0, 0);
    for (size_t i = 0; i < count; ++i) {
        result = result + Quaternion<T>::slerp(a[i], b[i], t);
    }
    return result.normalized();
}

template<typename T>
void quaternionBatchLookRotation(const Vector3<T>* directions, const Vector3<T>* ups, Quaternion<T>* out, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        out[i] = Quaternion<T>::lookRotation(directions[i], ups ? ups[i] : Vector3<T>::Up);
    }
}

template<typename T>
T quaternionAverageAngularDistance(const Quaternion<T>* quaternions, const Quaternion<T>& target, size_t count) {
    T sum = 0;
    for (size_t i = 0; i < count; ++i) {
        sum += quaternions[i].angularDistance(target);
    }
    return sum / static_cast<T>(count);
}

template Quaternion<f32> quaternionBatchSlerp<f32>(const Quaternion<f32>*, const Quaternion<f32>*, f32, size_t);
template Quaternion<f64> quaternionBatchSlerp<f64>(const Quaternion<f64>*, const Quaternion<f64>*, f64, size_t);
template void quaternionBatchLookRotation<f32>(const Vector3<f32>*, const Vector3<f32>*, Quaternion<f32>*, size_t);
template void quaternionBatchLookRotation<f64>(const Vector3<f64>*, const Vector3<f64>*, Quaternion<f64>*, size_t);
template f32 quaternionAverageAngularDistance<f32>(const Quaternion<f32>*, const Quaternion<f32>&, size_t);
template f64 quaternionAverageAngularDistance<f64>(const Quaternion<f64>*, const Quaternion<f64>&, size_t);

} // namespace nebula
