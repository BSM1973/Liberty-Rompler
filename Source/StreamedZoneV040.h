#pragma once
// Liberty ROMpler v0.40 - opt-in long sample streaming, real-time side only.
// JUCE decoder and disk I/O run exclusively in ReadAheadWorker's background thread.
// One worker per streamed zone in this first integration; a shared bank pool is a
// subsequent performance optimization, not a capability of this implementation.
#include "JuceStreamingReaderV039.h"
#include "StreamingPageCacheV039.h"
#include "StreamingReadAheadV039.h"
#include <JuceHeader.h>
#include <algorithm>
#include <cstdint>
#include <memory>

namespace liberty::stream_v040 {
class StreamedZone final {
public:
    static constexpr std::size_t pageFrames = 4096;
    static constexpr std::size_t cachePages = 32;
    using Cache = liberty::stream_v039::PageCache<pageFrames, cachePages, 96>;
    using Reader = liberty::stream_v039::JuceStreamingReader;
    using Worker = liberty::stream_v039::ReadAheadWorker<
        Reader, pageFrames, cachePages, 96>;

    explicit StreamedZone(std::unique_ptr<juce::AudioFormatReader> source)
        : reader(std::move(source), static_cast<int>(pageFrames)),
          cache(), worker(cache, reader, reader.totalFrames()) {}

    StreamedZone(const StreamedZone&) = delete;
    StreamedZone& operator=(const StreamedZone&) = delete;

    bool isValid() const noexcept { return reader.isValid(); }
    std::int64_t totalFrames() const noexcept { return reader.totalFrames(); }
    void start(std::int64_t attackEnd) {
        if (!isValid()) return;
        // Warm the next two pages BEFORE handing the instrument to the synthesiser.
        requestAhead(attackEnd);
        worker.start();
    }

    // Called in the audio callback. Only atomics + preallocated cache are touched.
    // Both channels are read at once; avoids tearing between channels.
    bool tryReadFrame(std::int64_t frame, float& l, float& r) noexcept {
        return cache.tryReadFrame(frame, l, r);
    }

    // Audio callback: bounded enqueue only. No waiting for worker or allocation.
    void requestAhead(std::int64_t frame) noexcept {
        if (frame < 0 || frame >= totalFrames()) return;
        const auto page = frame / static_cast<std::int64_t>(pageFrames);
        for (std::int64_t lookahead = 0; lookahead != 4; ++lookahead) {
            if ((page + lookahead) * static_cast<std::int64_t>(pageFrames)
                < totalFrames())
                (void) cache.requestPage(page + lookahead);
        }
    }

    Cache::Stats stats() const noexcept { return cache.stats(); }
    std::uint64_t errors() const noexcept { return worker.readErrors(); }
private:
    // Order is critical: worker must stop/join before decoder and cache die.
    Reader reader;
    Cache cache;
    Worker worker;
};
} // namespace liberty::stream_v040
