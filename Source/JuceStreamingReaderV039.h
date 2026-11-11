#pragma once
// Liberty v0.39: JUCE PCM decoder adapter for off-thread streaming prefetch.
// Explicitly constructed and used ONLY on the asset loading/worker thread.
// Never call this adapter from processBlock / SynthesiserVoice::renderNextBlock.
#include <JuceHeader.h>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace liberty::stream_v039 {
class JuceStreamingReader final {
public:
    explicit JuceStreamingReader(std::unique_ptr<juce::AudioFormatReader> source,
                                 int blockFrames = 4096)
      : reader(std::move(source)), buffer(2, juce::jmax(2, blockFrames)) {}

    bool isValid() const noexcept {
        return reader != nullptr && reader->numChannels >= 1 &&
            reader->numChannels <= 2 && reader->lengthInSamples >= 2;
    }
    std::int64_t totalFrames() const noexcept {
        return reader != nullptr ? static_cast<std::int64_t>(reader->lengthInSamples) : 0;
    }
    double sampleRate() const noexcept {
        return reader != nullptr ? reader->sampleRate : 0.0;
    }
    bool readFrames(std::int64_t first, std::size_t count,
                    float* left, float* right) {
        if (!isValid() || !left || !right || first < 0 || count == 0 ||
            count > static_cast<std::size_t>(buffer.getNumSamples()) ||
            first >= totalFrames() ||
            static_cast<std::uint64_t>(count) >
                static_cast<std::uint64_t>(totalFrames() - first)) return false;
        buffer.clear();
        if (!reader->read(&buffer, 0, static_cast<int>(count),
                          static_cast<juce::int64>(first), true, true))
            return false;
        const auto* sourceLeft = buffer.getReadPointer(0);
        const auto* sourceRight = buffer.getReadPointer(reader->numChannels > 1 ? 1 : 0);
        std::copy_n(sourceLeft, count, left);
        std::copy_n(sourceRight, count, right);
        return true;
    }
private:
    std::unique_ptr<juce::AudioFormatReader> reader;
    juce::AudioBuffer<float> buffer; // allocated during construction, not audio
};
} // namespace liberty::stream_v039
