#include "../Source/LoopCoverageV033.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using liberty::loop_v033::describeCoverage;

static bool close(double a, double b, double tolerance = 1e-9)
{
    return std::abs(a - b) < tolerance;
}

int main()
{
    // A quarter of the WAV is selected, independent of sample rate.
    const auto quarter = describeCoverage(96000, 24000, 48000);
    assert(quarter.valid && quarter.totalFrames == 96000);
    assert(quarter.loopFrames == 24000 && close(quarter.percentage, 25.0));
    const auto half = describeCoverage(96000, 24000, 72000);
    assert(half.valid && close(half.percentage, 50.0));

    // Loop may fill the whole file; end is exclusive, not end+1.
    const auto whole = describeCoverage(96000, 0, 96000);
    assert(whole.valid && whole.loopFrames == 96000);
    assert(close(whole.percentage, 100.0));
    const auto zeroStart = describeCoverage(96000, 0, 24000);
    const auto lastEnd = describeCoverage(96000, 72000, 96000);
    assert(zeroStart.valid && lastEnd.valid);
    assert(close(zeroStart.percentage, 25.0)
           && close(lastEnd.percentage, 25.0));

    // Same length at another position must yield identical coverage.
    const auto moved = describeCoverage(96000, 40000, 64000);
    assert(moved.valid && close(moved.percentage, quarter.percentage));

    // Existing minimum valid loop length: FIN - DEBUT >= 11.
    const auto minimum = describeCoverage(100, 10, 21);
    assert(minimum.valid && minimum.loopFrames == 11);
    assert(close(minimum.percentage, 11.0));

    // Very long WAVs must never overflow 32-bit arithmetic.
    constexpr int max = std::numeric_limits<int>::max();
    const auto enormous = describeCoverage(max, 0, max);
    assert(enormous.valid && close(enormous.percentage, 100.0));
    const auto oneThird = describeCoverage(max, 0, max / 3);
    assert(oneThird.valid && oneThird.percentage > 33.33);
    assert(oneThird.percentage < 33.34);

    // Invalid inputs never render fake percentages.
    assert(!describeCoverage(0, 0, 0).valid);
    assert(!describeCoverage(11, 0, 11).valid);
    assert(!describeCoverage(100, -1, 50).valid);
    assert(!describeCoverage(100, 10, 20).valid);
    assert(!describeCoverage(100, 30, 20).valid);
    assert(!describeCoverage(100, 10, 101).valid);
    assert(!describeCoverage(100, 0, 0).valid);

    std::cout << "v0.33 WAV coverage: full, quarter, translation and bounds OK\n";
}
