#include "../Source/LoopAuditionPlanV017.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

using liberty::loop_v017::planAudition;

int main()
{
    constexpr int frames = 2000000;
    const auto s44 = planAudition(44100.0, frames, 0, 60);
    const auto l44 = planAudition(44100.0, frames, 0, 250);
    assert(s44.valid && l44.valid);
    assert(s44.contextFrames == 2646);
    assert(l44.contextFrames == 11025);
    assert(l44.contextFrames > s44.contextFrames);

    const auto s48 = planAudition(48000.0, frames, 0, 60);
    const auto l48 = planAudition(48000.0, frames, 0, 250);
    assert(s48.contextFrames == 2880);
    assert(l48.contextFrames == 12000);
    const auto high = planAudition(384000.0, frames, 8192, 250);
    assert(high.valid && high.contextFrames == 32768);
    assert(high.fadeFrames == 8192);

    // Very short loops cannot exceed their actual length.
    const auto shortLoop = planAudition(48000.0, 200, 32, 250);
    assert(shortLoop.valid && shortLoop.contextFrames == 200);
    const auto minimum = planAudition(48000.0, 11, 0, 60);
    assert(minimum.valid && minimum.contextFrames == 11);

    // Large crossfades require a long-enough context.
    const auto fade = planAudition(8000.0, 20000, 3000, 60);
    assert(fade.valid && fade.contextFrames >= 3128);

    assert(!planAudition(0.0, frames, 0, 60).valid);
    assert(!planAudition(-1000.0, frames, 0, 60).valid);
    assert(!planAudition(std::numeric_limits<double>::infinity(), frames, 0, 60).valid);
    assert(!planAudition(std::numeric_limits<double>::quiet_NaN(), frames, 0, 60).valid);
    assert(!planAudition(400000.0, frames, 0, 250).valid);
    assert(!planAudition(44100.0, 10, 0, 60).valid);
    assert(!planAudition(44100.0, frames, -1, 60).valid);
    assert(!planAudition(44100.0, 100, 51, 60).valid);
    assert(!planAudition(44100.0, frames, 0, 0).valid);
    assert(!planAudition(44100.0, frames, 0, 500).valid);

    std::cout << "v0.17 audition context plans: tests OK\n";
}
