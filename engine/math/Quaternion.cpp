#include "Quaternion.h"

namespace nebula {

template class Quaternion<f32>;
template class Quaternion<f64>;

template Quaternion<f32> operator*<f32>(f32, const Quaternion<f32>&);
template Quaternion<f64> operator*<f64>(f64, const Quaternion<f64>&);

} // namespace nebula
