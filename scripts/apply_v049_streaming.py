#!/usr/bin/env python3
"""Apply v0.49 RT cache hint + bounded 8-SLICE RAM warmups after v0.48.

The v0.48 DSP and bank formats remain unchanged. Run only on reconstructed
source, once, after apply_v048_bundle.py and fix_v048_native_bench.py.
"""
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SRC=ROOT/'Source'

def edit(path, pairs):
    p=SRC/path
    s=p.read_text(encoding='utf-8')
    for before, after in pairs:
        n=s.count(before)
        if n!=1:
            raise SystemExit(f'v0.49 integration expected ONE anchor in {path}, found {n}: {before[:90]}')
        s=s.replace(before,after,1)
    p.write_text(s,encoding='utf-8')

edit('StreamingPageCacheV039.h', [
    ('        for (std::size_t i = 0; i < PageSlots; ++i) {\n            auto& slot = pages[i];\n            if (slot.page.load(std::memory_order_seq_cst) != index ||',
     '''        // Fast path: consecutive samples from the same page normally share
        // one resident slot. Hint is metadata only; pin+recheck below remains
        // mandatory, so concurrent evictions cannot expose torn audio.
        const auto preferred = hitSlotHint.load(std::memory_order_relaxed);
        auto readSlot = [&](std::size_t i) noexcept -> bool {
            auto& slot = pages[i];
            if (slot.page.load(std::memory_order_seq_cst) != index ||'''),
    ('                continue;\n            slot.readers.fetch_add(1, std::memory_order_seq_cst);',
     '                return false;\n            slot.readers.fetch_add(1, std::memory_order_seq_cst);'),
    ('                hits.fetch_add(1, std::memory_order_relaxed);\n                return true;',
     '''                hits.fetch_add(1, std::memory_order_relaxed);
                hitSlotHint.store(i, std::memory_order_relaxed);
                return true;'''),
    ('            slot.readers.fetch_sub(1, std::memory_order_seq_cst);\n        }\n        misses.fetch_add(1, std::memory_order_relaxed);',
     '''            slot.readers.fetch_sub(1, std::memory_order_seq_cst);
            return false;
        };
        if (readSlot(preferred)) return true;
        for (std::size_t i = 0; i < PageSlots; ++i)
            if (i != preferred && readSlot(i)) return true;
        misses.fetch_add(1, std::memory_order_relaxed);'''),
    ('    std::atomic<std::size_t> requestCursor {0};',
     '''    std::atomic<std::size_t> requestCursor {0};
    std::atomic<std::size_t> hitSlotHint {0}; // audio read hint; safe under eviction''')
])

edit('SampleEngine.h', [
    ('#include "CreativeEngineV038.h"', '#include "CreativeEngineV038.h"\n#include "SliceWarmupV049.h"'),
    ('    bool readStereoAt(double position, float& left, float& right) const noexcept;',
     '''    bool readStereoAt(double position, float& left, float& right,
                      int sliceIndex=-1) const noexcept;'''),
    ('    juce::AudioBuffer<float> data;\n    std::unique_ptr<liberty::stream_v040::StreamedZone> streamed;',
     '''    juce::AudioBuffer<float> data;
    // Only the slice-leading long-WAV zone on each layer owns these 7
    // immutable windows. Prepared by the preset loader, never in processBlock.
    std::array<juce::AudioBuffer<float>, 8> sliceWarmups;
    std::array<int,8> sliceWarmupStarts {};
    std::unique_ptr<liberty::stream_v040::StreamedZone> streamed;'''),
    ('    bool sliceVoice = false;', '    bool sliceVoice = false;\n    int sliceIndex = -1;')
])

edit('SampleEngine.cpp', [
    ('    attackFrames = count;\n    if (longSample) {',
     '''    attackFrames = count;
    // Bounded 7 distant SLICE attacks, decoded on the preset-loading thread.
    // Unlike page-cache requests, these can sound at the very first MIDI
    // frame even if all four shared workers are busy with disk I/O.
    if (longSample && leader) {
        for (int i=1; i<8; ++i) {
            const auto warm = liberty::stream_v049::planSliceWarmup(
                totalFrames, i, sourceRate);
            if (warm.frames < 2) continue;
            auto& buffer = sliceWarmups[(size_t)i];
            buffer.setSize((int)reader.numChannels, warm.frames);
            if (!reader.read(&buffer,0,warm.frames,warm.begin,true,true)) {
                buffer.setSize(0,0); continue;
            }
            sliceWarmupStarts[(size_t)i]=warm.begin;
        }
    }
    if (longSample) {'''),
    ('bool MappedSampleSound::readStereoAt(double pos, float& left, float& right) const noexcept {',
     '''bool MappedSampleSound::readStereoAt(double pos, float& left, float& right,
                                         int selectedSlice) const noexcept {'''),
    ('        if (n < attackFrames) {',
     '''        if (selectedSlice >= 1 && selectedSlice < 8) {
            const auto i = (size_t)selectedSlice;
            const auto& warm = sliceWarmups[i];
            const int first = sliceWarmupStarts[i];
            if (warm.getNumSamples() > 0 && n >= first &&
                n-first < warm.getNumSamples()) {
                const int at = (int)(n-first);
                l=warm.getReadPointer(0)[at];
                r=warm.getReadPointer(warm.getNumChannels()>1?1:0)[at];
                return true;
            }
        }
        if (n < attackFrames) {'''),
    ('    sliceVoice = s->isSliceMode();\n    sliceBegin = 0;',
     '''    sliceVoice = s->isSliceMode();
    sliceIndex = -1;
    sliceBegin = 0;'''),
    ('        sliceBegin=cut.begin; sliceEnd=cut.end; position=sliceBegin;',
     '''        sliceBegin=cut.begin; sliceEnd=cut.end; position=sliceBegin;
        sliceIndex=midiNoteNumber-36;'''),
    ('                return s->readStereoAt(readPos,l,r);',
     '                return s->readStereoAt(readPos,l,r,sliceIndex);'),
    ('            ok=s->readStereoAt(position,left,right);',
     '            ok=s->readStereoAt(position,left,right,sliceIndex);'),
    ('                if (s->readStereoAt(blend.headPosition,hL,hR)) {',
     '                if (s->readStereoAt(blend.headPosition,hL,hR,sliceIndex)) {')
])
for rel in ('SampleEngine.h','SampleEngine.cpp','StreamingPageCacheV039.h'):
    assert b'\xef\xbf\xbd' not in (SRC/rel).read_bytes()
print('Liberty v0.49 SLICE heads preloaded (only leaders) and lock-free cache hit hint integrated.')
