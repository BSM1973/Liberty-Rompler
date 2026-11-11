#!/usr/bin/env python3
"""Apply the v0.37 professional multisample engine to the reconstructed JUCE source.

The old v1 bank JSON schema remains compatible: every new zone property is
optional. Run after v0.36 MORPH integration, before JUCE compilation.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
S = ROOT / "Source"


def patch(name, changes):
    path = S / name
    content = path.read_text(encoding="utf-8")
    for before, after in changes:
        number = content.count(before)
        if number != 1:
            raise SystemExit(
                f"v0.37 {name}: expected 1 anchor, got {number}: {before[:100]!r}"
            )
        content = content.replace(before, after, 1)
    path.write_text(content, encoding="utf-8")


patch("BankLibrary.h", [
    (
        "    float gain = 1.0f;\n};",
        "    float gain = 1.0f;\n"
        "    // Optional v0.37 fields; neutral defaults preserve every old bank.\n"
        "    int rrGroup = 0, rrIndex = 0;\n"
        "    int articulation = 0, keyswitchNote = -1;\n"
        "    int velocityFade = 0;\n"
        "};",
    ),
])

patch("BankLibrary.cpp", [
    (
        '                zone.gain = decimal(z, "gain", 1.0f);',
        '                zone.gain = decimal(z, "gain", 1.0f);\n'
        '                zone.rrGroup = integer(z, "rr_group", 0);\n'
        '                zone.rrIndex = integer(z, "rr_index", 0);\n'
        '                zone.articulation = integer(z, "articulation", 0);\n'
        '                zone.keyswitchNote = integer(z, "keyswitch_note", -1);\n'
        '                zone.velocityFade = integer(z, "velocity_fade", 0);',
    ),
    (
        "                    zone.gain < 0 || zone.gain > 4) continue;",
        "                    zone.gain < 0 || zone.gain > 4 ||\n"
        "                    zone.rrGroup < 0 || zone.rrGroup > 16 ||\n"
        "                    (zone.rrGroup == 0 && zone.rrIndex != 0) ||\n"
        "                    (zone.rrGroup > 0 && (zone.rrIndex < 1 || zone.rrIndex > 16)) ||\n"
        "                    zone.articulation < 0 || zone.articulation > 16 ||\n"
        "                    (zone.articulation == 0 && zone.keyswitchNote != -1) ||\n"
        "                    (zone.articulation > 0 &&\n"
        "                     (zone.keyswitchNote < 0 || zone.keyswitchNote > 127)) ||\n"
        "                    zone.velocityFade < 0 || zone.velocityFade > 32) continue;",
    ),
])

patch("SampleEngine.h", [
    (
        '#include "BankLibrary.h"',
        '#include "BankLibrary.h"\n#include "MultiSamplePolicyV037.h"',
    ),
    (
        "    bool appliesToVelocity(float velocity) const;",
        "    bool appliesToVelocity(float velocity) const;\n"
        "    float velocityCrossfadeGain(float velocity) const noexcept;\n"
        "    int getRoundRobinGroup() const noexcept { return rrGroup; }\n"
        "    int getRoundRobinIndex() const noexcept { return rrIndex; }\n"
        "    int getArticulation() const noexcept { return articulation; }\n"
        "    int getKeyswitchNote() const noexcept { return keyswitchNote; }",
    ),
    (
        "    float gain;\n    const LayerControls& controls;",
        "    float gain;\n"
        "    int rrGroup = 0, rrIndex = 0;\n"
        "    int articulation = 0, keyswitchNote = -1, velocityFade = 0;\n"
        "    const LayerControls& controls;",
    ),
    (
        "    void noteOn(int midiChannel, int midiNoteNumber, float velocity) override;",
        "    void noteOn(int midiChannel, int midiNoteNumber, float velocity) override;\n"
        "    int getActiveArticulation() const noexcept {\n"
        "        return activeArticulation.load(std::memory_order_relaxed);\n"
        "    }\n"
        "    void setActiveArticulation(int value) noexcept {\n"
        "        activeArticulation.store(std::max(1, std::min(16, value)),\n"
        "                                 std::memory_order_relaxed);\n"
        "    }\n"
        "    void resetPerformanceSelectors() {\n"
        "        const juce::ScopedLock guard(lock);\n"
        "        robin.reset();\n"
        "        activeArticulation.store(1, std::memory_order_relaxed);\n"
        "    }\n"
        "private:\n"
        "    liberty::multisample_v037::RoundRobin robin;\n"
        "    std::atomic<int> activeArticulation {1};",
    ),
])

before_note = """void MappedSynthesiser::noteOn(int channel, int noteNumber, float velocity) {
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
}"""

after_note = """void MappedSynthesiser::noteOn(int channel, int noteNumber, float velocity) {
    const juce::ScopedLock sl(lock);
    if (noteNumber < 0 || noteNumber > 127) return;

    // Keyswitch notes select an articulation and MUST NOT trigger a sample.
    // An untagged zone continues to play on any articulation (legacy behavior).
    for (auto* sound : sounds) {
        auto* mapped = dynamic_cast<MappedSampleSound*>(sound);
        if (mapped && mapped->getArticulation() > 0 &&
            mapped->getKeyswitchNote() == noteNumber) {
            activeArticulation.store(mapped->getArticulation(),
                                     std::memory_order_relaxed);
            return;
        }
    }

    // Fixed-size masks of RR indices that actually match this pitch and velocity.
    // This avoids silence when some takes have different key ranges.
    std::array<std::uint32_t, 17> variants {};
    const int active = getActiveArticulation();
    for (auto* sound : sounds) {
        auto* mapped = dynamic_cast<MappedSampleSound*>(sound);
        if (!mapped || !mapped->appliesToChannel(channel) ||
            !mapped->appliesToNote(noteNumber) ||
            !mapped->appliesToVelocity(velocity) ||
            !liberty::multisample_v037::articulationAllowed(
                mapped->getArticulation(), active))
            continue;
        const int group = mapped->getRoundRobinGroup();
        const int index = mapped->getRoundRobinIndex();
        if (group > 0 && group <= 16 && index > 0 && index <= 16)
            variants[(size_t)group] |= (std::uint32_t{1} << index);
    }
    std::array<int,17> selected {};
    for (int group=1; group<=16; ++group)
        if (variants[(size_t)group] != 0)
            selected[(size_t)group] = robin.select(
                noteNumber, group, variants[(size_t)group]);

    // Preserve the existing held-note replacement behavior.
    for (auto* voice : voices)
        if (voice->getCurrentlyPlayingNote() == noteNumber &&
            voice->isPlayingChannel(channel))
            stopVoice(voice, 1.0f, true);

    for (auto* sound : sounds) {
        auto* mapped = dynamic_cast<MappedSampleSound*>(sound);
        if (!mapped || !mapped->appliesToChannel(channel) ||
            !mapped->appliesToNote(noteNumber) ||
            !mapped->appliesToVelocity(velocity) ||
            !liberty::multisample_v037::articulationAllowed(
                mapped->getArticulation(), active))
            continue;
        const int group = mapped->getRoundRobinGroup();
        if (group > 0 && (group > 16 ||
            selected[(size_t)group] != mapped->getRoundRobinIndex()))
            continue;
        startVoice(findFreeVoice(sound, channel, noteNumber, isNoteStealingEnabled()),
                   sound, channel, noteNumber, velocity);
    }
}"""

patch("SampleEngine.cpp", [
    (
        "      loopCrossfadeFrames(z.loopCrossfadeFrames), gain(z.gain),\n      controls(layer), index(layerIndex) {",
        "      loopCrossfadeFrames(z.loopCrossfadeFrames), gain(z.gain),\n"
        "      rrGroup(z.rrGroup), rrIndex(z.rrIndex),\n"
        "      articulation(z.articulation), keyswitchNote(z.keyswitchNote),\n"
        "      velocityFade(z.velocityFade), controls(layer), index(layerIndex) {",
    ),
    (
        """    return midiVelocity >= lowVelocity && midiVelocity <= highVelocity &&
           midiVelocity >= controls.lowVelocity.load(std::memory_order_relaxed) &&
           midiVelocity <= controls.highVelocity.load(std::memory_order_relaxed);
}""",
        """    return liberty::multisample_v037::velocityWeight(
               midiVelocity, lowVelocity, highVelocity, velocityFade) > 0.0f &&
           midiVelocity >= controls.lowVelocity.load(std::memory_order_relaxed) &&
           midiVelocity <= controls.highVelocity.load(std::memory_order_relaxed);
}
float MappedSampleSound::velocityCrossfadeGain(float velocity) const noexcept {
    const int midiVelocity = juce::jlimit(1, 127, (int)std::round(velocity * 127.0f));
    return liberty::multisample_v037::velocityWeight(
        midiVelocity, lowVelocity, highVelocity, velocityFade);
}""",
    ),
    (before_note, after_note),
    (
        "    velocityGain = std::sqrt(juce::jlimit(0.0f, 1.0f, velocity)) * s->getGain();",
        "    velocityGain = std::sqrt(juce::jlimit(0.0f, 1.0f, velocity)) *\n"
        "                   s->getGain() * s->velocityCrossfadeGain(velocity);",
    ),
])

patch("PluginProcessor.cpp", [
    (
        "    layerSources = desired;",
        "    layerSources = desired;\n"
        "    // Reset round-robin and articulation selection on bank/preset change.\n"
        "    for (auto& synth : synths) synth.resetPerformanceSelectors();",
    ),
    (
        '    object->setProperty("layerState", layerArray);',
        '    object->setProperty("layerState", layerArray);\n'
        '    for (int i=0; i<4; ++i)\n'
        '        object->setProperty(juce::Identifier("articulation" + juce::String(i)),\n'
        '                            synths[(size_t)i].getActiveArticulation());',
    ),
    (
        """        }
    }
}
juce::AudioProcessorEditor* LibertyRomplerAudioProcessor::createEditor() {""",
        """        }
        // Restore the selected keyswitch after the layer sources are available.
        // Pre-v0.37 projects have no such properties: default articulation 1.
        for (int i=0; i<4; ++i) {
            const juce::Identifier key("articulation" + juce::String(i));
            if (o->hasProperty(key))
                synths[(size_t)i].setActiveArticulation((int)o->getProperty(key));
        }
    }
}
juce::AudioProcessorEditor* LibertyRomplerAudioProcessor::createEditor() {""",
    ),
])

print("Liberty ROMpler v0.37: RR / keyswitch / velocity fades integrated.")
