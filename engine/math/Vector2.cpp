#include "Vector2.h"
#include <cmath>
#include <algorithm>

namespace nebula {

template class Vector2<f32>;
template class Vector2<i32>;
template class Vector2<u32>;
template class Vector2<f64>;

template Vector2<f32> operator*<f32>(f32, const Vector2<f32>&);
template Vector2<i32> operator*<i32>(i32, const Vector2<i32>&);
template Vector2<u32> operator*<u32>(u32, const Vector2<u32>&);
template Vector2<f64> operator*<f64>(f64, const Vector2<f64>&);

// Batch vector operations
template<typename T>
void vector2BatchNormalize(Vector2<T>* vectors, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        if (vectors[i].isZero()) continue;
        vectors[i].normalize();
    }
}

template<typename T>
T vector2BatchDotProduct(const Vector2<T>* a, const Vector2<T>* b, size_t count) {
    T sum = 0;
    for (size_t i = 0; i < count; ++i) {
        sum += a[i].dot(b[i]);
    }
    return sum;
}

template<typename T>
void vector2BatchLerp(const Vector2<T>* a, const Vector2<T>* b, Vector2<T>* out, T t, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        out[i] = Vector2<T>::lerp(a[i], b[i], t);
    }
}

template void vector2BatchNormalize<f32>(Vector2<f32>*, size_t);
template void vector2BatchNormalize<f64>(Vector2<f64>*, size_t);
template f32 vector2BatchDotProduct<f32>(const Vector2<f32>*, const Vector2<f32>*, size_t);
template f64 vector2BatchDotProduct<f64>(const Vector2<f64>*, const Vector2<f64>*, size_t);
template void vector2BatchLerp<f32>(const Vector2<f32>*, const Vector2<f32>*, Vector2<f32>*, f32, size_t);
template void vector2BatchLerp<f64>(const Vector2<f64>*, const Vector2<f64>*, Vector2<f64>*, f64, size_t);

} // namespace nebula
