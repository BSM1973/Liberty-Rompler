#pragma once
// Liberty v0.39: reader thread adapter for StreamingPageCacheV039.
// Reader::readFrames(int64_t first, size_t frames, float* l, float* r)
// is invoked exclusively by this background thread (never the realtime callback).
// The Reader owns any disk handles; it MUST outlive this worker.
// Prototype component NOT connected to JUCE SampleVoice yet.
#include "StreamingPageCacheV039.h"
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <thread>
#include <utility>
#include <algorithm>

namespace liberty::stream_v039 {
template <class Reader, std::size_t PageFrames = 4096,
          std::size_t PageSlots = 16, std::size_t RequestSlots = 64>
class ReadAheadWorker final {
public:
    using Cache = PageCache<PageFrames, PageSlots, RequestSlots>;
    ReadAheadWorker(Cache& c, Reader& reader, std::int64_t frames)
        : cache(c), source(reader), totalFrames(std::max<std::int64_t>(0, frames)) {}
    ReadAheadWorker(const ReadAheadWorker&) = delete;
    ReadAheadWorker& operator=(const ReadAheadWorker&) = delete;
    ~ReadAheadWorker() { stop(); }

    // Start/stop ONLY on UI/background (non-real-time) thread.
    void start() {
        if (!started.exchange(true, std::memory_order_acq_rel)) {
            stopping.store(false, std::memory_order_release);
            worker = std::thread([this] { run(); });
        }
    }
    void stop() noexcept {
        stopping.store(true, std::memory_order_release);
        if (worker.joinable()) worker.join();
        started.store(false, std::memory_order_release);
    }
    bool isRunning() const noexcept {
        return started.load(std::memory_order_relaxed) &&
               !stopping.load(std::memory_order_acquire);
    }
    std::uint64_t readErrors() const noexcept {
        return errors.load(std::memory_order_relaxed);
    }
private:
    Cache& cache;
    Reader& source;
    const std::int64_t totalFrames;
    std::thread worker;
    std::atomic<bool> started {false}, stopping {false};
    std::atomic<std::uint64_t> errors {0};
    std::array<float, PageFrames> left {}, right {};

    bool load(std::int64_t page) {
        if (page < 0 || page > (totalFrames-1) / static_cast<std::int64_t>(PageFrames) || totalFrames <= 0)
            return false;
        const auto first = page * static_cast<std::int64_t>(PageFrames);
        const auto count = static_cast<std::size_t>(std::min<std::int64_t>(
            static_cast<std::int64_t>(PageFrames), totalFrames - first));
        if (!source.readFrames(first, count, left.data(), right.data())) {
            errors.fetch_add(1, std::memory_order_relaxed);
            return false;
        }
        return cache.publishPage(page, left.data(), right.data(), count);
    }

    void run() {
        while (!stopping.load(std::memory_order_acquire)) {
            std::int64_t page=-1;
            if (!cache.pollRequest(page)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            if (stopping.load(std::memory_order_acquire)) break;
            if (load(page)) {
                // Read ahead one more page while the note is still in its
                // current page. Ignore if the next page is past EOF.
                if (page < (totalFrames-1) / static_cast<std::int64_t>(PageFrames))
                    (void)load(page+1);
            }
        }
    }
};
} // namespace liberty::stream_v039
