#!/usr/bin/env python3
"""v0.43: expose read-only per-instance disk streaming diagnostics in REGLAGES.
Run once after apply_v042_streaming_health.py on reconstructed JUCE sources.
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SRC = ROOT / "Source"


def patch(file, replacements):
    path = SRC / file
    content = path.read_text(encoding="utf-8")
    for old, new in replacements:
        count = content.count(old)
        if count != 1:
            raise SystemExit(f"v0.43 {file}: expected one anchor, got {count}: {old[:90]!r}")
        content = content.replace(old, new, 1)
    path.write_text(content, encoding="utf-8")


patch("PluginProcessor.h", [
    ('#include "CreativeHarmonyMidiV038.h"',
     '#include "CreativeHarmonyMidiV038.h"\n#include "StreamingDiagnosticsV043.h"'),
    ('    liberty::PerformanceControls& getControls() { return controls; }',
     '    liberty::PerformanceControls& getControls() { return controls; }\n'
     '    liberty::stream_v043::Diagnostics getStreamingDiagnostics() const;'),
])

patch("PluginProcessor.cpp", [
    ('void LibertyRomplerAudioProcessor::prepareToPlay(double rate, int maximumExpectedSamplesPerBlock) {',
     '''liberty::stream_v043::Diagnostics LibertyRomplerAudioProcessor::getStreamingDiagnostics() const {
    liberty::stream_v043::Diagnostics result;
    // JUCE sound references remain alive for the snapshot even across preset reloads.
    // Called by the editor's timer, NEVER in processBlock() or a voice callback.
    for (const auto& synth : synths) {
        const auto count = synth.getNumSounds();
        for (int i = 0; i < count; ++i) {
            auto sound = synth.getSound(i);
            auto* mapped = dynamic_cast<liberty::MappedSampleSound*>(sound.get());
            if (mapped && mapped->isStreamed())
                result.add(mapped->streamingHealth());
        }
    }
    return result;
}

void LibertyRomplerAudioProcessor::prepareToPlay(double rate, int maximumExpectedSamplesPerBlock) {'''),
])

patch("PluginEditor.h", [
    ('#include "LibertyCreativePanelV038.h"',
     '#include "LibertyCreativePanelV038.h"\n#include "LibertyStreamingDiagnosticsV043.h"'),
    ('    LibertyCreativePanelV038 creativePanel;',
     '    LibertyCreativePanelV038 creativePanel;\n    LibertyStreamingDiagnosticsV043 streamingDiagnostics;'),
])

patch("PluginEditor.cpp", [
    ('sampleView(p), morphPad(p), creativePanel(p) {',
     'sampleView(p), morphPad(p), creativePanel(p), streamingDiagnostics(p) {'),
    ('    addChildComponent(creativePanel);',
     '    addChildComponent(creativePanel);\n    addChildComponent(streamingDiagnostics);'),
    ('    settingsScan.setVisible(next==Page::Settings);',
     '    settingsScan.setVisible(next==Page::Settings);\n'
     '    streamingDiagnostics.setVisible(next==Page::Settings);'),
    ('    settingsScan.setBounds(left+juce::jmin(240,width/2-30)+30,work.getY()+185,\n'
     '                           juce::jmin(260,width/2-30),39);',
     '    settingsScan.setBounds(left+juce::jmin(240,width/2-30)+30,work.getY()+185,\n'
     '                           juce::jmin(260,width/2-30),39);\n'
     '    streamingDiagnostics.setBounds(work.getX()+24, work.getY()+282,\n'
     '                                   work.getWidth()-48, juce::jmax(165, work.getHeight()-300));'),
])

print("v0.43: streaming diagnostics connected to processor and REGLAGES UI.")
