#include "Vector3.h"

namespace nebula {

template class Vector3<f32>;
template class Vector3<i32>;
template class Vector3<u32>;
template class Vector3<f64>;

template Vector3<f32> operator*<f32>(f32, const Vector3<f32>&);
template Vector3<i32> operator*<i32>(i32, const Vector3<i32>&);
template Vector3<u32> operator*<u32>(u32, const Vector3<u32>&);
template Vector3<f64> operator*<f64>(f64, const Vector3<f64>&);

} // namespace nebula
