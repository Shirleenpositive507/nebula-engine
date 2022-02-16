#include "Matrix4.h"

namespace nebula {

template class Matrix4<f32>;
template class Matrix4<f64>;

template Matrix4<f32> operator*<f32>(f32, const Matrix4<f32>&);
template Matrix4<f64> operator*<f64>(f64, const Matrix4<f64>&);

} // namespace nebula
