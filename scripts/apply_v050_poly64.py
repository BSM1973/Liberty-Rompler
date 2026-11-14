#!/usr/bin/env python3
"""Integrate bounded poly64 RAM transitions after all v0.49 overlays.

No DSP changes to small WAVs, no bank-format changes, no disk operations in
processBlock. Stage loop heads and extended attack windows at preset load.
This is intentionally a small, strict, repeatable overlay over generated source.
"""
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
SRC=ROOT/'Source'

def edit(path, patches):
    p=SRC/path
    code=p.read_text(encoding='utf-8')
    for old, new in patches:
        if code.count(old)!=1:
            raise SystemExit(f'v0.50 unexpected baseline for {path}: {old[:100]!r}, count={code.count(old)}')
        code=code.replace(old,new,1)
    p.write_text(code,encoding='utf-8')

edit('SampleEngine.h', [
    ('#include "SliceWarmupV049.h"', '#include "SliceWarmupV049.h"\n#include "LoopTransitionV050.h"'),
    ('    juce::AudioBuffer<float> data;\n',
     '''    juce::AudioBuffer<float> data;
    // v0.50: immutable loop-head PCM. Loaded once OFF the audio thread;
    // read-only on the callback. Zero extra cost for non-looping WAVs.
    juce::AudioBuffer<float> loopHead;
    int loopHeadStart = -1;
'''),
    ('    int getSliceWarmupEnd(int chosenSlice) const noexcept {',
     '''    int getLoopWarmupEnd() const noexcept {
        return loopHeadStart >= 0 ? loopHeadStart + loopHead.getNumSamples() : 0;
    }
    int getSliceWarmupEnd(int chosenSlice) const noexcept {''')
])

edit('SampleEngine.cpp', [
    ('    // Long WAVs preload 0.75 seconds, or a bounded 3.25 seconds for SLICE leaders.',
     '    // v0.50 extends only non-SLICE long-WAV attacks, keeping the v0.49 leader policy.'),
    ('                                               : reader.sampleRate * 0.75))',
     '                                               : std::min(147456.0, reader.sampleRate * 1.50)))'),
    ('    if (longSample) {\n        streamed = std::make_unique<liberty::stream_v040::StreamedZone>(std::move(source));',
     '''    if (longSample) {
        // Reserve a short loop boundary buffer before handing the reader to
        // the worker. This avoids 64 simultaneous random seeks at MIDI loop
        // wrap, including crossfade reads, without changing SLICE warmups.
        const auto loop = liberty::stream_v050::planLoopWindow(
            totalFrames, loopStart, loopEnd, attackFrames, sourceRate);
        if (loop.frames >= 2) {
            loopHead.setSize((int)reader.numChannels, loop.frames);
            if (reader.read(&loopHead,0,loop.frames,loop.begin,true,true))
                loopHeadStart=loop.begin;
            else loopHead.setSize(0,0); // best effort, old streaming fallback
        }
        streamed = std::make_unique<liberty::stream_v040::StreamedZone>(std::move(source));'''),
    ('        if (n < attackFrames) {',
     '''        if (loopHeadStart >= 0 && n >= loopHeadStart &&
            n-loopHeadStart < loopHead.getNumSamples()) {
            const auto at = (int)(n-loopHeadStart);
            l = loopHead.getReadPointer(0)[at];
            r = loopHead.getReadPointer(loopHead.getNumChannels()>1 ? 1 : 0)[at];
            return true;
        }
        if (n < attackFrames) {'''),
    ('    const double wanted = warmedEnd > position ?\n        std::max(position, (double)warmedEnd-8192.0) : position;',
     '''    const int loopWarmedEnd = !sliceVoice ? s->getLoopWarmupEnd() : 0;
    const double wanted = warmedEnd > position ?
        std::max(position, (double)warmedEnd-8192.0) :
        (loopWarmedEnd > position && position >= s->getLoopStart()
            ? std::max(position, (double)loopWarmedEnd-8192.0) : position);'''),
    ('        s->requestStreamAhead(s->getLoopStart());\n'
     '        // Time-critical crossfade/loop-head demand near a wrap boundary.\n'
     '        if (position + count*timeRatio >=\n'
     '            s->getLoopEnd() - 2.0*s->getLoopCrossfadeFrames())\n'
     '            s->requestStreamCritical(s->getLoopStart());',
     '''        // The immutable v0.50 loop head supplies crossfades without
        // a seek. Preserve v0.49 requests only if preparation failed.
        if (s->getLoopWarmupEnd() <= s->getLoopStart()) {
            s->requestStreamAhead(s->getLoopStart());
            if (position + count*timeRatio >=
                s->getLoopEnd() - 2.0*s->getLoopCrossfadeFrames())
                s->requestStreamCritical(s->getLoopStart());
        }''')
])
for relative in ('SampleEngine.cpp','SampleEngine.h'):
    if b'\xef\xbf\xbd' in (SRC/relative).read_bytes():
        raise SystemExit('Bad French text encoding in '+relative)
print('v0.50 poly64 attack+loop RAM transitions integrated; SLICE v0.49 preserved.')
