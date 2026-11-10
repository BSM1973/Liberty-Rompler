#pragma once
#include <JuceHeader.h>
#include "BankLibrary.h"
#include "SampleEngine.h"
#include "SampleZoneMath.h"
#include "RoutingDSP.h"
#include "Arpeggiator.h"

class LibertyRomplerAudioProcessor final : public juce::AudioProcessor {
public:
    LibertyRomplerAudioProcessor();
    ~LibertyRomplerAudioProcessor() override = default;
    const juce::String getName() const override { return "Liberty Rompler"; }
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    double getTailLengthSeconds() const override { return 2.0; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return currentPresetName; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    liberty::BankLibrary& getLibrary() { return library; }
    juce::MidiKeyboardState& getKeyboard() { return keyboard; }
    liberty::ArpControls& getArpControls() { return arpSettings; }
    liberty::PerformanceControls& getControls() { return controls; }
    juce::Result selectPreset(const juce::String& bankId, const juce::String& presetId);
    juce::String getSelectedBankId() const { return currentBankId; }
    juce::String getSelectedPresetId() const { return currentPresetId; }
    juce::String getSelectedPresetName() const { return currentPresetName; }
    struct LayerSource { juce::String bankId, presetId; };
    LayerSource getLayerSource(int index) const;
    juce::Result setLayerSource(int index, const juce::String& bankId, const juce::String& presetId);
    // Edits are stored in the DAW state, never written into bank manifests or WAVs.
    struct SampleZoneEdit {
        juce::String bankId, presetId;
        int zoneIndex = -1;
        int lowNote = 0, highNote = 127, lowVelocity = 1, highVelocity = 127;
        int loopStart = -1, loopEnd = -1;
        int loopCrossfadeFrames = 0;
    };
    bool getSampleZone(const juce::String& bank, const juce::String& preset,
                       int zoneIndex, liberty::ZoneDescriptor& out) const;
    juce::Result setSampleZoneEdit(const SampleZoneEdit& edit);
    juce::Result resetSampleZoneEdit(const juce::String& bank,
                                    const juce::String& preset, int zoneIndex);

private:
    juce::AudioFormatManager formats;
    liberty::PerformanceControls controls;
    std::array<liberty::MappedSynthesiser, 4> synths;
    juce::AudioBuffer<float> layerScratch;
    std::array<liberty::RoutingFXRack,5> fxRacks;
    juce::MidiKeyboardState keyboard;
    liberty::ArpControls arpSettings;
    liberty::Arpeggiator arpEngine;
    liberty::BankLibrary library;
    juce::dsp::StateVariableTPTFilter<float> lowPass;
    juce::ADSR filterEnvelope;
    std::array<bool, 128> heldFilterNotes {};
    int heldFilterNoteCount = 0;
    double lfoPhaseOne = 0.0, lfoPhaseTwo = 0.0;
    float currentModWheel = 0.0f;
    double currentSampleRate = 44100.0;
    juce::String currentBankId, currentPresetId, currentPresetName;
    std::array<LayerSource, 4> layerSources;
    std::vector<SampleZoneEdit> zoneEdits;
    juce::Result reloadLayerSounds(const std::array<LayerSource, 4>& desired);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LibertyRomplerAudioProcessor)
};
