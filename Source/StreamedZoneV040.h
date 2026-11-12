#pragma once
// Liberty ROMpler v0.41: shared worker pool for long stereo WAV playback.
// No disk access, allocation or mutex in audio callback methods.
// v0.40's one-worker-per-zone implementation is replaced with four lanes
// shared across all instances of the plugin module.
#include "JuceStreamingReaderV039.h"
#include "StreamingPageCacheV039.h"
#include "StreamingPoolV041.h"
#include <JuceHeader.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <memory>

namespace liberty::stream_v040 {
class StreamedZone final : public liberty::stream_v041::ZoneService {
public:
    static constexpr std::size_t pageFrames = 4096;
    static constexpr std::size_t cachePages = 32;
    using Cache = liberty::stream_v039::PageCache<pageFrames, cachePages, 96>;
    using Reader = liberty::stream_v039::JuceStreamingReader;

    explicit StreamedZone(std::unique_ptr<juce::AudioFormatReader> source)
        : reader(std::move(source), static_cast<int>(pageFrames)) {}

    StreamedZone(const StreamedZone&) = delete;
    StreamedZone& operator=(const StreamedZone&) = delete;
    ~StreamedZone() override {
        // Off the audio thread; waits for any in-flight worker disk read.
        if (registered)
            liberty::stream_v041::StreamingPool::instance().unregisterZone(this);
    }

    bool isValid() const noexcept { return reader.isValid(); }
    std::int64_t totalFrames() const noexcept { return reader.totalFrames(); }

    // Called on the background / preset-loading thread, never from audio.
    // Explicit failure avoids creating a silently unplayable zone if all
    // 64 service slots are occupied.
    bool start(std::int64_t attackEnd) {
        if (!isValid() || registered) return false;
        requestAhead(attackEnd);
        registered = liberty::stream_v041::StreamingPool::instance()
                         .registerZone(this);
        return registered;
    }

    // Audio callback operations: bounded cache reads/atomic page requests.
    bool tryReadFrame(std::int64_t frame, float& l, float& r) noexcept {
        return cache.tryReadFrame(frame, l, r);
    }
    void requestAhead(std::int64_t frame) noexcept {
        if (frame < 0 || frame >= totalFrames()) return;
        const auto page = frame / static_cast<std::int64_t>(pageFrames);
        for (std::int64_t lookahead = 0; lookahead < 4; ++lookahead) {
            if ((page + lookahead) * static_cast<std::int64_t>(pageFrames)
                < totalFrames())
                (void)cache.requestPage(page + lookahead);
        }
    }
    Cache::Stats stats() const noexcept { return cache.stats(); }
    std::uint64_t errors() const noexcept {
        return readErrors.load(std::memory_order_relaxed);
    }

    // Called from the single assigned pool lane, never the audio thread.
    // No other worker can read this decoder or publish this cache at once.
    bool serviceOneRequest() noexcept override {
        std::int64_t page=-1;
        if (!cache.pollRequest(page)) return false;
        if (loadPage(page)) {
            const auto lastPage = (totalFrames()-1) /
                                  static_cast<std::int64_t>(pageFrames);
            if (page < lastPage)
                (void)loadPage(page+1);
        }
        return true;
    }

private:
    bool loadPage(std::int64_t page) noexcept {
        const auto frames = totalFrames();
        if (page < 0 || frames <= 0 ||
            page > (frames-1)/static_cast<std::int64_t>(pageFrames))
            return false;
        const auto first = page * static_cast<std::int64_t>(pageFrames);
        const auto count = static_cast<std::size_t>(
            std::min<std::int64_t>(static_cast<std::int64_t>(pageFrames),
                                   frames-first));
        if (!reader.readFrames(first, count, left.data(), right.data())) {
            readErrors.fetch_add(1, std::memory_order_relaxed);
            return false;
        }
        return cache.publishPage(page, left.data(), right.data(), count);
    }

    Reader reader;   // one decoder, owned by this zone, worker-only access
    Cache cache;     // fixed-size pages and mailbox; read from audio
    std::array<float, pageFrames> left {}, right {}; // worker-only scratch
    std::atomic<std::uint64_t> readErrors {0};
    bool registered = false; // load/unload thread only
};
} // namespace liberty::stream_v040
