#include "MathUtil.h"
#include <random>

namespace nebula {

f32 mathGaussianF32(f32 x, f32 mean, f32 sigma) {
    return Math::gaussian(x, mean, sigma);
}

f64 mathGaussianF64(f64 x, f64 mean, f64 sigma) {
    return Math::gaussian(x, mean, sigma);
}

f32 mathSigmoidF32(f32 x, f32 gain) {
    return Math::sigmoid(x, gain);
}

f32 mathBezierF32(f32 p0, f32 p1, f32 p2, f32 p3, f32 t) {
    return Math::bezier(p0, p1, p2, p3, t);
}

f32 mathCatmullRomF32(f32 p0, f32 p1, f32 p2, f32 p3, f32 t) {
    return Math::catmullRom(p0, p1, p2, p3, t);
}

f32 mathSmoothDampF32(f32 current, f32 target, f32& velocity, f32 smoothTime, f32 maxSpeed, f32 deltaTime) {
    return Math::smoothDamp(current, target, velocity, smoothTime, maxSpeed, deltaTime);
}

f32 mathRepeatF32(f32 t, f32 length) {
    return Math::repeat(t, length);
}

f32 mathPingPongF32(f32 t, f32 length) {
    return Math::pingPong(t, length);
}

f32 mathDeltaAngleDeg(f32 from, f32 to) {
    f32 diff = to - from;
    while (diff > 180.0f) diff -= 360.0f;
    while (diff < -180.0f) diff += 360.0f;
    return diff;
}

f32 mathEaseInBack(f32 t) {
    const f32 c1 = 1.70158f;
    const f32 c3 = c1 + 1.0f;
    return c3 * t * t * t - c1 * t * t;
}

f32 mathEaseOutBack(f32 t) {
    const f32 c1 = 1.70158f;
    const f32 c3 = c1 + 1.0f;
    return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
}

f32 mathEaseOutElastic(f32 t) {
    const f32 c4 = (2.0f * Math::PI) / 3.0f;
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}

} // namespace nebula
