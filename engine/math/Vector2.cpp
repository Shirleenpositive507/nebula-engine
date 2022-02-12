#include "Vector2.h"

namespace nebula {

template class Vector2<f32>;
template class Vector2<i32>;
template class Vector2<u32>;
template class Vector2<f64>;

template Vector2<f32> operator*<f32>(f32, const Vector2<f32>&);
template Vector2<i32> operator*<i32>(i32, const Vector2<i32>&);
template Vector2<u32> operator*<u32>(u32, const Vector2<u32>&);
template Vector2<f64> operator*<f64>(f64, const Vector2<f64>&);

} // namespace nebula
