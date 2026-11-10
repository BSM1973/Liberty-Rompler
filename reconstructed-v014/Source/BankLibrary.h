#pragma once
#include <JuceHeader.h>
#include <vector>

namespace liberty {
struct ZoneDescriptor {
    juce::File file;
    int rootNote = 60, lowNote = 0, highNote = 127;
    int lowVelocity = 1, highVelocity = 127;
    int loopStart = -1, loopEnd = -1; // sample frames, -1 disables loop
    int loopCrossfadeFrames = 0; // v0.12: 0 preserves historical playback
    float gain = 1.0f;
};
// Optional four-layer preset format. Direct references only (no nested layer presets).
struct LayerPresetDefinition {
    juce::String bankId, presetId;
    bool enabled = true;
    float gain = 0.6f, pan = 0.0f, cutoffHz = 20000.0f;
    int transpose = 0, lowNote = 0, highNote = 127, lowVelocity = 1, highVelocity = 127;
};
struct PresetDescriptor {
    juce::String id, name, category, bankId, bankName;
    std::vector<ZoneDescriptor> zones;
    std::vector<LayerPresetDefinition> layers;
};
struct BankDescriptor {
    juce::String id, name, version;
    juce::File folder;
    std::vector<PresetDescriptor> presets;
};
class BankLibrary {
public:
    explicit BankLibrary(juce::File directory);
    void rescan();
    const std::vector<BankDescriptor>& getBanks() const { return banks; }
    juce::File getDirectory() const { return bankRoot; }
    const juce::StringArray& getWarnings() const { return warnings; }
    const PresetDescriptor* find(const juce::String& bank, const juce::String& preset) const;
    static juce::File defaultDirectory();
private:
    juce::File bankRoot;
    std::vector<BankDescriptor> banks;
    juce::StringArray warnings;
};
} // namespace liberty
