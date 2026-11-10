#include "../Source/LoopEditorV015.h"
#include "../Source/LoopPreviewWindowsV016.h"
#include "../Source/LoopSeamDifferenceV016.h"

#include <cassert>
#include <cstddef>
#include <cmath>
#include <limits>
#include <iostream>
#include <vector>

using liberty::loop_v015::Loop;
using liberty::loop_v015::renderSeamPreview;
using liberty::loop_v016::renderSeamFromWindows;
using liberty::loop_v016::measureRawSeamDifference;

int main()
{
    // Compare against the v0.15 full-WAV algorithm on identical frames.
    for (int total : {32, 100, 1000})
    {
        std::vector<std::vector<float>> wav(2, std::vector<float>(static_cast<std::size_t>(total)));
        for (int i = 0; i < total; ++i)
        {
            wav[0][static_cast<std::size_t>(i)] = (i % 19 - 9) / 9.0f;
            wav[1][static_cast<std::size_t>(i)] = (i % 13 - 6) / 6.0f;
        }
        const auto before = wav;
        for (int begin : {0, 1, total / 4})
        {
            const int end = total;
            const int length = end - begin;
            for (int context : {1, 2, length / 2, length})
            {
                if (context < 1 || context > length) continue;
                for (int fade : {0, 1, 2, context - 1, context})
                {
                    if (fade < 0 || fade > context) continue;
                    std::vector<std::vector<float>> tail(2), head(2);
                    for (int ch = 0; ch < 2; ++ch)
                    {
                        const auto& in = wav[static_cast<std::size_t>(ch)];
                        tail[static_cast<std::size_t>(ch)].assign(
                            in.begin() + (end - context), in.begin() + end);
                        head[static_cast<std::size_t>(ch)].assign(
                            in.begin() + begin, in.begin() + begin + context);
                    }
                    const auto reference = renderSeamPreview(wav, Loop{begin, end, total},
                                                             context, fade);
                    const auto optimized = renderSeamFromWindows(tail, head, fade);
                    assert(reference.channels == optimized.channels);
                    assert(reference.headFrames == optimized.headFrames);
                    assert(reference.tailFrames == optimized.tailFrames);
                    assert(reference.overlapFrames == optimized.overlapFrames);
                }
            }
        }
        assert(wav == before);
    }

    std::vector<std::vector<float>> valid{{1.0f, 2.0f}, {3.0f, 4.0f}};
    assert(renderSeamFromWindows(valid, valid, -1).channels.empty());
    assert(renderSeamFromWindows(valid, valid, 3).channels.empty());
    assert(renderSeamFromWindows(valid, std::vector<std::vector<float>>{{1.0f}}, 0).channels.empty());
    assert(renderSeamFromWindows({}, {}, 0).channels.empty());
    assert(renderSeamFromWindows({{}}, {{}}, 0).channels.empty());

    std::vector<float> big(32769, 0.0f);
    assert(renderSeamFromWindows({big}, {big}, 0).channels.empty());

    // Raw seam amplitude-step diagnostic; not an audibility predictor.
    const auto unityJump = measureRawSeamDifference({{0.0f, 1.0f}},
                                                      {{0.0f, 0.0f}});
    assert(unityJump.valid);
    assert(unityJump.peakStep == 1.0f);
    assert(std::abs(unityJump.dbAmplitude) < 1.0e-6f);
    const auto seamless = measureRawSeamDifference({{0.01f}}, {{0.01f}});
    assert(seamless.valid && seamless.peakStep == 0.0f);
    assert(seamless.dbAmplitude == -120.0f);
    const auto stereoJump = measureRawSeamDifference(
        {{0.0f, 0.2f}, {0.0f, -1.0f}},
        {{0.2f, 0.1f}, {0.5f, 0.0f}});
    assert(stereoJump.valid);
    assert(stereoJump.peakStep == 1.5f);
    assert(stereoJump.dbAmplitude > 3.0f && stereoJump.dbAmplitude < 4.0f);
    assert(!measureRawSeamDifference({}, {}).valid);
    assert(!measureRawSeamDifference({{}}, {{}}).valid);
    assert(!measureRawSeamDifference({{1.0f}}, {{1.0f}, {1.0f}}).valid);
    assert(!measureRawSeamDifference({{std::numeric_limits<float>::quiet_NaN()}},
                                     {{0.0f}}).valid);

    std::cout << "Loop preview v0.16: bounded-window equivalence tests OK\n";
}
