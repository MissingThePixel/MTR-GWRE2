#include <cmath>

// LLVM emits roundevenf for the generated PPC round-to-nearest-even operation,
// but the Windows CRT used by this build does not provide that C23 function.
extern "C" float roundevenf(float value) {
    if (!std::isfinite(value) || std::fabs(value) >= 8388608.0f) {
        return value;
    }

    const float lower = std::floor(value);
    const float fraction = value - lower;
    float result;
    if (fraction < 0.5f) {
        result = lower;
    } else if (fraction > 0.5f) {
        result = lower + 1.0f;
    } else {
        result = std::fmod(lower, 2.0f) == 0.0f ? lower : lower + 1.0f;
    }
    return result == 0.0f ? std::copysign(0.0f, value) : result;
}
