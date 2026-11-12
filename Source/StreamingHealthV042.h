#pragma once
// Liberty v0.42 - pure C++17 snapshot of streaming health counters.
// Read by the UI/diagnostic side only. Audio updates remain atomic.
#include <cstdint>

namespace liberty::stream_v042 {
struct HealthSnapshot {
    std::uint64_t cacheHits = 0;
    std::uint64_t cacheMisses = 0;
    std::uint64_t queuedPages = 0;
    std::uint64_t droppedPages = 0;
    std::uint64_t publishedPages = 0;
    std::uint64_t decoderErrors = 0;
    std::uint64_t urgentQueued = 0;
    std::uint64_t urgentServiced = 0;
    std::uint64_t urgentDropped = 0;
    std::uint64_t underflowBlocks = 0;

    double hitRatePercent() const noexcept {
        const auto total = cacheHits + cacheMisses;
        return total == 0 ? 100.0
            : 100.0 * static_cast<double>(cacheHits) /
              static_cast<double>(total);
    }
    bool needsAttention() const noexcept {
        return decoderErrors > 0 || urgentDropped > 0 ||
               droppedPages > 0 || underflowBlocks > 0;
    }
};
} // namespace liberty::stream_v042
