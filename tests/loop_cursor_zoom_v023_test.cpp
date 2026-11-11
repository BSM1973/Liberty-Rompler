#include "../Source/LoopCursorZoomV023.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

using liberty::loop_v023::zoomAtCursor;

int main()
{
    // The sample under the cursor should stay there while zooming.
    const auto leftQuarter = zoomAtCursor(1000, 200, 400, 200, 800, 200);
    assert(leftQuarter.valid && leftQuarter.firstFrame == 250);
    assert(leftQuarter.spanFrames == 200);
    const auto rightQuarter = zoomAtCursor(1000, 200, 400, 200, 800, 600);
    assert(rightQuarter.valid && rightQuarter.firstFrame == 350);
    const auto unchanged = zoomAtCursor(1000, 200, 400, 400, 800, 600);
    assert(unchanged.valid && unchanged.firstFrame == 200);
    const auto atLeft = zoomAtCursor(1000, 200, 400, 200, 800, 0);
    assert(atLeft.valid && atLeft.firstFrame == 200);
    const auto atRight = zoomAtCursor(1000, 200, 400, 200, 800, 800);
    assert(atRight.valid && atRight.firstFrame == 400);

    // Outside-of-waveform mouse positions are safely clipped.
    const auto negativePointer = zoomAtCursor(1000, 200, 400, 200, 800, -999);
    const auto farPointer = zoomAtCursor(1000, 200, 400, 200, 800, 9999);
    assert(negativePointer.valid && negativePointer.firstFrame == 200);
    assert(farPointer.valid && farPointer.firstFrame == 400);
    const auto zoomOutClipped = zoomAtCursor(1000, 0, 200, 800, 800, 400);
    assert(zoomOutClipped.valid && zoomOutClipped.firstFrame == 0);
    const auto rightClip = zoomAtCursor(1000, 700, 300, 800, 800, 400);
    assert(rightClip.valid && rightClip.firstFrame == 200);

    // Exact fit is always first 0, full span.
    const auto fit = zoomAtCursor(1000, 50, 300, 1000, 800, 123);
    assert(fit.valid && fit.firstFrame == 0 && fit.spanFrames == 1000);

    // A single-frame WAV and long WAVs remain valid.
    const auto single = zoomAtCursor(1, 0, 1, 1, 1, 0);
    assert(single.valid && single.firstFrame == 0);
    constexpr int max = std::numeric_limits<int>::max();
    const auto huge = zoomAtCursor(max, 1000000000, 1000000000,
                                   500000000, 1024, 512);
    assert(huge.valid && huge.firstFrame == 1250000000);
    assert(huge.spanFrames == 500000000);

    // All incorrect spans and geometry are rejected before calculations.
    assert(!zoomAtCursor(0, 0, 1, 1, 800, 400).valid);
    assert(!zoomAtCursor(1000, -1, 100, 50, 800, 400).valid);
    assert(!zoomAtCursor(1000, 999, 10, 5, 800, 400).valid);
    assert(!zoomAtCursor(1000, 0, 0, 50, 800, 400).valid);
    assert(!zoomAtCursor(1000, 0, 100, 0, 800, 400).valid);
    assert(!zoomAtCursor(1000, 0, 100, 1001, 800, 400).valid);
    assert(!zoomAtCursor(1000, 0, 100, 50, 0, 400).valid);
    std::cout << "v0.23 cursor zoom: stable anchor and clipped bounds OK\n";
}
