#pragma once
// Liberty ROMpler v0.37 - pure C++17 RR, articulation and velocity policy.
// Used only with optional zone fields; all existing v1 banks remain unchanged.
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace liberty::multisample_v037
{
inline int midi(int value) noexcept { return std::max(1, std::min(127, value)); }

inline float saturate(float value) noexcept
{
    return std::max(0.0f, std::min(1.0f, value));
}

inline float velocityWeight(int midiVelocity, int low, int high, int fade) noexcept
{
    if (midiVelocity < 1 || midiVelocity > 127 || low < 1 || high > 127
        || low > high || fade < 0 || fade > 32)
        return 0.0f;
    if (fade == 0)
        return (midiVelocity >= low && midiVelocity <= high) ? 1.0f : 0.0f;
    if (midiVelocity < low - fade || midiVelocity > high + fade)
        return 0.0f;
    const float edge = static_cast<float>(fade) * 2.0f;
    const float enter = low <= 1 ? 1.0f : saturate(
        (static_cast<float>(midiVelocity) - (static_cast<float>(low) - fade - 0.5f)) / edge);
    const float exit = high >= 127 ? 1.0f : saturate(
        ((static_cast<float>(high) + fade + 0.5f) - midiVelocity) / edge);
    return enter * exit;
}

inline bool articulationAllowed(int zoneArticulation, int activeArticulation) noexcept
{
    return zoneArticulation == 0 || zoneArticulation == activeArticulation;
}

struct RoundRobin
{
    // Fixed-size, no allocation/locks in the real-time MIDI callback.
    std::array<std::array<std::uint8_t, 17>, 128> next {};

    void reset() noexcept
    {
        for (auto& byGroup : next) byGroup.fill(0);
    }

    int select(int note, int group, std::uint32_t variants) noexcept
    {
        if (note < 0 || note > 127 || group < 1 || group > 16)
            return 0;
        auto& cursor = next[static_cast<std::size_t>(note)]
                           [static_cast<std::size_t>(group)];
        for (int step=0; step<16; ++step)
        {
            const int take = (cursor + step) % 16 + 1;
            if (variants & (std::uint32_t{1} << take))
            {
                cursor = static_cast<std::uint8_t>(take % 16);
                return take;
            }
        }
        return 0;
    }
};
} // namespace liberty::multisample_v037
