#!/usr/bin/env python3
"""Inject dedicated MORPH page into the reconstructed JUCE editor, once.
Runs after v0.15-v0.17 scripts. Leaves previous tabs/controls untouched.
"""
from pathlib import Path

root = Path(__file__).resolve().parent.parent
header = root / "Source/PluginEditor.h"
source = root / "Source/PluginEditor.cpp"

def once(text, old, new):
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"v0.36 expected exactly one editor anchor, found {count}: {old[:80]}")
    return text.replace(old, new, 1)

h = header.read_text(encoding="utf-8")
h = once(h, '#include "SampleZoneView.h"',
         '#include "SampleZoneView.h"\n#include "LibertyMorphPadV036.h"')
h = once(h,
         "enum class Page { Presets, Layers, Routing, Effects, Matrix, Modulators, Arpeggiator, Samples, Settings };",
         "enum class Page { Presets, Layers, Morph, Routing, Effects, Matrix, Modulators, Arpeggiator, Samples, Settings };")
h = once(h, '    juce::TextButton samplesTab {"SAMPLES"};',
         '    juce::TextButton morphTab {"MORPH"};\n    juce::TextButton samplesTab {"SAMPLES"};')
h = once(h, '    LibertySampleZoneView sampleView;',
         '    LibertySampleZoneView sampleView;\n    LibertyMorphPadV036 morphPad;')

s = source.read_text(encoding="utf-8")
s = once(s, 'arpGrid(p.getArpControls()), sampleView(p) {',
         'arpGrid(p.getArpControls()), sampleView(p), morphPad(p) {')
s = once(s, '    makeTab(layersTab, Page::Layers);',
         '    makeTab(layersTab, Page::Layers);\n    makeTab(morphTab, Page::Morph);')
s = once(s, '    addChildComponent(sampleView);',
         '    addChildComponent(sampleView);\n    addChildComponent(morphPad);')
s = once(s, '    layersTab.setToggleState(next==Page::Layers,juce::dontSendNotification);',
         '    layersTab.setToggleState(next==Page::Layers,juce::dontSendNotification);\n    morphTab.setToggleState(next==Page::Morph,juce::dontSendNotification);')
s = once(s, '    sampleView.setVisible(next==Page::Samples);',
         '    sampleView.setVisible(next==Page::Samples);\n    morphPad.setVisible(next==Page::Morph);')
s = once(s, 'const auto heading=activePage==Page::Layers ? "COUCHES MULTISAMPLES" :',
         'const auto heading=activePage==Page::Layers ? "COUCHES MULTISAMPLES" :\n                           activePage==Page::Morph ? "LIBERTY MORPH  /  PAD XY" :')
s = once(s,
         '        } else if (activePage==Page::Routing) {\n            // Graph and interaction labels are drawn by the RoutingGraph child.',
         '        } else if (activePage==Page::Morph) {\n'
         '            // Dedicated interactive XY pad draws and handles its own content.\n'
         '        } else if (activePage==Page::Routing) {\n'
         '            // Graph and interaction labels are drawn by the RoutingGraph child.')
s = once(s,
         "    // Keep all nine buttons inside the title row at the 1120 px minimum.",
         "    // Ten tabs fit the 1120 px minimum; long French labels are fitted.")
s = once(s,
         "    const int tabWidth=juce::jlimit(80,126,(getWidth()-294-24-8*tabGap)/9);",
         "    const int tabWidth=juce::jlimit(72,116,(getWidth()-294-24-9*tabGap)/10);")
s = once(s, 'for (auto* tab : {&presetsTab,&layersTab,&routingTab,&effectsTab,&matrixTab,&modulatorsTab,&arpTab,&samplesTab,&settingsTab}) {',
         'for (auto* tab : {&presetsTab,&layersTab,&morphTab,&routingTab,&effectsTab,&matrixTab,&modulatorsTab,&arpTab,&samplesTab,&settingsTab}) {')
s = once(s,
         '    sampleView.setBounds(work.reduced(16,8).withTrimmedTop(40).withTrimmedBottom(8));',
         '    sampleView.setBounds(work.reduced(16,8).withTrimmedTop(40).withTrimmedBottom(8));\n'
         '    morphPad.setBounds(work.reduced(16,8).withTrimmedTop(40).withTrimmedBottom(8));')

header.write_text(h, encoding="utf-8")
source.write_text(s, encoding="utf-8")
print("Liberty MORPH v0.36: integrated pad into real JUCE editor, 10 tabs, layout safe.")
