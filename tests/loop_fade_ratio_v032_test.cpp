#include "../Source/LoopFadeRatioV032.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using liberty::loop_v032::describeFade;

static bool close(double a, double b, double tolerance = 1e-9)
{
    return std::abs(a - b) < tolerance;
}

int main()
{
    const auto ten = describeFade(100000, 24000, 48000, 2400);
    assert(ten.valid && ten.loopFrames == 24000);
    assert(ten.fadeFrames == 2400 && close(ten.percentage, 10.0));

    const auto zero = describeFade(100000, 24000, 48000, 0);
    assert(zero.valid && close(zero.percentage, 0.0));
    const auto half = describeFade(100000, 24000, 48000, 12000);
    assert(half.valid && close(half.percentage, 50.0));
    const auto full = describeFade(100000, 24000, 48000, 24000);
    assert(full.valid && close(full.percentage, 100.0));

    // The engine's actual UI can enforce a lower fade upper bound.
    const auto fraction = describeFade(12000, 0, 12000, 1);
    assert(fraction.valid && close(fraction.percentage, 100.0 / 12000.0));

    // End is an exclusive frame, so 10 frames of fade in 11 frames are
    // 90.909..., not 83.33% or 100%.
    const auto minimum = describeFade(100, 10, 21, 10);
    assert(minimum.valid && minimum.loopFrames == 11);
    assert(close(minimum.percentage, 1000.0 / 11.0));

    // Translation of both markers preserves the percentage exactly.
    const auto moved = describeFade(100000, 27000, 51000, 2400);
    assert(moved.valid && close(moved.percentage, ten.percentage));

    // Marker at last exclusive frame is valid.
    const auto atEnd = describeFade(48000, 24000, 48000, 1200);
    assert(atEnd.valid && close(atEnd.percentage, 5.0));

    // Long WAVs are safe (64-bit intermediates for end-begin).
    constexpr int max = std::numeric_limits<int>::max();
    const auto enormous = describeFade(max, 0, max, max / 2);
    assert(enormous.valid && enormous.loopFrames == max);
    assert(enormous.percentage > 49.99 && enormous.percentage < 50.0);

    // No fake ratio when sample, loop or fade are invalid.
    assert(!describeFade(0, 0, 0, 0).valid);
    assert(!describeFade(11, 0, 11, 1).valid);
    assert(!describeFade(100, -1, 50, 10).valid);
    assert(!describeFade(100, 10, 20, 1).valid);
    assert(!describeFade(100, 30, 20, 1).valid);
    assert(!describeFade(100, 10, 101, 1).valid);
    assert(!describeFade(100, 10, 50, -1).valid);
    assert(!describeFade(100, 10, 50, 41).valid);

    std::cout << "v0.32 fade ratio: percent, exclusive end and bounds OK\n";
}
