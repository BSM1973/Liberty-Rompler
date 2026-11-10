#pragma once
// Liberty ROMpler v0.15: robust text/keyboard editing of exact frame positions.
// No JUCE dependency. Use on the UI thread; applied state remains in the DAW.

#include "LoopEditorV015.h"
#include <charconv>
#include <cstdint>
#include <optional>
#include <string_view>
#include <system_error>

namespace liberty::loop_v015
{
inline std::optional<Frame> parseFrame (std::string_view value) noexcept
{
    if (value.empty())
        return std::nullopt;
    // Consistent and unambiguous in all host locales; never interpret seconds
    // as frames or silently ignore trailing characters.
    Frame result = 0;
    const auto* first = value.data();
    const auto* last = value.data() + value.size();
    auto parsed = std::from_chars (first, last, result, 10);
    if (parsed.ec != std::errc {} || parsed.ptr != last || result < 0)
        return std::nullopt;
    return result;
}

class PendingFrameEdit
{
public:
    explicit PendingFrameEdit (Loop initial, Frame minLength = 1) noexcept
        : applied (initial), current (initial), minimum (minLength) {}

    bool valid() const noexcept { return isValid (current, minimum); }
    const Loop& value() const noexcept { return current; }
    bool changed() const noexcept
    {
        return current.begin != applied.begin || current.end != applied.end;
    }

    bool setTyped (Marker marker, std::string_view text) noexcept
    {
        if (! valid())
            return false;
        const auto frame = parseFrame (text);
        if (! frame)
            return false;
        current = setMarker (current, marker, *frame, minimum);
        return true;
    }

    void step (Marker marker, Frame distance = 1) noexcept
    {
        if (valid())
            current = nudge (current, marker, distance, minimum);
    }

    // Call only after the host successfully persists/applies the edit.
    void markApplied() noexcept { applied = current; }
    void cancelPending() noexcept { current = applied; }

    // Projects created before this feature must not turn snapping on.
    bool autoZero = false;

private:
    Loop applied;
    Loop current;
    Frame minimum = 1;
};
} // namespace liberty::loop_v015
