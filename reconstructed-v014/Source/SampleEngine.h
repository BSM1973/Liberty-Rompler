#pragma once
#include "BankLibrary.h"
#include <atomic>
#include <array>

namespace liberty {
// All UI-editable parameters are atomic; audio voices never query the bank library.
struct LayerControls {
    std::atomic<bool> enabled {false};
    std::atomic<float> gain {0.6f};
    std::atomic<float> pan {0.0f};
    std::atomic<float> cutoffHz {20000.0f};
    std::atomic<int> transpose {0};
    std::atomic<int> lowNote {0}, highNote {127};
    std::atomic<int> lowVelocity {1}, highVelocity {127};
};
struct ModulationRoute {
    std::atomic<int> source {0};    // 0 Off, 1 LFO 1, 2 LFO 2, 3 Mod Wheel
    std::atomic<int> target {0};    // 0 Off, 1 Filter, 2 Level
    std::atomic<float> amount {0.0f}; // bipolar -1 to +1
};
struct PerformanceControls {
    std::atomic<float> masterGain {0.75f};
    std::atomic<float> attackSeconds {0.008f};
    std::atomic<float> releaseSeconds {0.30f};
    std::atomic<float> decaySeconds {0.05f};
    std::atomic<float> sustainLevel {1.0f};
    // Filter ADSR is global and follows MIDI gate (polyphonic samples share the filter).
    std::atomic<float> filterAttack {0.01f};
    std::atomic<float> filterDecay {0.18f};
    std::atomic<float> filterSustain {0.65f};
    std::atomic<float> filterRelease {0.35f};
    std::atomic<float> filterEnvDepth {0.0f}; // -1..+1 in four octaves

    std::atomic<float> cutoffHz {20000.0f};
    std::atomic<float> resonance {0.707f};
    // Genuine chorus and reverb DSP controls. Mix = 0 preserves v0.2 sound.
    std::atomic<float> chorusRate {0.75f};
    std::atomic<float> chorusDepth {0.35f};
    std::atomic<float> chorusMix {0.0f};
    std::atomic<float> reverbRoom {0.45f};
    std::atomic<float> reverbDamping {0.45f};
    std::atomic<float> reverbMix {0.0f};
    // Stereo delay: time in milliseconds, feedback and 0..1 wet/dry blend.
    std::atomic<float> delayTimeMs {375.0f};
    std::atomic<float> delayFeedback {0.30f};
    std::atomic<float> delayMix {0.0f};
    // Soft-clipping distortion: drive (dB), low-pass tone (Hz) and blend.
    std::atomic<float> distortionDriveDb {10.0f};
    std::atomic<float> distortionToneHz {6000.0f};
    std::atomic<float> distortionMix {0.0f};
    std::atomic<float> lfoOneHz {0.8f};
    std::atomic<float> lfoTwoHz {0.22f};
    std::atomic<int> lfoOneWave {0}; // 0 sine, 1 triangle, 2 sawtooth, 3 square
    std::atomic<int> lfoTwoWave {0};
    // Fixed-frequency three-band tone EQ (low shelf / mid bell / high shelf).
    std::atomic<float> eqLowDb {0.0f};
    std::atomic<float> eqMidDb {0.0f};
    std::atomic<float> eqHighDb {0.0f};
    // Compressor: 15 ms attack, 130 ms release. Ratio 1:1 is transparent.
    std::atomic<float> compThresholdDb {-18.0f};
    std::atomic<float> compRatio {1.0f};
    std::atomic<float> compMakeupDb {0.0f};

    std::array<ModulationRoute, 4> matrix;
    std::array<LayerControls, 4> layers;
    // Stable integer FX identifiers: 0 bypass, 1 Drive, 2 EQ, 3 Comp,
    // 4 Chorus, 5 Delay, 6 Reverb. Two slots per layer, six on master.
    std::array<std::array<std::atomic<int>,2>,4> layerFx;
    std::array<std::atomic<int>,6> masterFx;
};

class MappedSampleSound final : public juce::SynthesiserSound {
public:
    MappedSampleSound(const ZoneDescriptor& zone, juce::AudioFormatReader& reader,
                      const LayerControls& layer, int layerIndex);
    bool appliesToNote(int midiNoteNumber) override;
    bool appliesToChannel(int) override { return true; }
    bool appliesToVelocity(float velocity) const;
    const juce::AudioBuffer<float>& getAudio() const { return data; }
    double getSourceSampleRate() const { return sourceRate; }
    int getRootNote() const { return rootNote; }
    int getLoopStart() const { return loopStart; }
    int getLoopEnd() const { return loopEnd; }
    int getLoopCrossfadeFrames() const { return loopCrossfadeFrames; }
    float getGain() const { return gain; }
    const LayerControls& getLayer() const { return controls; }
    int getLayerIndex() const { return index; }
    bool isLoaded() const { return data.getNumSamples() > 1; }
private:
    juce::AudioBuffer<float> data;
    double sourceRate = 44100.0;
    int rootNote, lowNote, highNote, lowVelocity, highVelocity;
    int loopStart, loopEnd, loopCrossfadeFrames;
    float gain;
    const LayerControls& controls;
    int index;
};

// Unlike JUCE's default noteOn(), this does not stop an earlier velocity layer
// while starting another overlapping sound from the same preset.
class MappedSynthesiser final : public juce::Synthesiser {
public:
    void noteOn(int midiChannel, int midiNoteNumber, float velocity) override;
};

class SampleVoice final : public juce::SynthesiserVoice {
public:
    explicit SampleVoice(PerformanceControls& performance) : controls(performance) {}
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound,
                   int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void pitchWheelMoved(int value) override;
    void controllerMoved(int, int) override {}
    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                         int startSample, int numSamples) override;
private:
    PerformanceControls& controls;
    juce::ADSR envelope;
    double position = 0.0, ratio = 1.0;
    int note = 60, wheel = 8192;
    float velocityGain = 0.0f;
    std::array<juce::IIRFilter, 2> layerFilter;
    float previousCutoff = -1.0f;
    void updatePitch();
};
} // namespace liberty
