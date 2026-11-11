#include "../Source/LoopSegmentTimesV035.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using liberty::loop_v035::describeSegmentTimes;

static bool near(double a, double b, double tolerance = 1e-6)
{
    return std::abs(a - b) <= tolerance;
}

int main()
{
    // A 2 second WAV at 48 kHz: half a second before, one second after.
    const auto sample = describeSegmentTimes(96000, 24000, 48000, 48000.0);
    assert(sample.valid && sample.beforeFrames == 24000);
    assert(sample.afterFrames == 48000);
    assert(near(sample.beforeMs, 500.0) && near(sample.afterMs, 1000.0));

    // Full WAV loop, empty margins.
    const auto full = describeSegmentTimes(96000, 0, 96000, 48000.0);
    assert(full.valid && full.beforeFrames == 0 && full.afterFrames == 0);
    assert(near(full.beforeMs, 0.0) && near(full.afterMs, 0.0));

    // A loop at either edge leaves the remaining side correctly measured.
    const auto atStart = describeSegmentTimes(48000, 0, 24000, 48000.0);
    const auto atEnd = describeSegmentTimes(48000, 24000, 48000, 48000.0);
    assert(atStart.valid && atEnd.valid);
    assert(near(atStart.beforeMs, 0.0) && near(atStart.afterMs, 500.0));
    assert(near(atEnd.beforeMs, 500.0) && near(atEnd.afterMs, 0.0));

    // Same source frame interval, multiple standard sample rates.
    for (const double rate : {44100.0, 48000.0, 88200.0,
                              96000.0, 192000.0})
    {
        const auto time = describeSegmentTimes(96000, 24000, 48000, rate);
        assert(time.valid);
        assert(near(time.beforeMs, 24000000.0 / rate));
        assert(near(time.afterMs, 48000000.0 / rate));
    }

    // Moving a loop at fixed length changes only surrounding durations.
    const auto moved = describeSegmentTimes(96000, 48000, 72000, 48000.0);
    assert(moved.valid);
    assert(near(moved.beforeMs, 1000.0));
    assert(near(moved.afterMs, 500.0));

    // Minimum permitted loop is 11 frames.
    const auto minLoop = describeSegmentTimes(100, 10, 21, 1000.0);
    assert(minLoop.valid && minLoop.beforeFrames == 10);
    assert(minLoop.afterFrames == 79);
    assert(near(minLoop.beforeMs, 10.0) && near(minLoop.afterMs, 79.0));

    // Do not overflow on very large supported frame counts.
    constexpr int maxFrames = std::numeric_limits<int>::max();
    const auto longWav = describeSegmentTimes(maxFrames, 1000000000,
                                              2000000000, 44100.0);
    assert(longWav.valid && longWav.beforeFrames == 1000000000);
    assert(longWav.afterFrames == maxFrames - 2000000000);
    assert(std::isfinite(longWav.beforeMs) && std::isfinite(longWav.afterMs));

    // Invalid ranges and missing/non-finite WAV source sample rates.
    assert(!describeSegmentTimes(0, 0, 0, 48000.0).valid);
    assert(!describeSegmentTimes(11, 0, 11, 48000.0).valid);
    assert(!describeSegmentTimes(100, -1, 50, 48000.0).valid);
    assert(!describeSegmentTimes(100, 50, 101, 48000.0).valid);
    assert(!describeSegmentTimes(100, 50, 40, 48000.0).valid);
    assert(!describeSegmentTimes(100, 10, 20, 48000.0).valid);
    assert(!describeSegmentTimes(100, 10, 21, 0.0).valid);
    assert(!describeSegmentTimes(100, 10, 21, -48000.0).valid);
    assert(!describeSegmentTimes(100, 10, 21,
           std::numeric_limits<double>::quiet_NaN()).valid);
    assert(!describeSegmentTimes(100, 10, 21,
           std::numeric_limits<double>::infinity()).valid);
    assert(!describeSegmentTimes(maxFrames, 1000000000, 2000000000,
           std::numeric_limits<double>::denorm_min()).valid);

    std::cout << "v0.35 segment times: source rate and bounds OK\n";
    return 0;
}
