#pragma once
// Liberty ROMpler v0.53: deterministic, allocation-free SLICE prefetch cadence.
// Preloaded RAM windows are immutable. A pending disk seek is refreshed at a
// bounded frequency; near the window edge the full v0.52 request path resumes.
#include <algorithm>
#include <cmath>

namespace liberty::slice_v053 {
class PrefetchCadence final {
public:
    void reset() noexcept { framesUntilRefresh=0; }
    bool shouldRequest(int warmEnd, double position, double nextBlockEnd,
                       int blockFrames, double sampleRate) noexcept {
        if (warmEnd <= 0 || !std::isfinite(position) ||
            !std::isfinite(nextBlockEnd) || !std::isfinite(sampleRate) ||
            sampleRate <= 0 || blockFrames <= 0 || nextBlockEnd <= position ||
            position >= warmEnd) {
            reset();
            return true; // old critical-request behavior outside RAM
        }
        const double playbackSpeed = std::clamp(
            (nextBlockEnd-position)/(double)blockFrames, 0.5, 4.0);
        // Demand aggressively before the warm window is exhausted, including
        // pitched or time-stretched notes. At 44.1 kHz this starts >= 0.38s
        // before the edge, unlike a hard switch at the last 4096 samples.
        const double lead = std::clamp(sampleRate*.38*playbackSpeed,
                                       8192.0, 65536.0);
        if (nextBlockEnd + lead >= warmEnd) {
            framesUntilRefresh=0;
            return true;
        }
        if (framesUntilRefresh <= 0) {
            framesUntilRefresh = std::clamp(
                (int)(sampleRate*.075), 512, 8192);
            return true;
        }
        framesUntilRefresh -= blockFrames;
        return false;
    }
private:
    int framesUntilRefresh = 0;
};
} // namespace liberty::slice_v053
