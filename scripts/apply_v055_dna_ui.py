#!/usr/bin/env python3
"""Apply v0.55 controls to the generated, already-patched v0.54 MORPH UI.
Called after apply_v055_juce_spectral_dna.py. No change to legacy source archive.
"""
from pathlib import Path
path = Path(__file__).resolve().parents[1] / "Source/LibertyMorphPadV036.h"
source = path.read_text("utf-8")

def once(old, new):
    global source
    count=source.count(old)
    if count!=1:
        raise SystemExit(f"SPECTRAL DNA UI anchor mismatch ({count}): {old[:90]!r}")
    source=source.replace(old,new,1)

once('        syncControls();\n        startTimerHz(24);',
'''        addAndMakeVisible(dnaToggle);
        addAndMakeVisible(dnaSlider);
        dnaToggle.setClickingTogglesState(true);
        dnaToggle.setColour(juce::TextButton::buttonColourId,juce::Colour(0xff102337));
        dnaToggle.setColour(juce::TextButton::buttonOnColourId,juce::Colour(0xff30556e));
        dnaToggle.setColour(juce::TextButton::textColourOffId,juce::Colours::white);
        dnaToggle.setColour(juce::TextButton::textColourOnId,juce::Colours::white);
        dnaSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        dnaSlider.setTextBoxStyle(juce::Slider::TextBoxRight,false,58,24);
        dnaSlider.setRange(0.0,100.0,1.0);
        dnaSlider.setTextValueSuffix(" %");
        dnaSlider.setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);
        dnaSlider.setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff0f1928));
        dnaSlider.setColour(juce::Slider::trackColourId,juce::Colour(0xffe9ba74));
        dnaToggle.onClick=[this] {
            auto& p=processor.getSpectralDNAEnabled();
            p.beginChangeGesture();
            p.setValueNotifyingHost(dnaToggle.getToggleState()?1.0f:0.0f);
            p.endChangeGesture();
            syncControls();
        };
        dnaSlider.onValueChange=[this] {
            if(syncing) return;
            auto& p=processor.getSpectralDNAMix();
            p.beginChangeGesture();
            p.setValueNotifyingHost(p.convertTo0to1(float(dnaSlider.getValue())/100.0f));
            p.endChangeGesture();
        };
        syncControls();
        startTimerHz(24);''')
once('        place(3,seedLabel,seedSlider);',
'''        place(3,seedLabel,seedSlider);
        // Reserved dedicated row ABOVE the two original footer labels.
        const int dnaW=juce::jmin(205,juce::jmax(100,(getWidth()-40)/3));
        const int dnaY=juce::jmax(98,getHeight()-121);
        dnaToggle.setBounds(15,dnaY,dnaW,30);
        dnaSlider.setBounds(23+dnaW,dnaY,juce::jmax(0,getWidth()-dnaW-38),30);''')
once('        const auto pad = padArea();\n        if (pad.isEmpty()) return;',
'''        const auto pad = padArea();
        if (pad.isEmpty()) return;
        const auto dnaY=juce::jmax(98,getHeight()-130);
        const juce::Rectangle<float> dnaPanel(9.0f,float(dnaY),float(juce::jmax(0,getWidth()-18)),49.0f);
        g.setColour(juce::Colour(0xff102337));
        g.fillRoundedRectangle(dnaPanel,7.0f);
        g.setColour(juce::Colour(0xff486279));
        g.drawRoundedRectangle(dnaPanel,7.0f,1.0f);''')
once('    juce::TextButton gravityButton {juce::String::fromUTF8(u8"GRAVIT\\u00c9 MIDI OFF")};',
'''    juce::TextButton gravityButton {juce::String::fromUTF8(u8"GRAVIT\\u00c9 MIDI OFF")};
    juce::TextButton dnaToggle {"SPECTRAL DNA OFF"};
    juce::Slider dnaSlider;''')
once('                              (float)getHeight()-196.0f)));',
'                              (float)getHeight()-269.0f)));')
once('        const float usable=juce::jmax(0.0f,(float)getHeight()-168.0f);',
'        const float usable=juce::jmax(0.0f,(float)getHeight()-240.0f);')
once('        activeButton.setButtonText(active ? "MORPH ON" : "MORPH OFF");',
'''        activeButton.setButtonText(active ? "MORPH ON" : "MORPH OFF");
        const bool dnaOn=processor.getSpectralDNAEnabled().get();
        dnaToggle.setToggleState(dnaOn,juce::dontSendNotification);
        dnaToggle.setButtonText(dnaOn ? "SPECTRAL DNA ON" : "SPECTRAL DNA OFF");''')
once('        syncing=false;\n    }\n\n    void timerCallback()',
'''        if(!dnaSlider.isMouseButtonDown())
            dnaSlider.setValue(processor.getSpectralDNAMix().get()*100.0f,
                               juce::dontSendNotification);
        syncing=false;
    }

    void timerCallback()''')
if "\ufffd" in source:
    raise SystemExit("Invalid UTF-8 replacement in DNA UI")
path.write_text(source,"utf-8")
print("Liberty v0.55 SPECTRAL DNA visible MORPH controls integrated.")
