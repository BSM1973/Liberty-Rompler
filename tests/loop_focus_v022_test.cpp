#include "../Source/LoopFocusV022.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

using liberty::loop_v022::planFocus;

int main()
{
    const auto normal = planFocus(100000, 10000, 20000, 64.0);
    assert(normal.valid);
    assert(normal.centerFrame == 15000);
    assert(normal.targetFrames == 12500);
    assert(std::abs(normal.requestedZoom - 8.0) < 1e-9);

    // Full-file loops must remain fit-to-all, not unexpectedly zoom in.
    const auto entire = planFocus(100000, 0, 100000, 64.0);
    assert(entire.valid && entire.requestedZoom == 1.0);
    assert(entire.targetFrames == 100000);

    // Small loops are padded to at least 32 frames, respecting max zoom.
    const auto shortLoop = planFocus(10000, 999, 1010, 16.0);
    assert(shortLoop.valid && shortLoop.targetFrames == 32);
    assert(shortLoop.centerFrame == 1004);
    assert(shortLoop.requestedZoom == 16.0);

    const auto limitedZoom = planFocus(100000, 20000, 20100, 2.0);
    assert(limitedZoom.valid && limitedZoom.requestedZoom == 2.0);
    assert(limitedZoom.targetFrames == 126);

    // Do not lose loops near file boundaries or on long recordings.
    const auto first = planFocus(40000, 0, 11, 64.0);
    const auto last = planFocus(40000, 39989, 40000, 64.0);
    assert(first.valid && last.valid);
    assert(first.centerFrame == 5 && last.centerFrame == 39994);
    const auto huge = planFocus(std::numeric_limits<int>::max(),
                                1100000000, 2100000000, 64.0);
    assert(huge.valid);
    assert(huge.centerFrame == 1600000000);
    assert(huge.targetFrames == 1250000000);
    assert(huge.requestedZoom >= 1.0);

    // Invalid marker selection and invalid zoom cannot alter viewport.
    assert(!planFocus(11, 0, 11, 64.0).valid);
    assert(!planFocus(1000, -1, 200, 64.0).valid);
    assert(!planFocus(1000, 100, 110, 64.0).valid);
    assert(!planFocus(1000, 100, 1001, 64.0).valid);
    assert(!planFocus(1000, 200, 100, 64.0).valid);
    assert(!planFocus(1000, 100, 200, 0.9).valid);
    assert(!planFocus(1000, 100, 200,
                      std::numeric_limits<double>::infinity()).valid);
    assert(!planFocus(1000, 100, 200,
                      std::numeric_limits<double>::quiet_NaN()).valid);
    std::cout << "v0.22 focus: frame bounds, loop padding, zoom caps OK\n";
}
