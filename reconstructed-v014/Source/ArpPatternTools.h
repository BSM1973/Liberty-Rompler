#pragma once
#include "Arpeggiator.h"
#include <array>
#include <cstdint>

namespace liberty {
namespace arp_patterns {

struct StepValue {
    int velocity = 100;
    int transpose = 0;
    bool enabled = true;
};

inline StepValue read(const ArpStep& step) noexcept {
    return {step.velocity.load(std::memory_order_relaxed),
            step.transpose.load(std::memory_order_relaxed),
            step.enabled.load(std::memory_order_relaxed)};
}

inline void write(ArpStep& step, StepValue value) noexcept {
    step.velocity.store(juce::jlimit(1, 127, value.velocity), std::memory_order_relaxed);
    step.transpose.store(juce::jlimit(-24, 24, value.transpose), std::memory_order_relaxed);
    step.enabled.store(value.enabled, std::memory_order_relaxed);
}

inline void reset(ArpControls& controls) noexcept {
    for (auto& step : controls.pattern) write(step, {});
    controls.steps.store(16, std::memory_order_release);
}

// Six original musical starting points. All data remains stored in the existing
// 32-step state so v0.8 DAW projects and preset serialization stay compatible.
// 1 Straight, 2 House, 3 Trance, 4 Bass, 5 Syncopated, 6 Melodic rise.
inline void applyTemplate(ArpControls& controls, int templateId) noexcept {
    const int id = juce::jlimit(1, 6, templateId);
    const int count = (id == 6) ? 32 : 16;
    static constexpr int majorScale[8] {0, 2, 4, 5, 7, 9, 11, 12};
    for (int i = 0; i < 32; ++i) {
        StepValue step {};
        switch (id) {
            case 1: // Straight sixteenths
                step.velocity = ((i % 4) == 0) ? 118 : 92;
                break;
            case 2: // Four-on-the-floor accents
                step.velocity = ((i % 4) == 0) ? 123 : 78;
                break;
            case 3: // Trance: octave accents, a couple of rests
                step.velocity = (i % 4 == 0) ? 120 : 90;
                step.transpose = (i % 8 == 4) ? 12 : 0;
                step.enabled = (i % 16 != 7 && i % 16 != 15);
                break;
            case 4: // Bass groove, syncopated rests
                step.enabled = (i % 8 != 3 && i % 8 != 6);
                step.velocity = (i % 4 == 0) ? 126 : 86;
                step.transpose = (i % 8 == 5) ? -12 : ((i % 8 == 7) ? 7 : 0);
                break;
            case 5: // Offbeat pulse
                step.enabled = (i % 8 == 0 || i % 8 == 3 || i % 8 == 6);
                step.velocity = (i % 8 == 0) ? 124 : 96;
                break;
            case 6: // 32-step diatonic ascent
                step.velocity = (i % 8 == 0) ? 125 : 98;
                step.transpose = majorScale[i % 8] + ((i / 8) % 2 ? 0 : -12);
                break;
        }
        write(controls.pattern[(size_t)i], step);
    }
    controls.steps.store(count, std::memory_order_release);
}

inline void reverse(ArpControls& controls) noexcept {
    const int n = juce::jlimit(1, 32, controls.steps.load(std::memory_order_relaxed));
    std::array<StepValue, 32> values {};
    for (int i = 0; i < n; ++i) values[(size_t)i] = read(controls.pattern[(size_t)i]);
    for (int i = 0; i < n; ++i) write(controls.pattern[(size_t)i], values[(size_t)(n - 1 - i)]);
}

// Circular step-shift: last step becomes first. No allocation or change of pitch range.
inline void rotate(ArpControls& controls) noexcept {
    const int n = juce::jlimit(1, 32, controls.steps.load(std::memory_order_relaxed));
    const auto last = read(controls.pattern[(size_t)(n - 1)]);
    for (int i = n - 1; i > 0; --i)
        write(controls.pattern[(size_t)i], read(controls.pattern[(size_t)(i - 1)]));
    write(controls.pattern[0], last);
}

// Humanize ONLY the velocity, not timing/pitch, for predictable DAW sync.
// Deterministic xorshift allows test coverage and reproducible results by seed.
inline void humanize(ArpControls& controls, std::uint32_t seed) noexcept {
    const int n = juce::jlimit(1, 32, controls.steps.load(std::memory_order_relaxed));
    std::uint32_t value = seed == 0 ? 0x7239a1b5u : seed;
    for (int i = 0; i < n; ++i) {
        value ^= value << 13;
        value ^= value >> 17;
        value ^= value << 5;
        auto step = read(controls.pattern[(size_t)i]);
        step.velocity += (int)(value % 25u) - 12;
        write(controls.pattern[(size_t)i], step);
    }
}

} // namespace arp_patterns
} // namespace liberty
