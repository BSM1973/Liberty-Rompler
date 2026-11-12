#pragma once
// Liberty v0.39: cache de pages PCM non bloquant pour le fil audio.
// Production audio: tryReadFrame/requestPage only. Offline worker: publishPage/pollRequest.
// This is an isolated foundation, NOT YET CONNECTED to JUCE SampleVoice.
#include <array>
#include <atomic>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <thread>

namespace liberty::stream_v039 {

template <std::size_t PageFrames = 4096, std::size_t PageSlots = 16,
          std::size_t RequestSlots = 64>
class PageCache final {
    static_assert(PageFrames >= 2 && PageSlots > 0 && RequestSlots > 0,
                  "Invalid streaming cache configuration");
    struct Slot final {
        // State transition uses SC atomics together with reader pins.
        // A worker publishes one page at a time, exclusively.
        std::atomic<unsigned> readers {0};
        std::atomic<unsigned> state {0}; // 0=empty, 1=writer, 2=ready
        std::atomic<std::int64_t> page {-1};
        std::array<float, PageFrames> left {};
        std::array<float, PageFrames> right {};
        std::size_t valid = 0; // accessed only with the pin+state protocol
    };

public:
    PageCache() : pages(std::make_unique<Slot[]>(PageSlots)) {
        for (auto& req : requests) req.store(-1, std::memory_order_relaxed);
    }
    PageCache(const PageCache&) = delete;
    PageCache& operator=(const PageCache&) = delete;
    static constexpr std::size_t framesPerPage = PageFrames;
    static constexpr std::size_t capacityPages = PageSlots;

    // Real-time safe: no locks, no heap, no file operations, no waiting.
    bool tryReadFrame(std::int64_t absoluteFrame, float& left, float& right) noexcept {
        left = right = 0.0f;
        if (absoluteFrame < 0) { misses.fetch_add(1, std::memory_order_relaxed); return false; }
        const auto index = absoluteFrame / static_cast<std::int64_t>(PageFrames);
        const auto offset = static_cast<std::size_t>(absoluteFrame % static_cast<std::int64_t>(PageFrames));
        for (std::size_t i = 0; i < PageSlots; ++i) {
            auto& slot = pages[i];
            if (slot.page.load(std::memory_order_seq_cst) != index ||
                slot.state.load(std::memory_order_seq_cst) != 2)
                continue;
            slot.readers.fetch_add(1, std::memory_order_seq_cst);
            // Recheck after the pin to avoid reading data while eviction starts.
            if (slot.state.load(std::memory_order_seq_cst) == 2 &&
                slot.page.load(std::memory_order_seq_cst) == index &&
                offset < slot.valid) {
                left = slot.left[offset];
                right = slot.right[offset];
                slot.readers.fetch_sub(1, std::memory_order_seq_cst);
                hits.fetch_add(1, std::memory_order_relaxed);
                return true;
            }
            slot.readers.fetch_sub(1, std::memory_order_seq_cst);
        }
        misses.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    // Approximate residency check: atomic metadata only, no buffer access.
    // A concurrent eviction can invalidate the result immediately, which is
    // acceptable for scheduling, never for reading sample data.
    bool isPageResident(std::int64_t pageIndex) const noexcept {
        if (pageIndex < 0) return false;
        for (std::size_t i = 0; i < PageSlots; ++i) {
            const auto& slot = pages[i];
            if (slot.state.load(std::memory_order_seq_cst) == 2 &&
                slot.page.load(std::memory_order_seq_cst) == pageIndex)
                return true;
        }
        return false;
    }

    // Real-time safe MPSC request mailbox. If full, the caller must keep playing
    // its prefetched attack or use a controlled underflow, never block for disk.
    bool requestPage(std::int64_t pageIndex) noexcept {
        if (pageIndex < 0) return false;
        for (std::size_t i = 0; i < PageSlots; ++i) {
            if (pages[i].page.load(std::memory_order_relaxed) == pageIndex &&
                pages[i].state.load(std::memory_order_seq_cst) == 2)
                return true;
        }
        // Repeated real-time requests can race but never overwrite a pending one.
        for (auto& slot : requests)
            if (slot.load(std::memory_order_relaxed) == pageIndex)
                return true;
        const auto start = requestCursor.fetch_add(1, std::memory_order_relaxed);
        for (std::size_t k = 0; k < RequestSlots; ++k) {
            auto& slot = requests[(start + k) % RequestSlots];
            std::int64_t freeValue = -1;
            if (slot.compare_exchange_strong(freeValue, pageIndex,
                                             std::memory_order_release,
                                             std::memory_order_relaxed)) {
                enqueued.fetch_add(1, std::memory_order_relaxed);
                return true;
            }
        }
        dropped.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    // Worker ONLY. Poll and decode file data outside the realtime thread.
    bool pollRequest(std::int64_t& pageIndex) noexcept {
        const auto start = workerCursor++ % RequestSlots;
        for (std::size_t k = 0; k < RequestSlots; ++k) {
            const auto candidate = requests[(start + k) % RequestSlots].exchange(
                -1, std::memory_order_acq_rel);
            if (candidate >= 0) { pageIndex = candidate; return true; }
        }
        return false;
    }

    // Worker ONLY; data buffers must be decoded and ready before calling.
    // Multiple concurrent publishers are NOT supported.
    bool publishPage(std::int64_t pageIndex, const float* srcLeft,
                     const float* srcRight, std::size_t validFrames) noexcept {
        if (pageIndex < 0 || !srcLeft || !srcRight || validFrames == 0 ||
            validFrames > PageFrames) return false;
        std::size_t victim = PageSlots;
        for (std::size_t i = 0; i < PageSlots; ++i) {
            if (pages[i].state.load(std::memory_order_seq_cst) == 2 &&
                pages[i].page.load(std::memory_order_seq_cst) == pageIndex)
                return true;
            if (victim == PageSlots && pages[i].state.load(std::memory_order_seq_cst) == 0)
                victim = i;
        }
        if (victim == PageSlots) victim = nextVictim++ % PageSlots;
        auto& slot = pages[victim];
        slot.state.store(1, std::memory_order_seq_cst); // prohibit future readers
        // Worker can yield; audio thread must NEVER wait for these pins.
        while (slot.readers.load(std::memory_order_seq_cst) != 0)
            std::this_thread::yield();
        slot.page.store(-1, std::memory_order_seq_cst);
        std::copy_n(srcLeft, validFrames, slot.left.begin());
        std::copy_n(srcRight, validFrames, slot.right.begin());
        slot.valid = validFrames;
        slot.page.store(pageIndex, std::memory_order_seq_cst);
        slot.state.store(2, std::memory_order_seq_cst);
        publications.fetch_add(1, std::memory_order_relaxed);
        return true;
    }

    struct Stats { std::uint64_t hit, miss, request, droppedRequest, published; };
    Stats stats() const noexcept {
        return {hits.load(std::memory_order_relaxed),
                misses.load(std::memory_order_relaxed),
                enqueued.load(std::memory_order_relaxed),
                dropped.load(std::memory_order_relaxed),
                publications.load(std::memory_order_relaxed)};
    }
private:
    std::unique_ptr<Slot[]> pages;
    std::array<std::atomic<std::int64_t>, RequestSlots> requests;
    std::atomic<std::size_t> requestCursor {0};
    std::size_t workerCursor = 0, nextVictim = 0;
    std::atomic<std::uint64_t> hits {0}, misses {0}, enqueued {0},
                               dropped {0}, publications {0};
};
} // namespace liberty::stream_v039
