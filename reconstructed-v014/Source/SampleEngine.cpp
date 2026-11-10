#include "SampleEngine.h"
#include "LoopCrossfade.h"
#include <cmath>

namespace liberty {
MappedSampleSound::MappedSampleSound(const ZoneDescriptor& z, juce::AudioFormatReader& reader,
                                     const LayerControls& layer, int layerIndex)
    : sourceRate(reader.sampleRate), rootNote(z.rootNote), lowNote(z.lowNote),
      highNote(z.highNote), lowVelocity(z.lowVelocity), highVelocity(z.highVelocity),
      loopStart(z.loopStart), loopEnd(z.loopEnd),
      loopCrossfadeFrames(z.loopCrossfadeFrames), gain(z.gain),
      controls(layer), index(layerIndex) {
    // Prototype memory cap: up to 20 seconds per sample zone.
    const auto count = (int)juce::jmin(reader.lengthInSamples,
                                      (juce::int64)(reader.sampleRate * 20.0));
    if (count < 2 || reader.numChannels < 1 || reader.numChannels > 2) return;
    data.setSize((int)reader.numChannels, count + 1);
    data.clear();
    if (!reader.read(&data, 0, count, 0, true, true)) data.setSize(0, 0);
    if (loopStart < 0 || loopEnd <= loopStart + 10 || loopEnd > count) {
        loopStart = -1;
        loopEnd = -1;
        loopCrossfadeFrames = 0;
    } else {
        loopCrossfadeFrames = juce::jlimit(0, liberty::loopfade::maximum(loopStart, loopEnd),
                                          loopCrossfadeFrames);
    }
}
bool MappedSampleSound::appliesToNote(int n) {
    return controls.enabled.load(std::memory_order_relaxed) &&
           n >= lowNote && n <= highNote &&
           n >= controls.lowNote.load(std::memory_order_relaxed) &&
           n <= controls.highNote.load(std::memory_order_relaxed);
}
bool MappedSampleSound::appliesToVelocity(float v) const {
    const int midiVelocity = juce::jlimit(1, 127, (int)std::round(v * 127.0f));
    return midiVelocity >= lowVelocity && midiVelocity <= highVelocity &&
           midiVelocity >= controls.lowVelocity.load(std::memory_order_relaxed) &&
           midiVelocity <= controls.highVelocity.load(std::memory_order_relaxed);
}
void MappedSynthesiser::noteOn(int channel, int noteNumber, float velocity) {
    const juce::ScopedLock sl(lock);
    // Stop currently held layers once, not once per matching zone.
    for (auto* voice : voices)
        if (voice->getCurrentlyPlayingNote() == noteNumber && voice->isPlayingChannel(channel))
            stopVoice(voice, 1.0f, true);
    for (auto* sound : sounds) {
        auto* mapped = dynamic_cast<MappedSampleSound*>(sound);
        if (mapped && mapped->appliesToChannel(channel) && mapped->appliesToNote(noteNumber)
            && mapped->appliesToVelocity(velocity)) {
            startVoice(findFreeVoice(sound, channel, noteNumber, isNoteStealingEnabled()),
                       sound, channel, noteNumber, velocity);
        }
    }
}
bool SampleVoice::canPlaySound(juce::SynthesiserSound* sound) {
    return dynamic_cast<MappedSampleSound*>(sound) != nullptr;
}
void SampleVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int pitchWheel) {
    auto* s = dynamic_cast<MappedSampleSound*>(sound);
    if (!s || !s->isLoaded() || !s->appliesToVelocity(velocity)) {
        clearCurrentNote();
        return;
    }
    note = midiNoteNumber;
    wheel = pitchWheel;
    position = 0;
    for (auto& f : layerFilter) f.reset();
    previousCutoff = -1.0f;
    velocityGain = std::sqrt(juce::jlimit(0.0f, 1.0f, velocity)) * s->getGain();
    envelope.setSampleRate(juce::jmax(1.0, getSampleRate()));
    juce::ADSR::Parameters p;
    p.attack = controls.attackSeconds.load(std::memory_order_relaxed);
    p.decay = controls.decaySeconds.load(std::memory_order_relaxed);
    p.sustain = controls.sustainLevel.load(std::memory_order_relaxed);
    p.release = controls.releaseSeconds.load(std::memory_order_relaxed);
    envelope.setParameters(p);
    envelope.noteOn();
    updatePitch();
}
void SampleVoice::updatePitch() {
    auto* s = dynamic_cast<MappedSampleSound*>(getCurrentlyPlayingSound().get());
    if (!s) return;
    const double bend = ((double)wheel - 8192.0) / 8192.0 * 2.0; // +/- 2 semitones
    const auto transpose = s->getLayer().transpose.load(std::memory_order_relaxed);
    ratio = std::pow(2.0, ((double)(note - s->getRootNote() + transpose) + bend) / 12.0)
          * (s->getSourceSampleRate() / juce::jmax(1.0, getSampleRate()));
}
void SampleVoice::pitchWheelMoved(int value) { wheel = value; updatePitch(); }
void SampleVoice::stopNote(float, bool allowTailOff) {
    if (allowTailOff) envelope.noteOff();
    else { envelope.reset(); clearCurrentNote(); }
}
void SampleVoice::renderNextBlock(juce::AudioBuffer<float>& out, int start, int count) {
    auto* s = dynamic_cast<MappedSampleSound*>(getCurrentlyPlayingSound().get());
    if (!s || !s->isLoaded()) return;
    const auto& audio = s->getAudio();
    const int last = audio.getNumSamples() - 1;
    const int channels = juce::jmin(2, out.getNumChannels());
    const auto& layer = s->getLayer();
    // Per-voice filters are independent per layer and per stereo channel.
    const float sampleRate = (float)juce::jmax(8000.0, getSampleRate());
    const float cutoff = juce::jlimit(80.0f, sampleRate * 0.45f,
                                    layer.cutoffHz.load(std::memory_order_relaxed));
    const bool filtered = layer.cutoffHz.load(std::memory_order_relaxed) < 19500.0f;
    if (filtered && std::abs(cutoff - previousCutoff) > 0.1f) {
        const auto coeff = juce::IIRCoefficients::makeLowPass(sampleRate, cutoff, 0.707f);
        for (auto& f : layerFilter) f.setCoefficients(coeff);
        previousCutoff = cutoff;
    }
    updatePitch(); // Includes live per-layer transpose changes without note re-triggering.
    const float volume = layer.gain.load(std::memory_order_relaxed);
    const float pan = juce::jlimit(-1.0f, 1.0f, layer.pan.load(std::memory_order_relaxed));
    const float angle = (pan + 1.0f) * juce::MathConstants<float>::pi * 0.25f;
    const float leftGain = std::cos(angle) * std::sqrt(2.0f);
    const float rightGain = std::sin(angle) * std::sqrt(2.0f);
    for (int i = 0; i < count; ++i) {
        if (position >= last) { clearCurrentNote(); break; }
        const int frame = (int)position;
        const float mix = (float)(position - frame);
        const float amp = envelope.getNextSample() * velocityGain;
        const auto blend = liberty::loopfade::at(position, s->getLoopStart(), s->getLoopEnd(),
                                                 s->getLoopCrossfadeFrames());
        for (int ch = 0; ch < channels; ++ch) {
            const int source = juce::jmin(ch, audio.getNumChannels() - 1);
            const auto* data = audio.getReadPointer(source);
            float value = data[frame] + (data[frame + 1] - data[frame]) * mix;
            if (blend.fraction > 0.0f) {
                const int headFrame = (int)blend.headPosition;
                const float headMix = (float)(blend.headPosition - headFrame);
                const float head = data[headFrame] +
                                   (data[headFrame + 1] - data[headFrame]) * headMix;
                value += (head - value) * blend.fraction; // linear, phase-coherent loop seam
            }
            const float tone = filtered ? layerFilter[(size_t)ch].processSingleSampleRaw(value) : value;
            out.addSample(ch, start + i, tone * amp * volume * (ch == 0 ? leftGain : rightGain));
        }
        position += ratio;
        if (s->getLoopStart() >= 0 && position >= s->getLoopEnd())
            position = liberty::loopfade::wrap(position, s->getLoopStart(),
                                              s->getLoopEnd(), s->getLoopCrossfadeFrames());
        if (!envelope.isActive()) { clearCurrentNote(); break; }
    }
}
} // namespace liberty
