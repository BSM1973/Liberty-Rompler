#include "../Source/LoopEditorV015.h"
#include "../Source/LoopFrameInputV015.h"
#include "../Source/LoopPreviewVoiceV015.h"

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
    // Strict, locale-independent entry and reversible pending marker edits.
    assert (parseFrame ("123").value() == 123);
    assert (parseFrame ("0").value() == 0);
    assert (! parseFrame ("").has_value());
    assert (! parseFrame ("12.3").has_value());
    assert (! parseFrame ("12ms").has_value());
    assert (! parseFrame ("-1").has_value());
    assert (! parseFrame ("999999999999999999999999999999").has_value());
    PendingFrameEdit edit {Loop {100, 200, 500}};
    assert (! edit.autoZero);
    edit.step (Marker::begin, 1);
    edit.step (Marker::end, -10);
    assert (edit.value().begin == 101 && edit.value().end == 190);
    assert (edit.changed());
    edit.cancelPending();
    assert (! edit.changed() && edit.value().begin == 100 && edit.value().end == 200);
    assert (edit.setTyped (Marker::begin, "125"));
    assert (edit.value().begin == 125 && edit.changed());
    assert (! edit.setTyped (Marker::end, "20ms"));
    assert (edit.value().end == 200);
    edit.markApplied();
    assert (! edit.changed());
    edit.step (Marker::begin, -1000);
    assert (edit.value().begin == 0);
    edit.cancelPending();
    assert (edit.value().begin == 125);
    assert (edit.setTyped (Marker::end, "120"));
    assert (edit.value().end == 126);

    // Precomputed seam audio can be played through existing output buffers.
    // Preparation never happens from an active audio callback.
    SeamPreviewVoice voice;
    assert (voice.prepare (&a, 2));
    assert (! voice.isPlaying());
    float left[8] = {0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f};
    float right[8] = {};
    float sidechain[8] = {};
    float* out[] = {left, right, sidechain};
    voice.start();
    voice.process (out, 3, 8);
    assert (! voice.isPlaying());
    assert (voice.remainingFrames() == 0);
    assert (std::abs (left[0] - 3.0f) < 1e-6f); // 5 * 0.5 ramp + 0.5
    assert (std::abs (left[1] - 6.5f) < 1e-6f); // Full volume
    assert (std::abs (left[5] - 2.5f) < 1e-6f); // 4 * 0.5 ramp + 0.5
    assert (std::abs (left[6] - 0.5f) < 1e-6f); // No tail spill
    assert (std::abs (right[0] + 2.5f) < 1e-6f);
    for (float v : sidechain) assert (v == 0.0f);

    // Inactive voice must never write to host buffers.
    voice.process (out, 3, 8);
    assert (left[0] == 3.0f);
    voice.start();
    voice.stop();
    assert (! voice.isPlaying());
    SeamPreview invalid;
    invalid.channels = {{1, 2}, {1}};
    assert (! voice.prepare (&invalid));
    assert (! voice.isPlaying());

    // Mono preview to stereo outputs; explicit one-shot playback.
    std::vector<std::vector<float>> mono {{0, 1, 2, 3, 4, 5, 6, 7}};
    const auto monoSeam = renderSeamPreview (mono, Loop {1, 7, 8}, 2, 0);
    assert (voice.prepare (&monoSeam, 0));
    voice.start();
    float l2[4] = {}, r2[4] = {};
    float* stereo[] = {l2, r2};
    voice.process (stereo, 2, 4);
    assert (l2[0] == r2[0] && l2[1] == r2[1]);
    assert (! voice.isPlaying());

    std::cout << "Loop editor v0.15: C++ unit tests OK\n";
}
