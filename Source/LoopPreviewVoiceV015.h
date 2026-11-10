#pragma once
// Liberty ROMpler v0.15: one-shot seam audition using a pre-rendered snapshot.
// The processor must own the snapshot for the entire audition. Call prepare()
// only when the audio callback is suspended or otherwise guaranteed idle.
// process() is allocation-free, lock-free, and does not modify host MIDI/audio state.

#include "LoopEditorV015.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace liberty::loop_v015
{
class SeamPreviewVoice
{
public:
    // prepare() belongs to the control thread with processing suspended.
    bool prepare (const SeamPreview* immutablePreview, std::size_t rampFrames = 64) noexcept
    {
        stop();
        snapshot = nullptr;
        cursor = 0;
        ramp = rampFrames;
        if (immutablePreview == nullptr || immutablePreview->channels.empty()
            || immutablePreview->channels[0].empty())
            return false;
        const auto frames = immutablePreview->channels[0].size();
        for (const auto& channel : immutablePreview->channels)
            if (channel.size() != frames)
                return false;
        snapshot = immutablePreview;
        return true;
    }

    // start(), stop() and process() must run on the audio thread. A UI callback
    // can request these through the host's normal message-to-audio command path.
    void start() noexcept
    {
        cursor = 0;
        playing = snapshot != nullptr;
    }

    void stop() noexcept { playing = false; }

    bool isPlaying() const noexcept { return playing; }

    std::size_t remainingFrames() const noexcept
    {
        if (! playing || snapshot == nullptr)
            return 0;
        const auto size = snapshot->channels[0].size();
        return cursor < size ? size - cursor : 0;
    }

    // Adds to an existing host-owned buffer. No I/O, allocation or locking.
    // For a mono preview and stereo output, both L/R receive the same preview.
    // For stereo previews, output channel 0 receives left and 1 receives right.
    // Channels beyond the available preview channels are left untouched.
    void process (float* const* outputs, std::size_t outputChannels,
                  std::size_t frames, float gain = 1.0f) noexcept
    {
        if (! playing || snapshot == nullptr || outputs == nullptr
            || outputChannels == 0 || frames == 0)
            return;
        const auto total = snapshot->channels[0].size();
        const auto fadeLength = std::min (ramp, total / 2);
        const auto outputCount = snapshot->channels.size() == 1
            ? std::min (outputChannels, std::size_t {2})
            : std::min (outputChannels, snapshot->channels.size());

        for (std::size_t i = 0; i < frames && playing; ++i)
        {
            if (cursor >= total)
            {
                playing = false;
                break;
            }
            float envelope = 1.0f;
            if (fadeLength > 0)
            {
                const auto in = static_cast<float> (std::min (fadeLength, cursor + 1))
                                / static_cast<float> (fadeLength);
                const auto out = static_cast<float> (std::min (fadeLength, total - cursor))
                                 / static_cast<float> (fadeLength);
                envelope = std::min (in, out);
            }
            for (std::size_t ch = 0; ch < outputCount; ++ch)
            {
                if (outputs[ch] != nullptr)
                {
                    const auto inputChannel = snapshot->channels.size() == 1 ? 0 : ch;
                    outputs[ch][i] += snapshot->channels[inputChannel][cursor]
                                      * envelope * gain;
                }
            }
            ++cursor;
            if (cursor == total)
                playing = false;
        }
    }

private:
    const SeamPreview* snapshot = nullptr; // Lifetime is owned outside audio thread.
    std::size_t cursor = 0;
    std::size_t ramp = 64;
    bool playing = false;
};
} // namespace liberty::loop_v015
