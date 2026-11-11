#include "../Source/LoopWheelPanV025.h"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

using liberty::loop_v025::panByWheel;

int main()
{
    // A single normalized wheel notch moves 20% of the visible frames.
    const auto earlier = panByWheel(10000, 3000, 2000, 1.0);
    const auto later = panByWheel(10000, 3000, 2000, -1.0);
    assert(earlier.valid && earlier.firstFrame == 2600 && earlier.movedFrames == -400);
    assert(later.valid && later.firstFrame == 3400 && later.movedFrames == 400);
    const auto neutral = panByWheel(10000, 3000, 2000, 0.0);
    assert(neutral.valid && neutral.firstFrame == 3000 && neutral.movedFrames == 0);

    // Subsequent inverse wheel steps are reversible, barring file edges.
    const auto back = panByWheel(10000, earlier.firstFrame, 2000, -1.0);
    assert(back.valid && back.firstFrame == 3000);
    const auto otherBack = panByWheel(10000, later.firstFrame, 2000, 1.0);
    assert(otherBack.valid && otherBack.firstFrame == 3000);

    // Very large deltas cannot move beyond 80% of the viewport per event.
    const auto hugeUp = panByWheel(10000, 3000, 2000, 1e12);
    const auto hugeDown = panByWheel(10000, 3000, 2000, -1e12);
    assert(hugeUp.valid && hugeUp.firstFrame == 1400);
    assert(hugeDown.valid && hugeDown.firstFrame == 4600);

    // Boundaries remain clamped and a full-file fit never scrolls.
    assert(panByWheel(10000, 0, 2000, 1.0).firstFrame == 0);
    assert(panByWheel(10000, 8000, 2000, -1.0).firstFrame == 8000);
    assert(panByWheel(10000, 0, 10000, 1.0).firstFrame == 0);
    assert(panByWheel(10000, 0, 10000, -1.0).firstFrame == 0);

    // Small continuous trackpad deltas preserve nearest-frame rounding.
    const auto small = panByWheel(10000, 100, 100, 0.11);
    assert(small.valid && small.firstFrame == 98);
    const auto smaller = panByWheel(10000, 100, 100, 0.01);
    assert(smaller.valid && smaller.firstFrame == 100);

    // Extreme file lengths and position values do not overflow.
    constexpr int max = std::numeric_limits<int>::max();
    const auto nearEnd = panByWheel(max, max - 1000, 1000, -9999999.0);
    assert(nearEnd.valid && nearEnd.firstFrame == max - 1000);
    const auto nearStart = panByWheel(max, 0, 1000, 9999999.0);
    assert(nearStart.valid && nearStart.firstFrame == 0);
    const auto wide = panByWheel(max, 1000000000, 1000000000, 1.0);
    assert(wide.valid && wide.firstFrame == 800000000);

    // Invalid state and infinite/NaN scroll inputs are rejected.
    assert(!panByWheel(0, 0, 1, 1.0).valid);
    assert(!panByWheel(10000, -1, 500, 1.0).valid);
    assert(!panByWheel(10000, 9999, 500, 1.0).valid);
    assert(!panByWheel(10000, 0, 0, 1.0).valid);
    assert(!panByWheel(10000, 0, 10001, 1.0).valid);
    assert(!panByWheel(10000, 100, 200,
                       std::numeric_limits<double>::infinity()).valid);
    assert(!panByWheel(10000, 100, 200,
                       std::numeric_limits<double>::quiet_NaN()).valid);

    std::cout << "v0.25 Shift-wheel pan: stable direction, steps and bounds OK\n";
}
