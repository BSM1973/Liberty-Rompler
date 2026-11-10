#pragma once
// Liberty ROMpler v0.16 - Memory-bounded WAV seam audition.
// The UI reads ONLY the beginning and end of the loop, never the whole WAV.
// This helper allocates. Do not use it inside processBlock().

#include "LoopEditorV015.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace liberty::loop_v016
{
using liberty::loop_v015::Frame;
using liberty::loop_v015::SeamPreview;

// tail[ch] = last N frames immediately BEFORE the exclusive loop end.
// head[ch] = first N frames starting AT the inclusive loop start.
// Identical result to v0.15 renderSeamPreview() on a complete decoded WAV,
// but memory consumption is O(channels * previewFrames), not O(WAV length).
inline SeamPreview renderSeamFromWindows(
    const std::vector<std::vector<float>>& tail,
    const std::vector<std::vector<float>>& head,
    Frame fadeFrames)
{
    SeamPreview out;
    if (tail.empty() || tail.size() != head.size() || fadeFrames < 0)
        return out;

    const auto n = tail[0].size();
    if (n == 0 || n > 32768 || static_cast<std::uint64_t>(fadeFrames) > n)
        return out;
    for (std::size_t channel = 0; channel < tail.size(); ++channel)
        if (tail[channel].size() != n || head[channel].size() != n)
            return out;

    const auto fade = static_cast<std::size_t>(fadeFrames);
    out.tailFrames = static_cast<Frame>(n);
    out.headFrames = static_cast<Frame>(n);
    out.overlapFrames = fadeFrames;
    out.channels.resize(tail.size());

    for (std::size_t channel = 0; channel < tail.size(); ++channel)
    {
        auto& dst = out.channels[channel];
        const auto& a = tail[channel];
        const auto& b = head[channel];
        dst.reserve(2 * n - fade);

        for (std::size_t i = 0; i < n - fade; ++i)
            dst.push_back(a[i]);

        for (std::size_t i = 0; i < fade; ++i)
        {
            const float fraction = static_cast<float>(i)
                                   / static_cast<float>(fade);
            // Exactly the v0.14 live engine's LoopCrossfade::at() curve.
            dst.push_back(a[n - fade + i] * (1.0f - fraction)
                          + b[i] * fraction);
        }

        for (std::size_t i = fade; i < n; ++i)
            dst.push_back(b[i]);
    }
    return out;
}
} // namespace liberty::loop_v016
