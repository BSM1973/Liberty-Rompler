#pragma once
// Liberty ROMpler v0.20 - manual paired marker movement.
// Both boundaries move by the same bounded amount; length stays unchanged.
// Pure C++17 helper; no allocations, WAV writes or real-time audio effects.

#include "LoopEditorV015.h"
#include <algorithm>

namespace liberty::loop_v020
{
using liberty::loop_v015::Frame;
using liberty::loop_v015::Loop;

struct PairedNudge
{
    Loop loop {};
    Frame appliedFrames = 0;
    bool valid = false;
};

inline PairedNudge nudgePair(Loop r, Frame wantedFrames,
                             Frame minFrames = 11) noexcept
{
    PairedNudge out;
    out.loop = r;
    if (!liberty::loop_v015::isValid(r, minFrames))
        return out;

    // r is validated. Both bounds are nonnegative, so these subtractions
    // are safe on int64, even when the requested movement is extreme.
    const Frame firstAllowed = -r.begin;
    const Frame lastAllowed = r.total - r.end;
    const Frame actual = std::clamp(wantedFrames, firstAllowed, lastAllowed);

    out.loop.begin += actual;
    out.loop.end += actual;
    out.appliedFrames = actual;
    out.valid = liberty::loop_v015::isValid(out.loop, minFrames)
                && (out.loop.end - out.loop.begin == r.end - r.begin);
    return out;
}
} // namespace liberty::loop_v020
