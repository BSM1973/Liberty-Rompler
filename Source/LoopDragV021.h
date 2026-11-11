#pragma once
// Liberty ROMpler v0.21 - sample-accurate shift-drag of a whole loop.
// Gesture is defined against its initial anchor: no cumulative drift.
// Pure C++17 and independent of JUCE, WAV I/O, and the audio callback.

#include "LoopPairedNudgeV020.h"
#include <algorithm>

namespace liberty::loop_v021
{
using liberty::loop_v015::Frame;
using liberty::loop_v015::Loop;
using liberty::loop_v020::PairedNudge;

class PairedLoopDrag
{
public:
    bool begin(Loop source, Frame pointerFrame) noexcept
    {
        cancel();
        if (!liberty::loop_v015::isValid(source, 11)
            || pointerFrame < source.begin || pointerFrame >= source.end)
            return false;

        origin = source;
        anchor = pointerFrame;
        enabled = true;
        return true;
    }

    PairedNudge move(Frame pointerFrame) const noexcept
    {
        if (!enabled)
            return {};

        // The UI's frameAt() normally already bounds coordinates.
        // Clamp here as a safeguard for gestures continuing outside the WAV.
        const auto frame = std::clamp(pointerFrame, Frame{0}, origin.total);
        const auto offset = frame - anchor; // both nonnegative, signed-safe
        return liberty::loop_v020::nudgePair(origin, offset, 11);
    }

    void cancel() noexcept
    {
        enabled = false;
        origin = {};
        anchor = 0;
    }

    bool active() const noexcept { return enabled; }
    Loop original() const noexcept { return origin; }

private:
    Loop origin {};
    Frame anchor = 0;
    bool enabled = false;
};
} // namespace liberty::loop_v021
