#!/usr/bin/env python3
"""v0.55: offline formant variants on genuine v0.48 harmonic wavetable frames.
Apply after apply_v055_juce_spectral_dna.py and apply_v055_dna_ui.py.
Zero shift uses the original table and original v0.48 audio path.
"""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
spectral=root/"Source/SpectralEngineV048.h"
processor_h=root/"Source/PluginProcessor.h"
processor_cpp=root/"Source/PluginProcessor.cpp"
ui=root/"Source/LibertyMorphPadV036.h"

def once(s, old, new, name):
    count=s.count(old)
    if count!=1:
        raise SystemExit(f"formants integration {name} anchor count {count}: {old[:100]!r}")
    return s.replace(old,new,1)

s=spectral.read_text("utf-8")
s=once(s,
'    std::array<std::array<float, tableSize>, bands> tables {};',
'    std::array<std::array<float, tableSize>, bands> tables {};'
'\n    // Precomputed offline; no FFT or table modification in render().'
'\n    std::array<std::array<float, tableSize>, bands> formantDown {}, formantUp {};',
"frame")
s=once(s,
'    double peak=0.;',
'''    // Smooth magnitude in the harmonic domain. A moving local envelope
    // reduces narrow-bin jumps when resampling its frequency positions.
    std::array<double,65> envelope {};
    for(int h=1;h<=64;++h) {
        double weighted=0.,norm=0.;
        for(int k=std::max(1,h-2);k<=std::min(64,h+2);++k) {
            const double w=3.-std::abs(double(h-k));
            weighted+=std::abs(harmonics[(size_t)k])*w;
            norm+=w;
        }
        envelope[(size_t)h]=norm>0.?weighted/norm:0.;
    }
    const auto sampleEnvelope=[&](double harmonic) noexcept {
        const double k=std::clamp(harmonic,1.,64.);
        const int base=(int)k;
        const int next=std::min(64,base+1);
        return envelope[(size_t)base]+
            (envelope[(size_t)next]-envelope[(size_t)base])*(k-base);
    };
    std::array<float,65> gainDown {}, gainUp {};
    for(int h=1;h<=64;++h) {
        const double source=std::max(1.e-6,envelope[(size_t)h]);
        // Fixed endpoints -0.5 and +0.5 octave formant placement.
        const double lower=sampleEnvelope(h/std::pow(2.,-.5));
        const double upper=sampleEnvelope(h/std::pow(2.,+.5));
        gainDown[(size_t)h]=(float)std::clamp(lower/source,.15,3.);
        gainUp[(size_t)h]=(float)std::clamp(upper/source,.15,3.);
    }
    double peak=0.,peakDown=0.,peakUp=0.;''',"envelope")
s=once(s,
'        auto& wave=result->tables[(size_t)b];',
'''        auto& wave=result->tables[(size_t)b];
        auto& down=result->formantDown[(size_t)b];
        auto& up=result->formantUp[(size_t)b];''',"wave pointers")
s=once(s,
'            double value=0.;',
'            double value=0.,low=0.,high=0.;',"values")
s=once(s,
'                value+=a.real()*std::cos(phase*h)-a.imag()*std::sin(phase*h);',
'''                const double harmonic=a.real()*std::cos(phase*h)-a.imag()*std::sin(phase*h);
                value+=harmonic;
                low+=harmonic*gainDown[(size_t)h];
                high+=harmonic*gainUp[(size_t)h];''',"harmonic")
s=once(s,
'            wave[(size_t)i]=(float)value;\n            if(b==bands-1) peak=std::max(peak,std::abs(value));',
'''            wave[(size_t)i]=(float)value;
            down[(size_t)i]=(float)low;
            up[(size_t)i]=(float)high;
            if(b==bands-1) {
                peak=std::max(peak,std::abs(value));
                peakDown=std::max(peakDown,std::abs(low));
                peakUp=std::max(peakUp,std::abs(high));
            }''',"peaks")
s=once(s,
'    for(auto& wave:result->tables)\n        for(auto& sample:wave) sample*=scale;',
'''    for(auto& wave:result->tables)
        for(auto& sample:wave) sample*=scale;
    // Shared bounded loudness reference avoids artificial level jumps
    // between envelope shapes; all three use the same original peak scale.
    const float downScale=scale*float(std::min(1.,peak/std::max(peakDown,1.e-6)));
    const float upScale=scale*float(std::min(1.,peak/std::max(peakUp,1.e-6)));
    for(auto& wave:result->formantDown)
        for(auto& sample:wave) sample*=downScale;
    for(auto& wave:result->formantUp)
        for(auto& sample:wave) sample*=upScale;''',"scale")
