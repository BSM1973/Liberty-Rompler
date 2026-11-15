#!/usr/bin/env python3
"""v0.55: rapid re-publication starts from latest observed in-flight fade.

Apply after harden_v055_preset_publication.py. At a rapid second publication,
construct outgoing tables OFF the audio thread from the last atomic audio
progress (frame epoch + progress in one word). Never copy on audio callback.

The audio callback publishes a progress snapshot after each sample. For a
simultaneous mid-block publication this is the most recent observed position;
the producer cannot synchronously sample an exact future callback boundary.
"""
from pathlib import Path
p=Path(__file__).resolve().parents[1]/"Source/SpectralEngineV048.h"
s=p.read_text("utf-8")
def once(old,new,label):
    global s
    n=s.count(old)
    if n!=1:raise SystemExit(f"rapid preset blend anchor {label}: {n}")
    s=s.replace(old,new,1)

once('''        if(frame) {
            // Clone and stamp immutable data BEFORE the atomic pointer store.''',
'''        if(frame) {
            // Clone and stamp immutable data BEFORE the atomic pointer store.''',"snapshot")
once('''            if(transition) {
                next->previousTables=owned->tables;
                next->previousDown=owned->formantDown;
                next->previousUp=owned->formantUp;
                next->hasPrevious=true;
            }''',
'''            if(transition) {
                // An earlier 256-frame transition may still be audible.
                // The observed epoch and progress are one atomic value.
                const auto progress=audibleFade.load(std::memory_order_acquire);
                const auto seenEpoch=(progress>>16);
                const auto completed=unsigned(progress&0xffffu);
                const bool inFlight=owned->hasPrevious &&
                    seenEpoch==owned->publicationId && completed<256u;
                const float t=inFlight ? float(completed)/256.f : 1.f;
                const bool notYetObserved=owned->hasPrevious &&
                    seenEpoch!=owned->publicationId;
                if(inFlight || notYetObserved) {
                    const float weight=notYetObserved ? 0.f : t;
                    const auto accumulate=[weight](auto& dst,const auto& oldTables,
                                                   const auto& newTables) noexcept {
                        for(std::size_t band=0;band<dst.size();++band)
                            for(std::size_t sample=0;sample<dst[band].size();++sample)
                                dst[band][sample]=oldTables[band][sample]*(1.f-weight)
                                    +newTables[band][sample]*weight;
                    };
                    accumulate(next->previousTables,owned->previousTables,owned->tables);
                    accumulate(next->previousDown,owned->previousDown,owned->formantDown);
                    accumulate(next->previousUp,owned->previousUp,owned->formantUp);
                } else {
                    next->previousTables=owned->tables;
                    next->previousDown=owned->formantDown;
                    next->previousUp=owned->formantUp;
                }
                next->hasPrevious=true;
            }''',"blend previous snapshot")
once('''            const float tableCrossfade=presetFade.advance();''',
'''            const float tableCrossfade=presetFade.advance();
            if(frame && frame->hasPrevious) {
                const auto samplesDone=std::uint64_t(
                    std::clamp(int(std::lround(tableCrossfade*256.f)),0,256));
                audibleFade.store((frame->publicationId<<16)|samplesDone,
                                  std::memory_order_release);
            }''',"audio progress")
once('''    liberty::spectral_dna_v055::PresetFade presetFade;''',
'''    liberty::spectral_dna_v055::PresetFade presetFade;
    // High bits = the audible Frame epoch; low 16 bits = samples completed.
    // Only the render side writes this lock-free progress; the loader reads it.
    std::atomic<std::uint64_t> audibleFade{0};''',"atomic progress")
# When a new epoch is selected, its progress remains associated with the
# PREVIOUS epoch until the first post-publish audio sample advances.
p.write_text(s,"utf-8")
print("v0.55: rapid spectral preset re-publications blend outgoing audible progress outside audio.")
