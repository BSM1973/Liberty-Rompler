#pragma once
// Liberty ROMpler v0.27 - one-step waveform viewport history.
// View-only state, never loop markers, WAV data, presets or real-time audio.
// Pure C++17; a toggle swaps the current view with the previous view.

#include <cmath>
#include <cstdint>

namespace liberty::loop_v027
{
struct View
{
    int total = 0;
    int first = 0;
    int span = 0;
    double zoom = 1.0;
};

inline bool valid(View v) noexcept
{
    return v.total > 0 && v.first >= 0 && v.span >= 1
        && v.span <= v.total
        && static_cast<std::int64_t>(v.first) + v.span <= v.total
        && std::isfinite(v.zoom) && v.zoom >= 1.0;
}

inline bool same(View a, View b) noexcept
{
    return a.total == b.total && a.first == b.first
        && a.span == b.span && a.zoom == b.zoom;
}

class ViewHistory
{
public:
    void clear() noexcept { saved = {}; hasSaved = false; }

    bool remember(View before) noexcept
    {
        if (!valid(before)) return false;
        saved = before;
        hasSaved = true;
        return true;
    }

    // Toggle is reversible; Shift-double-click twice returns to
    // the view where the user started (provided nothing else changed).
    bool toggle(View current, View& restored) noexcept
    {
        if (!hasSaved || !valid(current) || !valid(saved)
            || saved.total != current.total || same(saved, current))
            return false;
        restored = saved;
        saved = current;
        return true;
    }

    bool available() const noexcept { return hasSaved; }

private:
    View saved {};
    bool hasSaved = false;
};
} // namespace liberty::loop_v027
