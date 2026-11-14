#pragma once
// Liberty v0.49: bounded non-RT warmup for distant SLICE starts.
// Pure C++17; no decoder, allocation or IO on the audio thread.
#include "CreativeEngineV038.h"
#include <algorithm>
#include <cmath>

namespace liberty::stream_v049 {
struct SliceWindow { int begin=0, frames=0; };

inline SliceWindow planSliceWarmup(int totalFrames, int sliceIndex,
                                   double sourceRate) noexcept {
    // Slice zero already has a 750 ms attack cache from the legacy loader.
    if (sliceIndex <= 0 || sliceIndex >= 8 || totalFrames < 16 ||
        !std::isfinite(sourceRate) || sourceRate <= 0.0)
        return {};
    const auto region = liberty::creative_v038::slice(totalFrames-1, sliceIndex);
    if (!region.valid) return {};
    // Max 0.30 s, 16,384 frames, per source slice head; bounded total RAM.
    const int requested = std::max(512, std::min(16384,
        static_cast<int>(std::min(16384.0, sourceRate*0.30))));
    // The extra interpolator frame is included, without crossing the region.
    const int available = std::min(totalFrames-region.begin,
                                   region.end-region.begin);
    return {region.begin, std::min(requested+1, available)};
}
} // namespace liberty::stream_v049
