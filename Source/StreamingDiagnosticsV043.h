#pragma once
// v0.43: aggregate counters of streamed zones; this header does not depend on JUCE.
#include "StreamingHealthV042.h"
#include <cstdint>

namespace liberty::stream_v043 {
struct Diagnostics {
    std::uint32_t streamedZones = 0;
    stream_v042::HealthSnapshot totals {};

    void add(const stream_v042::HealthSnapshot& s) noexcept {
        ++streamedZones;
        totals.cacheHits += s.cacheHits;
        totals.cacheMisses += s.cacheMisses;
        totals.queuedPages += s.queuedPages;
        totals.droppedPages += s.droppedPages;
        totals.publishedPages += s.publishedPages;
        totals.decoderErrors += s.decoderErrors;
        totals.urgentQueued += s.urgentQueued;
        totals.urgentServiced += s.urgentServiced;
        totals.urgentDropped += s.urgentDropped;
        totals.underflowBlocks += s.underflowBlocks;
    }
    bool hasStreamedZones() const noexcept { return streamedZones != 0; }
    bool hasMeasurements() const noexcept {
        return totals.cacheHits != 0 || totals.cacheMisses != 0;
    }
    bool hasIncidents() const noexcept {
        return hasStreamedZones() && totals.needsAttention();
    }
    double hitRatePercent() const noexcept {
        return totals.hitRatePercent();
    }
};
} // namespace liberty::stream_v043
