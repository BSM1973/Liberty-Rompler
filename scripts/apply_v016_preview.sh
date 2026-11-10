#!/usr/bin/env bash
set -euo pipefail
# Apply after scripts/apply_v015_overlays.sh. No GitHub build is triggered.
python3 - <<'PY'
from pathlib import Path
p = Path("Source/PluginProcessor.cpp")
s = p.read_text(encoding="utf-8")
anchor = '#include "LoopCrossfade.h"'
assert s.count(anchor) == 1, "Cannot locate shipped loop fade header"
s = s.replace(anchor, anchor + '\n#include "LoopPreviewWindowsV016.h"'
                         + '\n#include "LoopSeamDifferenceV016.h"', 1)
signature = 'const juce::File& wav, int startFrame, int endFrame, int crossfadeFrames) {'
assert s.count(signature) == 1, "Expected original preview signature"
s = s.replace(signature,
              'const juce::File& wav, int startFrame, int endFrame, int crossfadeFrames,'
              ' float* rawSeamStepDb) {', 1)

start_anchor = '    juce::AudioBuffer<float> decoded(channels, total);'
end_anchor = '    if (snapshot.channels.empty() || snapshot.channels[0].empty())'
assert s.count(start_anchor) == 1, "Whole-WAV v0.15 preview missing"
assert s.count(end_anchor) == 1, "Preview end anchor missing"
start, end = s.index(start_anchor), s.index(end_anchor)
old = s[start:end]
assert 'reader->read(&decoded, 0, total, 0' in old
assert 'renderSeamPreview(' in old
assert 'samples[(size_t)ch].assign' in old
replacement = r'''    // Bounded preview: never decode the full sample just to hear its seam.
    // Both windows are read on the UI thread, outside processBlock().
    if (contextFrames <= 0 || contextFrames > 32768)
        return juce::Result::fail("Pr\u00e9\u00e9coute trop longue.");
    juce::AudioBuffer<float> tailBuffer(channels, contextFrames);
    juce::AudioBuffer<float> headBuffer(channels, contextFrames);
    tailBuffer.clear();
    headBuffer.clear();
    if (!reader->read(&tailBuffer, 0, contextFrames,
                      endFrame - contextFrames, true, true)
        || !reader->read(&headBuffer, 0, contextFrames,
                         startFrame, true, true))
        return juce::Result::fail("Lecture des extraits WAV impossible.");

    std::vector<std::vector<float>> tail((size_t)channels);
    std::vector<std::vector<float>> head((size_t)channels);
    for (int ch = 0; ch < channels; ++ch) {
        const auto* t = tailBuffer.getReadPointer(ch);
        const auto* h = headBuffer.getReadPointer(ch);
        tail[(size_t)ch].assign(t, t + contextFrames);
        head[(size_t)ch].assign(h, h + contextFrames);
    }
    const auto seamDifference = liberty::loop_v016::measureRawSeamDifference(tail, head);
    if (!seamDifference.valid)
        return juce::Result::fail("Raccord WAV non analysable.");
    auto snapshot = liberty::loop_v016::renderSeamFromWindows(
        tail, head, crossfadeFrames);
'''
s = s[:start] + replacement + s[end:]
assert 'juce::AudioBuffer<float> decoded(channels, total)' not in s
assert 'renderSeamFromWindows(' in s
assert 'ScopedTryLock previewTryLock(previewLock)' in s
# Only publish a metric once the one-shot preview was prepared successfully.
last_preview_return = '''        previewVoice.start();
    }
    return juce::Result::ok();
}

bool LibertyRomplerAudioProcessor::getSampleZone('''
assert s.count(last_preview_return) == 1, "Unexpected v0.15 preview epilogue"
s = s.replace(last_preview_return,
    '''        previewVoice.start();
    }
    if (rawSeamStepDb != nullptr)
        *rawSeamStepDb = seamDifference.dbAmplitude;
    return juce::Result::ok();
}

bool LibertyRomplerAudioProcessor::getSampleZone(''', 1)
assert 'renderSeamFromWindows(' in s
assert 'measureRawSeamDifference(tail, head)' in s
p.write_text(s, encoding="utf-8")
print("v0.16 preview: whole-WAV read replaced by two bounded windows.")
PY
