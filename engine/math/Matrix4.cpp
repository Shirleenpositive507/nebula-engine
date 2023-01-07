#include "Matrix4.h"

namespace nebula {

template class Matrix4<f32>;
template class Matrix4<f64>;

template struct Plane<f32>;
template struct Plane<f64>;
template struct TRSDecomposition<f32>;
template struct TRSDecomposition<f64>;

template Matrix4<f32> operator*<f32>(f32, const Matrix4<f32>&);
template Matrix4<f64> operator*<f64>(f64, const Matrix4<f64>&);

// Batch matrix operations
template<typename T>
void matrix4BatchMultiply(const Matrix4<T>* a, const Matrix4<T>* b, Matrix4<T>* out, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        out[i] = a[i] * b[i];
    }
}

template<typename T>
void matrix4BatchInverse(const Matrix4<T>* matrices, Matrix4<T>* out, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        out[i] = matrices[i].inverse();
    }
}

template<typename T>
bool matrix4DecomposeAll(const Matrix4<T>* matrices, TRSDecomposition<T>* decomps, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        decomps[i] = matrices[i].decompose();
    }
    return true;
}

template void matrix4BatchMultiply<f32>(const Matrix4<f32>*, const Matrix4<f32>*, Matrix4<f32>*, size_t);
template void matrix4BatchMultiply<f64>(const Matrix4<f64>*, const Matrix4<f64>*, Matrix4<f64>*, size_t);
template void matrix4BatchInverse<f32>(const Matrix4<f32>*, Matrix4<f32>*, size_t);
template void matrix4BatchInverse<f64>(const Matrix4<f64>*, Matrix4<f64>*, size_t);
template bool matrix4DecomposeAll<f32>(const Matrix4<f32>*, TRSDecomposition<f32>*, size_t);
template bool matrix4DecomposeAll<f64>(const Matrix4<f64>*, TRSDecomposition<f64>*, size_t);

} // namespace nebula
