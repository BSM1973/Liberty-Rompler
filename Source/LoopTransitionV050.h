#pragma once
// v0.50: bounded immutable loop-head PCM preparation. All decoding is performed
// by the preset loader, NEVER by the real-time audio callback.
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace liberty::stream_v050 {
struct LoopWindow { int begin=0, frames=0; };
constexpr int maxLoopHeadFrames = 49152; // 384 KiB / stereo zone at most
constexpr int maxAttackFrames = 147456;

inline int plannedAttackFrames(std::int64_t length, double rate, bool sliceLeader) noexcept {
    if(length < 2 || !std::isfinite(rate) || rate <= 0.0) return 0;
    // SLICE v0.49 leader untouched: never regress the 3.25-s attack.
    const double seconds = sliceLeader ? 3.25 : 1.50;
    const int bounded = (int)std::min<double>(maxAttackFrames,
                              std::max(2.0, std::floor(rate*seconds)));
    return (int)std::min<std::int64_t>(length, bounded);
}

inline LoopWindow planLoopWindow(int totalFrames, int loopStart, int loopEnd,
                                 int attackEnd, double rate) noexcept {
    // Initial RAM attack already covers a loop beginning before attackEnd.
    // A loop crossing its end may need a dedicated partial window in future.
    if(totalFrames < 2 || loopStart < 0 || loopStart < attackEnd ||
       loopStart >= loopEnd - 10 || loopEnd > totalFrames ||
       !std::isfinite(rate) || rate <= 0.0) return {};
    const int span=loopEnd-loopStart;
    const int requested=(int)std::min<double>(maxLoopHeadFrames,
                        std::max(2.0,std::floor(rate*.75)+1.));
    return {loopStart, std::min(requested,span)};
}

} // namespace liberty::stream_v050