s=once(s,
'    float mix=0.f,transient=0.5f;',
'    float mix=0.f,transient=0.5f,formantShift=0.f;', "parameters")
s=once(s,
'        const float transient=limit(p.transient,0.f,1.f);',
'        const float transient=limit(p.transient,0.f,1.f);\n'
'        const float formant=limit(std::isfinite(p.formantShift)?p.formantShift:0.f,-1.f,1.f);',
"formant control")
s=once(s,
'                const float raw=wave[(size_t)a]*(1.f-f)+wave[(size_t)b]*f;',
'''                float raw=wave[(size_t)a]*(1.f-f)+wave[(size_t)b]*f;
                if(formant!=0.f) {
                    const auto& shifted=formant<0.f
                        ? frame->formantDown[(size_t)v.band]
                        : frame->formantUp[(size_t)v.band];
                    const float variant=shifted[(size_t)a]*(1.f-f)+shifted[(size_t)b]*f;
                    raw+=(variant-raw)*std::abs(formant);
                }''',"render sample")
spectral.write_text(s,"utf-8")

h=processor_h.read_text("utf-8")
h=once(h,
'    juce::AudioParameterFloat& getSpectralDNAMix() noexcept { return *spectralDNAMix; }',
'    juce::AudioParameterFloat& getSpectralDNAMix() noexcept { return *spectralDNAMix; }\n'
'    juce::AudioParameterFloat& getSpectralDNAFormant() noexcept { return *spectralDNAFormant; }',
"processor getter")
h=once(h,'    juce::AudioParameterFloat* spectralDNAMix = nullptr;',
'    juce::AudioParameterFloat* spectralDNAMix = nullptr;\n'
'    juce::AudioParameterFloat* spectralDNAFormant = nullptr;', "processor member")
processor_h.write_text(h,"utf-8")

p=processor_cpp.read_text("utf-8")
p=once(p,
'    addParameter(spectralDNAMix);',
'''    addParameter(spectralDNAMix);
    spectralDNAFormant = new juce::AudioParameterFloat(
        juce::ParameterID("libertySpectralDNAFormant",1),
        "Liberty SPECTRAL DNA Formants",
        juce::NormalisableRange<float>(-1.f,1.f),0.f);
    addParameter(spectralDNAFormant);''',"processor registration")
p=once(p,
'        spectralParams.mix=liberty::spectral_dna_v055::spectralMixForLayer(\n'
'            legacySpectralMix,layerDNAWeight,spectralDNAWet,spectralDNAOn);',
'''        spectralParams.mix=liberty::spectral_dna_v055::spectralMixForLayer(
            legacySpectralMix,layerDNAWeight,spectralDNAWet,spectralDNAOn);
        // Shift is evaluated from fixed, precomputed harmonic table variants.
        // DNA OFF leaves v0.48 source tables bit-identical.
        spectralParams.formantShift=spectralDNAOn ? spectralDNAFormant->get() : 0.f;''',
"processor render")
processor_cpp.write_text(p,"utf-8")

u=ui.read_text("utf-8")
u=once(u,
'        addAndMakeVisible(dnaSlider);',
'        addAndMakeVisible(dnaSlider);\n        addAndMakeVisible(formantSlider);',"ui add")
u=once(u,
'        dnaSlider.setTextValueSuffix(" %");',
'''        dnaSlider.setTextValueSuffix(" %");
        formantSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        formantSlider.setTextBoxStyle(juce::Slider::TextBoxRight,false,62,24);
        formantSlider.setRange(-12.0,12.0,1.0);
        formantSlider.setTextValueSuffix(" st");
        formantSlider.setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);
        formantSlider.setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff0f1928));
        formantSlider.setColour(juce::Slider::trackColourId,juce::Colour(0xff68d9cc));''',"ui config")
u=once(u,
'        dnaToggle.onClick=[this] {',
'''        formantSlider.onValueChange=[this] {
            if(syncing) return;
            auto& p=processor.getSpectralDNAFormant();
            p.beginChangeGesture();
            p.setValueNotifyingHost(p.convertTo0to1(float(formantSlider.getValue())/12.f));
            p.endChangeGesture();
        };
        dnaToggle.onClick=[this] {''',"ui events")
u=once(u,
'        dnaSlider.setBounds(23+dnaW,dnaY,juce::jmax(0,getWidth()-dnaW-38),30);',
'''        const int remaining=juce::jmax(0,getWidth()-dnaW-46);
        const int mixW=remaining/2;
        dnaSlider.setBounds(23+dnaW,dnaY,mixW,30);
        formantSlider.setBounds(31+dnaW+mixW,dnaY,remaining-mixW,30);''',
"ui bounds")
u=once(u,
'    juce::Slider dnaSlider;',
'    juce::Slider dnaSlider,formantSlider;', "ui member")
u=once(u,
'        if(!dnaSlider.isMouseButtonDown())',
'''        if(!formantSlider.isMouseButtonDown())
            formantSlider.setValue(processor.getSpectralDNAFormant().get()*12.0f,
                                   juce::dontSendNotification);
        if(!dnaSlider.isMouseButtonDown())''',"ui sync")
ui.write_text(u,"utf-8")
print("Liberty v0.55 SPECTRAL DNA formant tables prepared off audio, render and MORPH controls wired.")
