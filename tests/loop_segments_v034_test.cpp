#include "../Source/LoopSegmentsV034.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using liberty::loop_v034::describeSegments;

static bool near(double a, double b, double tolerance = 1e-9)
{
    return std::abs(a - b) < tolerance;
}

static void checkSum(const liberty::loop_v034::Segments& result, int total)
{
    assert(result.valid);
    assert(result.beforeFrames + static_cast<long long>(result.loopFrames)
           + result.afterFrames == total);
    assert(near(result.beforePercentage + result.loopPercentage
                + result.afterPercentage, 100.0));
}

int main()
{
    const auto quarter = describeSegments(96000, 24000, 48000);
    checkSum(quarter, 96000);
    assert(quarter.beforeFrames == 24000);
    assert(quarter.loopFrames == 24000);
    assert(quarter.afterFrames == 48000);
    assert(near(quarter.beforePercentage, 25.0));
    assert(near(quarter.loopPercentage, 25.0));
    assert(near(quarter.afterPercentage, 50.0));

    const auto entire = describeSegments(96000, 0, 96000);
    checkSum(entire, 96000);
    assert(near(entire.beforePercentage, 0.0));
    assert(near(entire.loopPercentage, 100.0));
    assert(near(entire.afterPercentage, 0.0));

    const auto atStart = describeSegments(96000, 0, 24000);
    const auto atEnd = describeSegments(96000, 72000, 96000);
    checkSum(atStart, 96000);
    checkSum(atEnd, 96000);
    assert(near(atStart.afterPercentage, 75.0));
    assert(near(atEnd.beforePercentage, 75.0));

    const auto relocated = describeSegments(96000, 48000, 72000);
    checkSum(relocated, 96000);
    assert(near(relocated.loopPercentage, quarter.loopPercentage));
    assert(near(relocated.beforePercentage, 50.0));
    assert(near(relocated.afterPercentage, 25.0));

    const auto minimum = describeSegments(100, 10, 21);
    checkSum(minimum, 100);
    assert(minimum.loopFrames == 11 && minimum.afterFrames == 79);
    assert(near(minimum.loopPercentage, 11.0));

    constexpr int max = std::numeric_limits<int>::max();
    const auto huge = describeSegments(max, 1000000000, 2000000000);
    checkSum(huge, max);
    assert(huge.afterFrames == max - 2000000000);

    for (const auto invalid : {
        describeSegments(0, 0, 0),
        describeSegments(11, 0, 11),
        describeSegments(100, -1, 50),
        describeSegments(100, 50, 101),
        describeSegments(100, 50, 40),
        describeSegments(100, 10, 20),
        describeSegments(100, 101, 100),
        describeSegments(max, -1, max),
    })
        assert(!invalid.valid);

    // These percentages refer to frame positions, not the host's sample rate.
    const auto odd = describeSegments(44100, 11025, 33075);
    checkSum(odd, 44100);
    assert(near(odd.beforePercentage, 25.0));
    assert(near(odd.loopPercentage, 50.0));
    assert(near(odd.afterPercentage, 25.0));

    std::cout << "v0.34 WAV segments, bounds and translation OK\n";
    return 0;
}
