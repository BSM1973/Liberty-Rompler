#!/usr/bin/env python3
"""Integrate opt-in spectral DNA v0.55 with v0.48's real-time JUCE resynths.
Run only AFTER apply_v054_orbit.py on the reconstructed generated sources.
This does NOT insert the offline FFT/WOLA prototype into processBlock.
"""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
header=root/"Source/PluginProcessor.h"
source=root/"Source/PluginProcessor.cpp"

def once(s,old,new):
    count=s.count(old)
    if count!=1:
        raise SystemExit(f"v0.55 JUCE anchor count {count}, expected 1: {old[:110]!r}")
    return s.replace(old,new,1)

h=header.read_text(encoding="utf-8")
h=once(h,
    '#include "SpectralEngineV048.h"',
    '#include "SpectralEngineV048.h"\n#include "SpectralDNAMixV055.h"')
h=once(h,
    '    juce::AudioParameterBool& getSpectralEnabled(int i) noexcept',
    '    juce::AudioParameterBool& getSpectralDNAEnabled() noexcept { return *spectralDNAEnabled; }\n'
    '    juce::AudioParameterFloat& getSpectralDNAMix() noexcept { return *spectralDNAMix; }\n'
    '    juce::AudioParameterBool& getSpectralEnabled(int i) noexcept')
h=once(h,
    '    std::array<juce::AudioParameterBool*,4> spectralEnabled {};',
    '    juce::AudioParameterBool* spectralDNAEnabled = nullptr;\n'
    '    juce::AudioParameterFloat* spectralDNAMix = nullptr;\n'
    '    std::array<juce::AudioParameterBool*,4> spectralEnabled {};')

p=source.read_text(encoding="utf-8")
p=once(p,
    '    morphActiveParam = new juce::AudioParameterBool(',
    '    // v0.55 host automation. Both OFF / 0 by default for legacy sessions.\n'
    '    spectralDNAEnabled = new juce::AudioParameterBool(\n'
    '        juce::ParameterID("libertySpectralDNAEnabled",1),\n'
    '        "Liberty SPECTRAL DNA Active", false);\n'
    '    addParameter(spectralDNAEnabled);\n'
    '    spectralDNAMix = new juce::AudioParameterFloat(\n'
    '        juce::ParameterID("libertySpectralDNAMix",1),\n'
    '        "Liberty SPECTRAL DNA Morph",\n'
    '        juce::NormalisableRange<float>(0.f,1.f),0.f);\n'
    '    addParameter(spectralDNAMix);\n'
    '    morphActiveParam = new juce::AudioParameterBool(')
p=once(p,
    '    const auto morphWeights = liberty::morph_v036::weights(orbitXY.x,orbitXY.y);',
    '    const auto morphWeights = liberty::morph_v036::weights(orbitXY.x,orbitXY.y);\n'
    '    const bool spectralDNAOn=spectralDNAEnabled->get() && spectralDNAMix->get()>0.f;\n'
    '    const float spectralDNAWet=juce::jlimit(0.f,1.f,spectralDNAMix->get());')
p=once(p,
    '        spectralParams.enabled=spectralEnabled[(size_t)layer]->get() && hybridParams.enabled;\n'
    '        spectralParams.mix=spectralMixes[(size_t)layer]->get();',
    '        // Reuse v0.48 harmonic resynthesis in this actual JUCE render path.\n'
    '        // No FFT, allocation, lock, worker or additional buffering here.\n'
    '        spectralParams.enabled=(spectralEnabled[(size_t)layer]->get() || spectralDNAOn)\n'
    '            && hybridParams.enabled;\n'
    '        const float legacySpectralMix=spectralMixes[(size_t)layer]->get();\n'
    '        const float layerDNAWeight=morphOn ? morphWeights[(size_t)layer] : .25f;\n'
    '        spectralParams.mix=liberty::spectral_dna_v055::spectralMixForLayer(\n'
    '            legacySpectralMix,layerDNAWeight,spectralDNAWet,spectralDNAOn);')
for name,value in [(header,h),(source,p)]:
    if '\ufffd' in value: raise SystemExit("Invalid replacement character in "+str(name))
    name.write_text(value,encoding="utf-8")
print("v0.55 SPECTRAL DNA: 4-layer real-time v0.48 JUCE wet morph integrated (default OFF).")
