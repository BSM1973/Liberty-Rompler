#include "../Source/LoopDragV021.h"
#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>

using liberty::loop_v015::Loop;
using liberty::loop_v021::PairedLoopDrag;

int main()
{
    PairedLoopDrag drag;
    assert(!drag.active());
    assert(!drag.move(250).valid);

    const Loop original{100, 500, 1000};
    assert(!drag.begin(original, 99));
    assert(!drag.begin(original, 500)); // exclusive end
    assert(drag.begin(original, 240));
    assert(drag.active());
    assert(drag.original().begin == 100);

    const auto forward = drag.move(260);
    assert(forward.valid && forward.appliedFrames == 20);
    assert(forward.loop.begin == 120 && forward.loop.end == 520);
    assert(forward.loop.end - forward.loop.begin == 400);

    // Moving back uses the original anchor, not the last pointer position.
    const auto back = drag.move(190);
    assert(back.valid && back.appliedFrames == -50);
    assert(back.loop.begin == 50 && back.loop.end == 450);
    const auto restored = drag.move(240);
    assert(restored.valid && restored.appliedFrames == 0);
    assert(restored.loop.begin == original.begin);
    assert(restored.loop.end == original.end);

    // Boundary clipping preserves loop duration.
    const auto left = drag.move(-9999999);
    assert(left.valid && left.appliedFrames == -100);
    assert(left.loop.begin == 0 && left.loop.end == 400);
    const auto right = drag.move(9999999);
    assert(right.valid && right.appliedFrames == 500);
    assert(right.loop.begin == 600 && right.loop.end == 1000);

    drag.cancel();
    assert(!drag.active());
    assert(!drag.move(240).valid);

    // A minimum-length loop at the file boundary cannot move outward.
    assert(drag.begin({0, 11, 11}, 0));
    const auto fixed = drag.move(11);
    assert(fixed.valid && fixed.appliedFrames == 0);
    assert(fixed.loop.begin == 0 && fixed.loop.end == 11);
    drag.cancel();

    // Reject bad loops and out-of-loop anchor frames.
    assert(!drag.begin({0, 10, 100}, 5));
    assert(!drag.begin({10, 30, 29}, 15));
    assert(!drag.begin({10, 30, 100}, -1));
    assert(!drag.begin({10, 30, 100}, 30));

    // Extreme frame coordinates are bounded without integer overflow.
    const auto maximum = std::numeric_limits<std::int64_t>::max();
    const Loop huge{maximum - 100, maximum - 20, maximum};
    assert(drag.begin(huge, maximum - 40));
    const auto atRight = drag.move(maximum);
    assert(atRight.valid && atRight.appliedFrames == 20);
    assert(atRight.loop.end == maximum);
    assert(atRight.loop.end - atRight.loop.begin == 80);
    const auto atLeft = drag.move(std::numeric_limits<std::int64_t>::min());
    assert(atLeft.valid && atLeft.loop.begin == 0);
    assert(atLeft.loop.end == 80);

    std::cout << "v0.21 waveform shift-drag: stable anchor, clipping and duration OK\n";
}
