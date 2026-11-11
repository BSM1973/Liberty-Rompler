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


# Update the legacy assertions that hard-coded precisely nine tabs.
# Preserve all geometry tests, now calculated for the real tenth MORPH tab.
def upgrade_test(relative_path, edits):
    path = root / relative_path
    content = path.read_text(encoding="utf-8")
    for old, new in edits:
        content = once(content, old, new)
    path.write_text(content, encoding="utf-8")

old_pages = "enum class Page { Presets, Layers, Routing, Effects, Matrix, Modulators, Arpeggiator, Samples, Settings }"
new_pages = "enum class Page { Presets, Layers, Morph, Routing, Effects, Matrix, Modulators, Arpeggiator, Samples, Settings }"
upgrade_test("tests/test_layers_v06.py", [(old_pages, new_pages)])
upgrade_test("tests/test_ui_v05.py", [(old_pages, new_pages)])
upgrade_test("tests/test_arp_v08.py", [
    ("(getWidth()-294-24-8*tabGap)/9",
     "(getWidth()-294-24-9*tabGap)/10"),
    ("max(80,min(126,(w-294-24-8*4)//9))",
     "max(72,min(116,(w-294-24-9*4)//10))"),
    ("294+9*tab+8*4", "294+10*tab+9*4"),
])
upgrade_test("tests/test_ui_v10.py", [
    ("def test_exactly_nine_pages_in_requested_order(self):",
     "def test_exactly_ten_pages_in_requested_order(self):"),
    ('["presetsTab", "layersTab", "routingTab", "effectsTab", "matrixTab",',
     '["presetsTab", "layersTab", "morphTab", "routingTab", "effectsTab", "matrixTab",'),
    (old_pages, new_pages),
    ("(width - 294 - 24 - 8*gap) // 9", "(width - 294 - 24 - 9*gap) // 10"),
    ("max(80, min(126,", "max(72, min(116,"),
    ("for i in range(9):", "for i in range(10):"),
    ("tabWidth=juce::jlimit(80,126,(getWidth()-294-24-8*tabGap)/9)",
     "tabWidth=juce::jlimit(72,116,(getWidth()-294-24-9*tabGap)/10)"),
])
print("Liberty MORPH v0.36: legacy tab tests upgraded to check 10 tabs.")
