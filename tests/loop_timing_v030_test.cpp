#include "../Source/LoopTimingV030.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using liberty::loop_v030::describe;

static bool closeEnough(double left, double right, double tolerance = 1e-7)
{
    return std::abs(left - right) <= tolerance;
}

int main()
{
    // Half-second loop at the real source file rate.
    const auto typical = describe(96000, 24000, 48000, 2400, 48000.0);
    assert(typical.valid && typical.durationFrames == 24000);
    assert(typical.fadeFrames == 2400);
    assert(closeEnough(typical.durationMs, 500.0));
    assert(closeEnough(typical.fadeMs, 50.0));

    // 44.1 kHz WAVs must not be converted assuming 48 kHz.
    const auto cdRate = describe(44100, 0, 44100, 441, 44100.0);
    assert(cdRate.valid && closeEnough(cdRate.durationMs, 1000.0));
    assert(closeEnough(cdRate.fadeMs, 10.0));

    // Inclusive/exclusive convention: end-begin, NOT end-begin+1.
    const auto minimum = describe(100, 10, 21, 0, 48000.0);
    assert(minimum.valid && minimum.durationFrames == 11);
    assert(closeEnough(minimum.durationMs, 11.0 / 48.0));
    assert(minimum.fadeMs == 0.0);

    // A loop ending exactly on the last exclusive WAV frame is legal.
    const auto ending = describe(96000, 95900, 96000, 40, 96000.0);
    assert(ending.valid && ending.durationFrames == 100);
    assert(closeEnough(ending.durationMs, 100.0 / 96.0));

    // Other valid rates: 88.2 kHz, 192 kHz and non-integer rates.
    assert(closeEnough(describe(88200, 0, 88200, 0, 88200.0).durationMs, 1000.0));
    assert(closeEnough(describe(192000, 0, 192000, 0, 192000.0).durationMs, 1000.0));
    assert(describe(1000, 0, 1000, 0, 48000.5).valid);

    // Long WAV calculations use int64 for frame length.
    constexpr int max = std::numeric_limits<int>::max();
    const auto longFile = describe(max, 0, max, max / 4, 44100.0);
    assert(longFile.valid && longFile.durationFrames == max);
    assert(longFile.durationMs > 48000.0);
    assert(longFile.fadeMs > 10000.0);

    // Missing/invalid audio must not render fake duration values.
    assert(!describe(0, 0, 0, 0, 44100.0).valid);
    assert(!describe(11, 0, 11, 0, 44100.0).valid);
    assert(!describe(100, -1, 30, 0, 44100.0).valid);
    assert(!describe(100, 10, 20, 0, 44100.0).valid);
    assert(!describe(100, 30, 20, 0, 44100.0).valid);
    assert(!describe(100, 10, 101, 0, 44100.0).valid);
    assert(!describe(100, 10, 30, -1, 44100.0).valid);
    assert(!describe(100, 10, 30, 21, 44100.0).valid);
    assert(!describe(100, 10, 30, 1, 0.0).valid);
    assert(!describe(100, 10, 30, 1, -44100.0).valid);
    assert(!describe(100, 10, 30, 1,
                     std::numeric_limits<double>::infinity()).valid);
    assert(!describe(100, 10, 30, 1,
                     std::numeric_limits<double>::quiet_NaN()).valid);

    std::cout << "v0.30 timing: WAV sample rate, frames, ms and fade OK\n";
}
