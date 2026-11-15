#pragma once
// Liberty ROMpler v0.55: single-producer single-consumer spectral PCM transport.
// Producer: ONE dedicated render worker. Consumer: ONE audio thread.
// No allocations, mutexes, sleeps or file access in push/pop. Call reset()
// only before either thread starts. A generation number invalidates old presets.
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cmath>

namespace liberty::spectral_dna_v055 {
class SpectralPcmQueue final {
public:
    static constexpr std::size_t kFrameSamples=64;
    static constexpr std::size_t kFrames=32;
    struct Frame {
        std::array<float,kFrameSamples> mono{};
        std::uint64_t firstSample=0;
        std::uint32_t generation=0;
    };
    SpectralPcmQueue()=default;
    SpectralPcmQueue(const SpectralPcmQueue&)=delete;
    SpectralPcmQueue& operator=(const SpectralPcmQueue&)=delete;
    void resetBeforeStart() noexcept {
        readIndex.store(0,std::memory_order_relaxed);
        writeIndex.store(0,std::memory_order_relaxed);
    }
    bool push(const Frame& frame) noexcept {
        const auto w=writeIndex.load(std::memory_order_relaxed);
        if(w-readIndex.load(std::memory_order_acquire)>=kFrames) return false;
        data[std::size_t(w%kFrames)]=frame;
        writeIndex.store(w+1,std::memory_order_release);
        return true;
    }
    bool pop(Frame& result) noexcept {
        const auto r=readIndex.load(std::memory_order_relaxed);
        if(r==writeIndex.load(std::memory_order_acquire)) return false;
        result=data[std::size_t(r%kFrames)];
        readIndex.store(r+1,std::memory_order_release);
        return true;
    }
    std::size_t availableApprox() const noexcept {
        const auto w=writeIndex.load(std::memory_order_acquire);
        const auto r=readIndex.load(std::memory_order_acquire);
        return std::size_t(w-r);
    }
private:
    std::array<Frame,kFrames> data{};
    alignas(64) std::atomic<std::uint64_t> readIndex{0};
    alignas(64) std::atomic<std::uint64_t> writeIndex{0};
};

// Consumer-owned cursor. Pop has bounded work per sample: at most one frame.
// Out-of-date timestamps and old preset generations must NOT be replayed.
// If one frame is missing, return false and let the dry/wet fade handle it.
class SpectralPcmCursor final {
public:
    void resetBeforeStart() noexcept { offset=kFrameSamples; }
    bool next(SpectralPcmQueue& queue,std::uint32_t activeGeneration,
              std::uint64_t expectedSample,float& output) noexcept {
        if(offset==kFrameSamples) {
            if(!queue.pop(current)) return false;
            offset=0;
        }
        const bool valid=current.generation==activeGeneration &&
                         current.firstSample+offset==expectedSample;
        const float sample=current.mono[offset++];
        if(!valid || !std::isfinite(sample)) {
            offset=kFrameSamples; // reject remainder of stale/misaligned frame
            return false;
        }
        output=sample;
        return true;
    }
private:
    SpectralPcmQueue::Frame current{};
    std::size_t offset=kFrameSamples;
};
} // namespace liberty::spectral_dna_v055
