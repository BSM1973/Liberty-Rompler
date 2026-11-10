#pragma once
// Liberty ROMpler v0.15: helpers for frame-accurate, non-destructive loop editing.
// Independent from JUCE; UI may call these functions on the message thread.
// renderSeamPreview allocates: NEVER call it from the real-time audio callback.

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

namespace liberty::loop_v015
{
using Frame = std::int64_t;

struct Loop
{
    Frame begin = 0;       // inclusive
    Frame end = 1;         // exclusive
    Frame total = 1;
};

enum class Marker { begin, end };

inline bool isValid (const Loop& r, Frame minFrames = 1) noexcept
{
    return minFrames > 0 && r.total >= minFrames
        && r.begin >= 0 && r.begin <= r.total - minFrames
        && r.end >= r.begin + minFrames && r.end <= r.total;
}

inline Frame saturatingAdd (Frame a, Frame b) noexcept
{
    if (b > 0 && a > std::numeric_limits<Frame>::max() - b)
        return std::numeric_limits<Frame>::max();
    if (b < 0 && a < std::numeric_limits<Frame>::min() - b)
        return std::numeric_limits<Frame>::min();
    return a + b;
}

// Used by +/- one-frame or +/- ten-frame buttons and by keyboard arrows.
// A drag can first convert pointer position to a frame then call setMarker.
inline Loop setMarker (Loop r, Marker marker, Frame frame, Frame minFrames = 1) noexcept
{
    if (! isValid (r, minFrames))
        return r;
    if (marker == Marker::begin)
        r.begin = std::clamp (frame, Frame { 0 }, r.end - minFrames);
    else
        r.end = std::clamp (frame, r.begin + minFrames, r.total);
    return r;
}

inline Loop nudge (Loop r, Marker marker, Frame frames, Frame minFrames = 1) noexcept
{
    if (! isValid (r, minFrames))
        return r;
    return setMarker (r, marker,
                      saturatingAdd (marker == Marker::begin ? r.begin : r.end, frames),
                      minFrames);
}

struct Viewport
{
    Frame first = 0;
    Frame frames = 1;
    double left = 0.0;
    double width = 1.0;
};

inline double frameToX (Frame frame, Viewport view) noexcept
{
    if (view.frames <= 0 || view.width <= 0 || ! std::isfinite (view.width)
        || ! std::isfinite (view.left))
        return view.left;
    const auto position = (static_cast<long double> (frame)
                           - static_cast<long double> (view.first))
                          / static_cast<long double> (view.frames);
    return view.left + static_cast<double> (position) * view.width;
}

struct VisibleBand
{
    double x = 0.0;
    double width = 0.0;
    bool visible = false;
};

// Two bands: at the loop tail and at the loop head, both clipped to viewport.
inline VisibleBand band (Frame first, Frame last, Viewport view) noexcept
{
    if (last <= first || view.frames <= 0 || view.width <= 0)
        return {};
    const auto x1 = frameToX (first, view);
    const auto x2 = frameToX (last, view);
    const auto lo = std::max (view.left, std::min (x1, x2));
    const auto hi = std::min (view.left + view.width, std::max (x1, x2));
    return {lo, std::max (0.0, hi - lo), hi > lo};
}

struct FadeBands
{
    VisibleBand tail;
    VisibleBand head;
};

inline FadeBands crossfadeBands (Loop r, Frame fadeFrames, Viewport view) noexcept
{
    if (! isValid (r) || fadeFrames <= 0)
        return {};
    const auto length = r.end - r.begin;
    const auto fade = std::min (fadeFrames, length);
    return {band (r.end - fade, r.end, view),
            band (r.begin, r.begin + fade, view)};
}

struct SeamPreview
{
    // One vector per channel, samples rendered offline for UI audition.
    std::vector<std::vector<float>> channels;
    Frame tailFrames = 0;
    Frame headFrames = 0;
    Frame overlapFrames = 0;
};

// Renders only the transition around the loop seam. This is NOT a modification
// of the sample bank or DAW state. Playback must be delegated to a UI-owned,
// thread-safe preview player; never allocate or read files in the audio callback.
// Source channels have identical frame counts and must cover r.total frames.
inline SeamPreview renderSeamPreview (const std::vector<std::vector<float>>& source,
                                      Loop r, Frame contextFrames,
                                      Frame crossfadeFrames)
{
    SeamPreview out;
    if (! isValid (r) || source.empty() || contextFrames <= 0
        || crossfadeFrames < 0)
        return out;
    for (const auto& channel : source)
        if (static_cast<std::uint64_t> (r.total) > channel.size())
            return out;

    const Frame length = r.end - r.begin;
    const Frame context = std::min (contextFrames, length);
    const Frame overlap = std::min (crossfadeFrames, context);
    // Avoid signed overflow before calculating a potentially large preview.
    if (context > std::numeric_limits<Frame>::max() / 2)
        return out;
    const Frame resultFrames = context * 2 - overlap;
    if (resultFrames <= 0 || static_cast<std::uint64_t> (resultFrames)
        > static_cast<std::uint64_t> (std::numeric_limits<std::size_t>::max()))
        return out;

    out.tailFrames = context;
    out.headFrames = context;
    out.overlapFrames = overlap;
    out.channels.resize (source.size());
    for (std::size_t channel = 0; channel < source.size(); ++channel)
    {
        auto& dst = out.channels[channel];
        const auto& src = source[channel];
        dst.reserve (static_cast<std::size_t> (resultFrames));

        for (Frame i = 0; i < context - overlap; ++i)
            dst.push_back (src[static_cast<std::size_t> (r.end - context + i)]);

        for (Frame i = 0; i < overlap; ++i)
        {
            const float a = src[static_cast<std::size_t> (r.end - overlap + i)];
            const float b = src[static_cast<std::size_t> (r.begin + i)];
            const float t = overlap == 1 ? 0.5f
                                         : static_cast<float> (i + 1)
                                               / static_cast<float> (overlap + 1);
            dst.push_back (a * (1.0f - t) + b * t);
        }

        for (Frame i = overlap; i < context; ++i)
            dst.push_back (src[static_cast<std::size_t> (r.begin + i)]);
    }
    return out;
}
} // namespace liberty::loop_v015
