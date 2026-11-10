#!/usr/bin/env bash
set -euo pipefail
# Apply AFTER reconstruction + v0.15 overlays + v0.16 windowed preview.
# Does not alter uploaded WAVs, DAW state, or old presets.
python3 - <<'PY'
from pathlib import Path

p = Path("Source/PluginProcessor.cpp")
s = p.read_text(encoding="ascii")
inc = '#include "LoopSeamDifferenceV016.h"'
assert s.count(inc) == 1, "v0.16 processor required"
s = s.replace(inc, inc + '\n#include "LoopAuditionPlanV017.h"', 1)

old_signature = "float* rawSeamStepDb) {"
new_signature = "float* rawSeamStepDb, int contextMilliseconds) {"
assert s.count(old_signature) == 1, "v0.16 signature mismatch"
s = s.replace(old_signature, new_signature, 1)

old_context = '''    const int contextFrames = juce::jmin(
        length, juce::jmax(256, juce::jmax(crossfadeFrames + 128,
                            (int)std::round(reader->sampleRate * 0.06))));'''
new_context = r'''    const auto plan = liberty::loop_v017::planAudition(
        reader->sampleRate, length, crossfadeFrames, contextMilliseconds);
    if (!plan.valid)
        return juce::Result::fail("Dur\u00e9e de pr\u00e9\u00e9coute incorrecte.");
    const int contextFrames = plan.contextFrames;'''
assert s.count(old_context) == 1, "Expected v0.16 audition duration"
s = s.replace(old_context, new_context, 1)

# The preview is a diagnostic audition, not the performance signal.
# It must remain audible even if the instrument master gain is set to zero.
old_mix = '''    // One-shot loop audition is mixed only when the UI has already prepared
    // an immutable snapshot. tryLock never blocks the real-time audio thread.
    if (audio.getNumChannels() > 0) {
        juce::ScopedTryLock previewTryLock(previewLock);
        if (previewTryLock.isLocked()) {
            float* out[2] = { audio.getWritePointer(0),
                audio.getNumChannels() > 1 ? audio.getWritePointer(1) : nullptr };
            previewVoice.process(out, (size_t)juce::jmin(2, audio.getNumChannels()),
                                 (size_t)audio.getNumSamples(), 0.32f);
        }
    }
    audio.applyGain(level);'''
new_mix = '''    audio.applyGain(level);
    // The seam audition is independent from the instrument master gain.
    // The audio thread uses a nonblocking lock, performs no file I/O or
    // allocation, and adds the precomputed snapshot after the main gain.
    if (audio.getNumChannels() > 0) {
        juce::ScopedTryLock previewTryLock(previewLock);
        if (previewTryLock.isLocked()) {
            float* out[2] = { audio.getWritePointer(0),
                audio.getNumChannels() > 1 ? audio.getWritePointer(1) : nullptr };
            previewVoice.process(out, (size_t)juce::jmin(2, audio.getNumChannels()),
                                 (size_t)audio.getNumSamples(), 0.32f);
        }
    }'''
assert s.count(old_mix) == 1, "Unexpected v0.16 render mixing order"
s = s.replace(old_mix, new_mix, 1)

assert s.count("planAudition(") == 1
assert s.count("float* rawSeamStepDb, int contextMilliseconds) {") == 1
assert s.index("    audio.applyGain(level);") < s.index("    // The seam audition is independent")
assert 'juce::AudioBuffer<float> decoded(channels, total)' not in s
assert 'renderSeamFromWindows(' in s
assert 'ScopedTryLock previewTryLock(previewLock)' in s
p.write_text(s, encoding="ascii")
print("v0.17: bounded short/long audition and gain-independent preview installed.")
PY
