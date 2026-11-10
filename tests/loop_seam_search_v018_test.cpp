#include "../Source/LoopSeamSearchV018.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
#include <array>

using liberty::loop_v018::findLowestRawStep;

int main()
{
    // Searching a start marker must compare candidate[start] to end-1
    // across *both* channels and never change the input windows.
    const std::vector<std::vector<float>> starts {
        {0.7f, 0.6f, 0.5f, 0.2f, 0.01f, 0.3f, 0.7f},
        {0.7f, 0.6f, 0.5f, 0.2f, 0.02f, 0.3f, 0.7f}
    };
    const auto before = starts;
    const std::vector<std::array<float, 2>> fixedStart {{0, 0}, {0, 0}};
    const auto bestStart = findLowestRawStep(starts, 100, fixedStart,
                                             true, 102, 100, 105, 512);
    assert(bestStart.valid);
    assert(bestStart.marker == 104);
    assert(std::abs(bestStart.previousStep - 0.5f) < 1e-6f);
    assert(std::abs(bestStart.bestStep - 0.02f) < 1e-6f);
    assert(starts == before);

    // Searching an exclusive end uses moving[end-1] vs fixed start[0].
    const std::vector<std::vector<float>> ends {
        {0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.01f, 0.3f, 0.9f},
        {0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.02f, 0.3f, 0.9f}
    };
    const auto bestEnd = findLowestRawStep(ends, 200, fixedStart,
                                           false, 204, 202, 208, 512);
    assert(bestEnd.valid);
    assert(bestEnd.marker == 206);
    assert(std::abs(bestEnd.previousStep - 0.5f) < 1e-6f);
    assert(std::abs(bestEnd.bestStep - 0.02f) < 1e-6f);

    // A perfect existing seam should not be moved for merely equal scores.
    const std::vector<std::vector<float>> quiet {{0.0f, 0.0f, 0.0f, 0.0f}};
    const std::vector<std::array<float, 2>> zeroEdge {{0, 0}};
    const auto alreadyBest = findLowestRawStep(
        quiet, 10, zeroEdge, true, 11, 10, 12, 512);
    assert(alreadyBest.valid && alreadyBest.marker == 11);
    assert(alreadyBest.bestStep == 0.0f);

    // On stereo material a result cannot improve left at right's expense
    // when the maximum two-channel discontinuity would increase.
    const std::vector<std::vector<float>> stereo {
        {0.0f, 0.1f, 0.3f, 0.0f},
        {0.0f, 0.1f, 0.8f, 0.9f}
    };
    const auto stereoChoice = findLowestRawStep(
        stereo, 10, {{0, 0}, {0, 0}}, true, 11, 10, 12);
    assert(stereoChoice.valid);
    assert(stereoChoice.marker == 10);
    assert(stereoChoice.bestStep < stereoChoice.previousStep);

    // Radius and bounds must be respected; no improvement stays unchanged.
    const auto fixedBound = findLowestRawStep(starts, 100, fixedStart,
                                              true, 102, 101, 103, 0);
    assert(fixedBound.valid && fixedBound.marker == 102);
    const auto oneFrame = findLowestRawStep(starts, 100, fixedStart,
                                            true, 102, 101, 103, 1);
    assert(oneFrame.valid && oneFrame.marker == 103);
    assert(!findLowestRawStep({}, 0, {}, true, 10, 0, 10).valid);
    assert(!findLowestRawStep(quiet, 0, zeroEdge, true, 2, 0, 10, -1).valid);
    assert(!findLowestRawStep(quiet, 0, zeroEdge, true, 2, 0, 10, 5000).valid);
    assert(!findLowestRawStep(quiet, -1, zeroEdge, true, 2, 0, 10).valid);
    assert(!findLowestRawStep(quiet, 0, zeroEdge, true, 2, 12, 10).valid);

    auto invalid = quiet;
    invalid[0][1] = std::numeric_limits<float>::quiet_NaN();
    assert(!findLowestRawStep(invalid, 0, zeroEdge, true, 1, 0, 2).valid);

    std::vector<float> tooLarge(16385, 0.0f);
    assert(!findLowestRawStep({tooLarge}, 0, zeroEdge, true, 10, 0, 20).valid);

    std::cout << "v0.18 raw seam search: conservative mono/stereo tests OK\n";
}
