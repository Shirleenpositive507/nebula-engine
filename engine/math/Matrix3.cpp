#include "Matrix3.h"

namespace nebula {

template class Matrix3<f32>;
template class Matrix3<f64>;

template Matrix3<f32> operator*<f32>(f32, const Matrix3<f32>&);
template Matrix3<f64> operator*<f64>(f64, const Matrix3<f64>&);

} // namespace nebula
