#include "../Source/LoopMarkerTimesV031.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using liberty::loop_v031::describePositions;

static bool close(double a, double b, double epsilon = 1e-8)
{
    return std::abs(a - b) <= epsilon;
}

int main()
{
    // Source timestamps, not time since the beginning of the loop.
    const auto standard = describePositions(96000, 24000, 48000, 48000.0);
    assert(standard.valid && standard.beginFrame == 24000);
    assert(standard.endFrame == 48000);
    assert(close(standard.beginMs, 500.0));
    assert(close(standard.endMs, 1000.0));

    // Correct WAV sample rate for 44.1, 88.2, 96 and 192 kHz.
    const auto compactDisc = describePositions(88200, 22050, 44100, 44100.0);
    assert(compactDisc.valid && close(compactDisc.beginMs, 500.0));
    assert(close(compactDisc.endMs, 1000.0));
    const auto high = describePositions(192000, 48000, 96000, 96000.0);
    assert(high.valid && close(high.beginMs, 500.0));
    assert(close(high.endMs, 1000.0));
    const auto hiRes = describePositions(384000, 96000, 192000, 192000.0);
    assert(hiRes.valid && close(hiRes.beginMs, 500.0));
    assert(close(hiRes.endMs, 1000.0));
    const auto dsd = describePositions(88200, 22050, 44100, 88200.0);
    assert(dsd.valid && close(dsd.beginMs, 250.0));
    assert(close(dsd.endMs, 500.0));

    // End is an exclusive boundary. Minimum 11 frames is legal.
    const auto minimum = describePositions(100, 10, 21, 48000.0);
    assert(minimum.valid && minimum.endFrame - minimum.beginFrame == 11);
    assert(close(minimum.beginMs, 10.0 / 48.0));
    assert(close(minimum.endMs, 21.0 / 48.0));
    const auto entire = describePositions(48000, 0, 48000, 48000.0);
    assert(entire.valid && close(entire.beginMs, 0.0));
    assert(close(entire.endMs, 1000.0));

    // Long WAVs use safe int64 intermediates.
    constexpr int limit = std::numeric_limits<int>::max();
    const auto large = describePositions(limit, limit - 2000, limit, 48000.0);
    assert(large.valid && large.endFrame == limit);
    assert(large.endMs > large.beginMs);
    assert(close(large.endMs - large.beginMs, 2000.0 / 48.0, 1e-6));

    // No fake timing for invalid samples, loops, or source rates.
    assert(!describePositions(0, 0, 0, 44100.0).valid);
    assert(!describePositions(11, 0, 11, 44100.0).valid);
    assert(!describePositions(100, -1, 30, 44100.0).valid);
    assert(!describePositions(100, 10, 20, 44100.0).valid);
    assert(!describePositions(100, 30, 20, 44100.0).valid);
    assert(!describePositions(100, 10, 101, 44100.0).valid);
    assert(!describePositions(100, 10, 30, 0.0).valid);
    assert(!describePositions(100, 10, 30, -1.0).valid);
    assert(!describePositions(100, 10, 30,
        std::numeric_limits<double>::infinity()).valid);
    assert(!describePositions(100, 10, 30,
        std::numeric_limits<double>::quiet_NaN()).valid);

    std::cout << "v0.31 marker times: source WAV rate, absolute ms, exclusive end OK\n";
}
