#!/usr/bin/env python3
"""v0.55: copy the outgoing harmonics OFF audio and crossfade immutable tables in render().
Run after scripts/apply_v055_formants.py on reconstructed v0.48 JUCE source.
"""
from pathlib import Path
path=Path(__file__).resolve().parents[1]/"Source/SpectralEngineV048.h"
s=path.read_text("utf-8")
def once(old,new,label):
    global s
    matches=s.count(old)
    if matches!=1: raise SystemExit(f"v0.55 preset fade anchor {label}: got {matches}")
    s=s.replace(old,new,1)

once('#include "SpectralDNAFormantSlewV055.h"',
     '#include "SpectralDNAFormantSlewV055.h"\n#include "SpectralDNAPresetFadeV055.h"',"header")
once('    bool valid = false;',
'''    // Owned by immutable published Frame; prepared on preset loading thread.
    // Avoid dangling cross-block pointers to old/retired frames.
    std::array<std::array<float, tableSize>, bands> previousTables {};
    std::array<std::array<float, tableSize>, bands> previousDown {},previousUp {};
    bool hasPrevious=false;
    bool valid = false;''',"snapshot")
once('        wetState=0.f;\n        formantSlew.reset();\n        activeGeneration=0;',
'''        wetState=0.f;
        formantSlew.reset();
        presetFade.reset();
        activeGeneration=0;''',"prepare")
once('    void publish(std::unique_ptr<const Frame> frame) {\n        auto old=std::move(owned);',
'''    void publish(std::unique_ptr<const Frame> frame) {
        // Build a self-contained previous spectral snapshot before publishing.
        // A render callback never copies tables, holds refcounts or waits.
        const bool transition=bool(owned && owned->valid && frame && frame->valid);
        if(transition) {
            auto next=std::make_unique<Frame>(*frame);
            next->previousTables=owned->tables;
            next->previousDown=owned->formantDown;
            next->previousUp=owned->formantUp;
            next->hasPrevious=true;
            frame=std::move(next);
        }
        auto old=std::move(owned);''',"publish frame")
once('        published.store(owned.get(),std::memory_order_release);\n        generation.fetch_add(1,std::memory_order_acq_rel);',
'''        published.store(owned.get(),std::memory_order_release);
        transitionPublished.store(transition,std::memory_order_release);
        generation.fetch_add(1,std::memory_order_acq_rel);''',"publish ordering")
once('            wetState+=(target-wetState)*.004f;\n            const float currentFormant=formantSlew.next(formant);',
'''            wetState+=(target-wetState)*.004f;
            const float currentFormant=formantSlew.next(formant);
            const float tableCrossfade=presetFade.advance();''',"advance")
once('                tone+=double(raw*v.env*v.velocity);',
'''                if(frame->hasPrevious && tableCrossfade<1.f) {
                    const auto& prior=frame->previousTables[(size_t)v.band];
                    float oldRaw=prior[(size_t)a]*(1.f-f)+prior[(size_t)b]*f;
                    if(currentFormant!=0.f) {
                        const auto& variant=currentFormant<0.f
                            ? frame->previousDown[(size_t)v.band]
                            : frame->previousUp[(size_t)v.band];
                        const float shiftedOld=variant[(size_t)a]*(1.f-f)+variant[(size_t)b]*f;
                        oldRaw+=(shiftedOld-oldRaw)*std::abs(currentFormant);
                    }
                    raw=presetFade.mix(oldRaw,raw,tableCrossfade);
                }
                tone+=double(raw*v.env*v.velocity);''',"blend")
once('private:\n    liberty::spectral_dna_v055::FormantSlew formantSlew;',
'''private:
    liberty::spectral_dna_v055::FormantSlew formantSlew;
    liberty::spectral_dna_v055::PresetFade presetFade;
    std::atomic<bool> transitionPublished{false};''',"fade state")
once('            for(auto& voice:voices) voice=Voice{};\n            sustain.fill(false);\n            wetState=0.f;\n            formantSlew.reset();',
'''            if(transitionPublished.load(std::memory_order_acquire)) {
                // Preserve phase/envelopes of active voices while table content
                // fades over 256 samples. MIDI note-off continues to function.
                presetFade.start();
            } else {
                presetFade.reset();
                for(auto& voice:voices) voice=Voice{};
                sustain.fill(false);
                wetState=0.f;
                formantSlew.reset();
            }''',"generation")
# Preserve the default old sound path with no valid old spectrum.
path.write_text(s,"utf-8")
print("v0.55: immutable old spectra captured outside audio; 256-sample crossfade in actual v0.48 render.")
