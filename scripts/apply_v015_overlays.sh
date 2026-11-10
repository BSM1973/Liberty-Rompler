#!/usr/bin/env bash
set -euo pipefail

# Applied AFTER reconstruction through the v0.14 source patches.
# No macOS compilation occurs here. Keep the source snapshot reviewable.
test -f Source/SampleZoneView.h
test -f Source/PluginProcessor.cpp
test -f Source/PluginProcessor.h
test -f Source/LoopEditorV015.h
test -f Source/LoopPreviewVoiceV015.h

grep -q 'class LibertySampleZoneView final' Source/SampleZoneView.h
grep -q 'audio.applyGain(level)' Source/PluginProcessor.cpp
grep -q 'class LibertyRomplerAudioProcessor final' Source/PluginProcessor.h

for filename in PluginProcessor.h PluginProcessor.cpp SampleZoneView.h; do
  test -s "overlays/v015/$filename"
  cp "overlays/v015/$filename" "Source/$filename"
done

grep -q 'previewSampleZoneLoop' Source/PluginProcessor.cpp
grep -q 'previewSampleZoneLoop' Source/SampleZoneView.h
grep -q 'ScopedTryLock' Source/PluginProcessor.cpp
grep -q 'frameNudges' Source/SampleZoneView.h
grep -q 'crossfadeBands' Source/SampleZoneView.h

echo 'Liberty ROMpler v0.15: integrated sample-zone editor and DSP sources.'
