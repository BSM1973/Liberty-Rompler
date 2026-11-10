#include "../Source/LoopPairedNudgeV020.h"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>

using liberty::loop_v015::Loop;
using liberty::loop_v020::nudgePair;

int main()
{
    const Loop original{120, 1200, 10000};
    for (const auto requested : {-500, -10, -1, 0, 1, 10, 500})
    {
        const auto nudge = nudgePair(original, requested);
        assert(nudge.valid);
        assert(nudge.loop.end - nudge.loop.begin == original.end - original.begin);
        assert(nudge.loop.begin - original.begin == nudge.appliedFrames);
        assert(nudge.loop.end - original.end == nudge.appliedFrames);
    }
    assert(nudgePair(original, -10).loop.begin == 110);
    assert(nudgePair(original, 10).loop.end == 1210);

    const auto hardLeft = nudgePair(original, -5000);
    assert(hardLeft.valid && hardLeft.appliedFrames == -120);
    assert(hardLeft.loop.begin == 0 && hardLeft.loop.end == 1080);
    const auto hardRight = nudgePair(original, 20000);
    assert(hardRight.valid && hardRight.appliedFrames == 8800);
    assert(hardRight.loop.end == 10000 && hardRight.loop.begin == 8920);

    // Already at either boundary: request is safe and non-destructive.
    const auto left = nudgePair({0, 100, 1000}, -1);
    assert(left.valid && left.appliedFrames == 0 && left.loop.begin == 0);
    const auto right = nudgePair({900, 1000, 1000}, 10);
    assert(right.valid && right.appliedFrames == 0 && right.loop.end == 1000);

    // Exclusive end and minimum-length loops never change length.
    const auto minimum = nudgePair({6, 17, 100}, -10);
    assert(minimum.valid && minimum.loop.begin == 0 && minimum.loop.end == 11);
    assert(minimum.loop.end - minimum.loop.begin == 11);

    // The offset itself may be an extreme int64 input, without overflow.
    const auto max = nudgePair(original, std::numeric_limits<std::int64_t>::max());
    const auto min = nudgePair(original, std::numeric_limits<std::int64_t>::min());
    assert(max.valid && max.loop.end == original.total);
    assert(min.valid && min.loop.begin == 0);

    const auto invalid = nudgePair({0, 9, 100}, 1);
    assert(!invalid.valid);
    assert(invalid.loop.begin == 0 && invalid.loop.end == 9);
    assert(!nudgePair({0, 11, 100}, 1, 0).valid);
    assert(!nudgePair({10, 100, 99}, 1).valid);
    assert(!nudgePair({10, 50, 100}, 1, 50).valid);

    const Loop extreme{std::numeric_limits<std::int64_t>::max() - 15,
                       std::numeric_limits<std::int64_t>::max() - 4,
                       std::numeric_limits<std::int64_t>::max()};
    const auto huge = nudgePair(extreme, 9999);
    assert(huge.valid && huge.appliedFrames == 4);
    assert(huge.loop.end == huge.loop.total);
    assert(huge.loop.end - huge.loop.begin == 11);

    std::cout << "v0.20 paired manual nudge: all duration and bounds tests OK\n";
}
