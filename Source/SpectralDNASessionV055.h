#pragma once
// Liberty ROMpler v0.55: latency and preset lifecycle control contract.
// setLatencySamples MUST only be called from the host-safe preparation path.
// The latency below is provisional and must be measured against v0.48's
// complete WOLA/worker scheduling before production activation.
#include <atomic>
#include <cstdint>
namespace liberty::spectral_dna_v055 {
class SessionControl final {
public:
    static constexpr int kPrototypeLatencySamples=256;
    // This method is NOT for the audio thread. Host callback policy and JUCE
    // prepareToPlay sequencing must be verified before actual integration.
    template<class Processor>
    void configureHostLatency(Processor& processor) {
        processor.setLatencySamples(kPrototypeLatencySamples);
    }
    std::uint32_t beginPresetChange() noexcept {
        // A monotonically increasing epoch invalidates outstanding frames.
        // Handle wraparound by stopping/reinitializing the audio engine before
        // 2^32 switches; ordinary project sessions never approach this.
        return generation.fetch_add(1,std::memory_order_acq_rel)+1;
    }
    std::uint32_t activeGeneration() const noexcept {
        return generation.load(std::memory_order_acquire);
    }
private:
    std::atomic<std::uint32_t> generation{1};
};
} // namespace liberty::spectral_dna_v055
