#include "../Source/LoopMarkerFocusV028.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>

using liberty::loop_v028::focusOnMarker;

int main()
{
    // A marker in a long WAV is centered while existing zoom is retained.
    const auto start = focusOnMarker(100000, 5000, 1000, 10000);
    const auto end = focusOnMarker(100000, 5000, 1000, 20000);
    assert(start.valid && start.firstFrame == 9500);
    assert(end.valid && end.firstFrame == 19500);
    assert(start.spanFrames == 1000 && end.spanFrames == 1000);
    assert(start.markerFrame == 10000 && end.markerFrame == 20000);

    // Near the file boundaries, constrain the view, not the marker.
    const auto atStart = focusOnMarker(100000, 5000, 1000, 0);
    const auto nearStart = focusOnMarker(100000, 5000, 1000, 100);
    const auto atEnd = focusOnMarker(100000, 5000, 1000, 100000);
    const auto nearEnd = focusOnMarker(100000, 5000, 1000, 99950);
    assert(atStart.valid && atStart.firstFrame == 0);
    assert(nearStart.valid && nearStart.firstFrame == 0);
    assert(atEnd.valid && atEnd.firstFrame == 99000);
    assert(nearEnd.valid && nearEnd.firstFrame == 99000);

    // Fit-all does not change; a repeated focus is idempotent.
    const auto full = focusOnMarker(100000, 0, 100000, 10000);
    assert(full.valid && full.firstFrame == 0 && full.spanFrames == 100000);
    const auto repeated = focusOnMarker(100000, start.firstFrame, 1000, 10000);
    assert(repeated.valid && repeated.firstFrame == start.firstFrame);

    // A minimum valid loop can still focus either endpoint.
    assert(focusOnMarker(12, 0, 12, 0).valid);
    assert(focusOnMarker(12, 0, 12, 12).valid);

    // Long WAVs must not overflow signed 32-bit during centering.
    constexpr int limit = std::numeric_limits<int>::max();
    const auto bigEnd = focusOnMarker(limit, 1500000000, 100000000,
                                      limit);
    const auto bigStart = focusOnMarker(limit, 0, 100000000, 0);
    assert(bigEnd.valid && bigEnd.firstFrame == limit - 100000000);
    assert(bigStart.valid && bigStart.firstFrame == 0);
    const auto nearLimit = focusOnMarker(limit, limit - 200000000,
                                          200000000, limit - 10);
    assert(nearLimit.valid
           && nearLimit.firstFrame == limit - 200000000);

    // Invalid source/viewport/marker coordinates are rejected.
    assert(!focusOnMarker(11, 0, 11, 0).valid);
    assert(!focusOnMarker(100000, -1, 1000, 5000).valid);
    assert(!focusOnMarker(100000, 99999, 1000, 5000).valid);
    assert(!focusOnMarker(100000, 0, 0, 5000).valid);
    assert(!focusOnMarker(100000, 0, 100001, 5000).valid);
    assert(!focusOnMarker(100000, 0, 1000, -1).valid);
    assert(!focusOnMarker(100000, 0, 1000, 100001).valid);

    std::cout << "v0.28 loop marker focus: precise center, zoom lock, bounds OK\n";
}
