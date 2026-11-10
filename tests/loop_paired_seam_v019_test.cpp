#include "../Source/LoopPairedSeamV019.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

using liberty::loop_v019::findBestPairedShift;

int main()
{
    // Mono/stereo improvement with identical offset and exact loop length.
    std::vector<std::vector<float>> wav(2, std::vector<float>(40));
    for (int i = 0; i < 40; ++i)
    {
        wav[0][static_cast<std::size_t>(i)] = i < 20 ? 0.7f : 0.2f;
        wav[1][static_cast<std::size_t>(i)] = i < 20 ? 0.6f : 0.3f;
    }
    wav[0][10] = 0.20f;
    wav[0][25] = 0.20f;
    wav[1][10] = 0.31f;
    wav[1][25] = 0.30f;
    const auto original = wav;
    std::vector<std::vector<float>> head(2), tail(2);
    for (int ch = 0; ch < 2; ++ch)
    {
        head[static_cast<std::size_t>(ch)].assign(wav[ch].begin() + 3, wav[ch].begin() + 15);
        tail[static_cast<std::size_t>(ch)].assign(wav[ch].begin() + 17, wav[ch].begin() + 31);
    }
    const auto paired = findBestPairedShift(head, 3, tail, 17, 8, 24, 40, 3);
    assert(paired.valid);
    assert(paired.deltaFrames == 2);
    assert(paired.startFrame == 10 && paired.endFrame == 26);
    assert(paired.endFrame - paired.startFrame == 16);
    assert(paired.bestPeakStep < paired.previousPeakStep);
    assert(std::abs(paired.bestPeakStep - 0.01f) < 1.0e-5f);
    assert(wav == original); // the source WAV stays intact

    // Already seamless: do not drift to a different loop just for slopes.
    const std::vector<std::vector<float>> quiet {{0, 0, 0, 0, 0}};
    const auto stable = findBestPairedShift(quiet, 9, quiet, 23, 10, 26, 50, 1);
    assert(stable.valid && stable.deltaFrames == 0);
    assert(stable.bestPeakStep == 0.0f);

    // Stereo max-peak metric vetoes a candidate that helps L but hurts R.
    const std::vector<std::vector<float>> startStereo {
        {0.5f, 0.0f, 0.1f, 0.2f, 0.3f},
        {0.5f, 0.0f, -0.6f, 0.2f, 0.3f}
    };
    const std::vector<std::vector<float>> endStereo {
        {0.0f, 0.0f, 0.1f, 0.1f, 0.0f},
        {0.0f, 0.0f, 0.1f, 0.6f, 0.0f}
    };
    const auto stereoSafe = findBestPairedShift(startStereo, 9, endStereo, 23,
                                                 10, 26, 50, 1);
    assert(stereoSafe.valid && stereoSafe.deltaFrames == 0);

    // Beginning of WAV: negative offsets prohibited. End at totalFrames:
    // positive offsets prohibited; exclusive-end semantics stay intact.
    const auto atStart = findBestPairedShift(head, 0, tail, 17, 0, 24, 40, 4);
    assert(atStart.valid && atStart.deltaFrames >= 0);
    const auto atEnd = findBestPairedShift(head, 3, tail, 17, 20, 40, 40, 4);
    assert(!atEnd.valid); // deliberately missing current marker sample windows

    // Short loops and invalid sources must never be accepted.
    assert(!findBestPairedShift(head, 3, tail, 17, 8, 18, 40, 4).valid);
    assert(!findBestPairedShift(head, 3, tail, 17, 8, 24, 40, -1).valid);
    assert(!findBestPairedShift(head, 3, tail, 17, 8, 24, 40, 5000).valid);
    assert(!findBestPairedShift(head, -1, tail, 17, 8, 24, 40, 4).valid);
    assert(!findBestPairedShift({}, 3, {}, 17, 8, 24, 40, 4).valid);
    assert(!findBestPairedShift({{1.0f}}, 3, {{1.0f}}, 17, 8, 24, 40, 4).valid);
    assert(!findBestPairedShift({{0.0f}, {0.0f}}, 3,
                                 {{0.0f}}, 17, 8, 24, 40, 4).valid);
    auto nanStart = head;
    nanStart[0][static_cast<std::size_t>(8 - 3)] =
        std::numeric_limits<float>::quiet_NaN();
    assert(!findBestPairedShift(nanStart, 3, tail, 17, 8, 24, 40, 4).valid);

    std::cout << "v0.19 paired loop: stereo and duration-lock tests OK\n";
}
