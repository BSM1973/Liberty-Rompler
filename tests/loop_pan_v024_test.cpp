#include "../Source/LoopPanV024.h"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>

using liberty::loop_v024::panFromAnchor;

int main()
{
    // Translation is computed from a fixed mouse-down anchor.
    const auto init = panFromAnchor(10000, 3000, 2000, 1000, 500, 500);
    assert(init.valid && init.firstFrame == 3000 && init.deltaFrames == 0);
    const auto right = panFromAnchor(10000, 3000, 2000, 1000, 500, 750);
    assert(right.valid && right.firstFrame == 2500 && right.deltaFrames == -500);
    const auto left = panFromAnchor(10000, 3000, 2000, 1000, 500, 250);
    assert(left.valid && left.firstFrame == 3500 && left.deltaFrames == 500);
    const auto back = panFromAnchor(10000, 3000, 2000, 1000, 500, 500);
    assert(back.valid && back.firstFrame == 3000); // no drift

    // Gesture may leave the waveform; never extrapolate out of the file.
    const auto farRight = panFromAnchor(10000, 3000, 2000, 1000, 500, 100000);
    const auto farLeft = panFromAnchor(10000, 3000, 2000, 1000, 500, -100000);
    assert(farRight.valid && farRight.firstFrame == 1000);
    assert(farLeft.valid && farLeft.firstFrame == 5000);

    // At file edges, pointer movement cannot reveal non-existent samples.
    const auto start = panFromAnchor(10000, 0, 2000, 1000, 500, 1000);
    const auto end = panFromAnchor(10000, 8000, 2000, 1000, 500, 0);
    assert(start.valid && start.firstFrame == 0);
    assert(end.valid && end.firstFrame == 8000);

    // Full WAV fit view stays stationary, even with Option+drag.
    const auto full = panFromAnchor(10000, 0, 10000, 1000, 500, 800);
    assert(full.valid && full.firstFrame == 0);

    // Frame rounding and pointer invariance are deterministic.
    const auto tiny = panFromAnchor(10000, 3000, 100, 400, 200, 199);
    assert(tiny.valid && tiny.firstFrame == 3000);
    const auto onePixel = panFromAnchor(10000, 3000, 100, 400, 200, 196);
    assert(onePixel.valid && onePixel.firstFrame == 3001);

    // Huge integer coordinates are safe. A width of one pixel is valid.
    constexpr int max = std::numeric_limits<int>::max();
    constexpr int min = std::numeric_limits<int>::min();
    const auto huge = panFromAnchor(max, 1000000000, 1000000000, 1, max, min);
    assert(huge.valid && huge.firstFrame == 1147483647);
    const auto reverseHuge = panFromAnchor(max, 1000000000, 1000000000, 1, min, max);
    assert(reverseHuge.valid && reverseHuge.firstFrame == 0);

    // Invalid viewport state is rejected without changing anything.
    assert(!panFromAnchor(0, 0, 1, 100, 20, 30).valid);
    assert(!panFromAnchor(10000, -1, 100, 100, 20, 30).valid);
    assert(!panFromAnchor(10000, 9999, 100, 100, 20, 30).valid);
    assert(!panFromAnchor(10000, 100, 0, 100, 20, 30).valid);
    assert(!panFromAnchor(10000, 100, 10001, 100, 20, 30).valid);
    assert(!panFromAnchor(10000, 100, 100, 0, 20, 30).valid);
    assert(panFromAnchor(1, 0, 1, 1, 0, 1).valid);

    std::cout << "v0.24 anchored pan: bounded viewport and no marker edits OK\n";
}
