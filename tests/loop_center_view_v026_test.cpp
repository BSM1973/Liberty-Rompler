#include "../Source/LoopCenterViewV026.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>

using liberty::loop_v026::centerOnPointer;

int main()
{
    // Click a frame at 75% of the viewport and bring it to the center.
    const auto normal = centerOnPointer(1000, 200, 400, 800, 600);
    assert(normal.valid && normal.targetFrame == 500);
    assert(normal.firstFrame == 300 && normal.spanFrames == 400);
    const auto otherSide = centerOnPointer(1000, 200, 400, 800, 200);
    assert(otherSide.valid && otherSide.targetFrame == 300);
    assert(otherSide.firstFrame == 100);

    // Mouse moved outside waveform must clamp to the nearest visible frame.
    const auto before = centerOnPointer(1000, 200, 400, 800, -5000);
    assert(before.valid && before.targetFrame == 200);
    assert(before.firstFrame == 0);
    const auto beyond = centerOnPointer(1000, 200, 400, 800, 5000);
    assert(beyond.valid && beyond.targetFrame == 600);
    assert(beyond.firstFrame == 400);

    // Full-fit view never moves; no change to zoom/span in any result.
    const auto fit = centerOnPointer(1000, 0, 1000, 800, 401);
    assert(fit.valid && fit.firstFrame == 0 && fit.spanFrames == 1000);
    const auto atStart = centerOnPointer(1000, 0, 200, 800, 0);
    const auto atEnd = centerOnPointer(1000, 800, 200, 800, 800);
    assert(atStart.valid && atStart.firstFrame == 0);
    assert(atEnd.valid && atEnd.firstFrame == 800);

    // Exact single-frame audio and max-size WAVs avoid signed overflow.
    const auto single = centerOnPointer(1, 0, 1, 1, 0);
    assert(single.valid && single.firstFrame == 0 && single.targetFrame == 0);
    constexpr int largest = std::numeric_limits<int>::max();
    const auto huge = centerOnPointer(largest, 1000000000,
                                      1000000000, 1024, 512);
    assert(huge.valid && huge.targetFrame == 1500000000);
    assert(huge.firstFrame == 1000000000);
    const auto extremeX = centerOnPointer(largest, 0, largest,
                                          largest, largest);
    assert(extremeX.valid && extremeX.firstFrame == 0);
    assert(extremeX.targetFrame == largest);

    // Invalid viewport state is reported rather than guessed.
    assert(!centerOnPointer(0, 0, 1, 800, 400).valid);
    assert(!centerOnPointer(1000, -1, 200, 800, 400).valid);
    assert(!centerOnPointer(1000, 900, 200, 800, 400).valid);
    assert(!centerOnPointer(1000, 0, 0, 800, 400).valid);
    assert(!centerOnPointer(1000, 0, 1001, 800, 400).valid);
    assert(!centerOnPointer(1000, 0, 200, 0, 400).valid);

    std::cout << "v0.26 center view: pointer frame, zoom lock and edges OK\n";
}
