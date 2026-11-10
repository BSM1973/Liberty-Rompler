#include "../Source/LoopEditorV015.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

using namespace liberty::loop_v015;

int main()
{
    // Exact one-frame adjustments, exclusive end and minimum loop length.
    Loop r {100, 200, 500};
    assert (isValid (r));
    assert (nudge (r, Marker::begin, 1).begin == 101);
    assert (nudge (r, Marker::end, -1).end == 199);
    assert (nudge (r, Marker::begin, 10).begin == 110);
    assert (nudge (r, Marker::begin, 99999).begin == 199);
    assert (nudge (r, Marker::end, -99999).end == 101);
    assert (nudge (r, Marker::begin, -99999).begin == 0);
    assert (nudge (r, Marker::end, 99999).end == 500);
    assert (nudge (r, Marker::begin, std::numeric_limits<Frame>::max()).begin == 199);
    assert (nudge (r, Marker::end, std::numeric_limits<Frame>::min()).end == 101);
    assert (setMarker (r, Marker::begin, 199, 2).begin == 198);
    assert (! isValid (Loop {-1, 50, 100}));
    assert (! isValid (Loop {50, 50, 100}));
    assert (! isValid (Loop {50, 101, 100}));
    assert (nudge (Loop {-1, 50, 100}, Marker::begin, 1).begin == -1);

    // Consistent graphics when zooming and scrolling, no overshoot.
    Viewport v {100, 100, 10.0, 400.0};
    assert (std::abs (frameToX (100, v) - 10.0) < 1e-9);
    assert (std::abs (frameToX (200, v) - 410.0) < 1e-9);
    auto f = crossfadeBands (Loop {110, 190, 500}, 15, v);
    assert (f.tail.visible && f.head.visible);
    assert (std::abs (f.tail.width - 60.0) < 1e-9);
    assert (std::abs (f.head.width - 60.0) < 1e-9);
    assert (! crossfadeBands (r, 0, v).head.visible);
    auto offscreen = crossfadeBands (Loop {300, 400, 500}, 15, v);
    assert (! offscreen.head.visible && ! offscreen.tail.visible);
    assert (! band (5, 5, v).visible);

    // Seam preview: sample-accurate output, mono/stereo independence,
    // no bank modification, no implicit audio playback.
    std::vector<std::vector<float>> src {
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
        {0, -1, -2, -3, -4, -5, -6, -7, -8, -9}};
    auto original = src;
    auto a = renderSeamPreview (src, Loop {2, 8, 10}, 3, 0);
    assert (a.channels.size() == 2);
    assert (a.channels[0] == std::vector<float> ({5, 6, 7, 2, 3, 4}));
    assert (a.channels[1] == std::vector<float> ({-5, -6, -7, -2, -3, -4}));
    auto b = renderSeamPreview (src, Loop {2, 8, 10}, 3, 2);
    assert (b.overlapFrames == 2);
    assert (b.channels[0].size() == 4);
    assert (b.channels[0][0] == 5);
    assert (b.channels[0][3] == 4);
    assert (std::isfinite (b.channels[0][1]));
    assert (src == original);
    assert (renderSeamPreview (src, Loop {-1, 8, 10}, 3, 0).channels.empty());
    assert (renderSeamPreview (src, Loop {2, 8, 10}, 0, 0).channels.empty());
    assert (renderSeamPreview (src, Loop {2, 8, 10}, 3, -1).channels.empty());
    std::cout << "Loop editor v0.15: C++ unit tests OK\n";
}
