#include "Vector4.h"

namespace nebula {

template class Vector4<f32>;
template class Vector4<i32>;
template class Vector4<u32>;
template class Vector4<f64>;

template Vector4<f32> operator*<f32>(f32, const Vector4<f32>&);
template Vector4<i32> operator*<i32>(i32, const Vector4<i32>&);
template Vector4<u32> operator*<u32>(u32, const Vector4<u32>&);
template Vector4<f64> operator*<f64>(f64, const Vector4<f64>&);

} // namespace nebula
