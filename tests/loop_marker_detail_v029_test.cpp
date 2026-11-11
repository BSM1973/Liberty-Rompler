#include "../Source/LoopMarkerDetailV029.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using liberty::loop_v029::planPrecisionZoom;

int main()
{
    // Ordinary WAVs aim for roughly 512 visible frames.
    const auto close = planPrecisionZoom(16000, 6000, 1.0, 64.0);
    assert(close.valid && close.markerFrame == 6000);
    assert(std::abs(close.requestedZoom - 31.25) < 1e-9);

    // A longer WAV is limited to the application's maximum zoom.
    const auto limited = planPrecisionZoom(1000000, 1000000, 1.0, 64.0);
    assert(limited.valid && limited.requestedZoom == 64.0);
    assert(limited.markerFrame == 1000000); // valid exclusive end

    // Preserve an existing tighter zoom: DETAIL never zooms out.
    const auto existing = planPrecisionZoom(16000, 10000, 48.0, 64.0);
    assert(existing.valid && existing.requestedZoom == 48.0);
    const auto maximum = planPrecisionZoom(16000, 10000, 64.0, 64.0);
    assert(maximum.valid && maximum.requestedZoom == 64.0);

    // Tiny files cannot be magnified beyond what is useful.
    const auto tiny = planPrecisionZoom(12, 0, 1.0, 64.0);
    assert(tiny.valid && tiny.requestedZoom == 1.0);
    const auto smallerZoom = planPrecisionZoom(4096, 10, 1.0, 2.0);
    assert(smallerZoom.valid && smallerZoom.requestedZoom == 2.0);

    // Custom desired window is supported in pure C++ engine.
    const auto custom = planPrecisionZoom(8192, 4096, 1.0, 64.0, 256);
    assert(custom.valid && custom.requestedZoom == 32.0);
    const auto fit = planPrecisionZoom(8192, 4096, 1.0, 64.0, 8192);
    assert(fit.valid && fit.requestedZoom == 1.0);

    // Frames and zoom limits remain valid for very long samples.
    constexpr int largest = std::numeric_limits<int>::max();
    const auto huge = planPrecisionZoom(largest, largest, 1.0, 64.0);
    assert(huge.valid && huge.requestedZoom == 64.0);
    const auto hugeCurrent = planPrecisionZoom(largest, 0, 63.0, 64.0);
    assert(hugeCurrent.valid && hugeCurrent.requestedZoom == 64.0);

    // Invalid marker, zoom, limits and window reject without side effects.
    assert(!planPrecisionZoom(11, 0, 1.0, 64.0).valid);
    assert(!planPrecisionZoom(1000, -1, 1.0, 64.0).valid);
    assert(!planPrecisionZoom(1000, 1001, 1.0, 64.0).valid);
    assert(!planPrecisionZoom(1000, 500, 0.9, 64.0).valid);
    assert(!planPrecisionZoom(1000, 500, 65.0, 64.0).valid);
    assert(!planPrecisionZoom(1000, 500, 1.0, 0.0).valid);
    assert(!planPrecisionZoom(1000, 500, 1.0, 64.0, 0).valid);
    assert(!planPrecisionZoom(1000, 500, 1.0, 64.0, -1).valid);
    assert(!planPrecisionZoom(1000, 500,
        std::numeric_limits<double>::infinity(), 64.0).valid);
    assert(!planPrecisionZoom(1000, 500, 1.0,
        std::numeric_limits<double>::quiet_NaN()).valid);

    std::cout << "v0.29 precision zoom: 512-frame target, zoom cap, no zoom-out OK\n";
}
