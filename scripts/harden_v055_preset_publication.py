#!/usr/bin/env python3
"""Make v0.55 renderer use a consistent immutable frame+generation snapshot.

The old v0.48 syncGeneration() ran BEFORE readers++/published.load().
Concurrent publish could otherwise make a callback see a NEW frame and an
OLD generation, starting the fade one block too late. Only one producer
is supported, as in the existing v0.48 publication contract.
Run after apply_v055_preset_crossfade.py.
"""
from pathlib import Path
path=Path(__file__).resolve().parents[1]/"Source/SpectralEngineV048.h"
s=path.read_text("utf-8")
def once(old,new,name):
    global s
    n=s.count(old)
    if n!=1: raise SystemExit(f"frame epoch anchor {name}: {n}")
    s=s.replace(old,new,1)

once('    bool hasPrevious=false;',
     '    bool hasPrevious=false;\n    std::uint64_t publicationId=0;', "frame epoch")
once('''        if(transition) {
            auto next=std::make_unique<Frame>(*frame);
            next->previousTables=owned->tables;''',
'''        if(frame) {
            // Clone and stamp immutable data BEFORE the atomic pointer store.
            auto next=std::make_unique<Frame>(*frame);
            next->publicationId=generation.load(std::memory_order_relaxed)+1;
            if(transition) {
                next->previousTables=owned->tables;''',"publish clone")
once('''            next->previousUp=owned->formantUp;
            next->hasPrevious=true;
            frame=std::move(next);
        }''',
'''                next->previousUp=owned->formantUp;
                next->hasPrevious=true;
            }
            frame=std::move(next);
        }''',"publish clone end")
once('        transitionPublished.store(transition,std::memory_order_release);\n','',"remove redundant flag store")
once('    std::atomic<bool> transitionPublished{false};\n','',"remove redundant flag")
once('''    void noteOn(int ch,int note,int velocity,const Parameters& p) noexcept {
        syncGeneration();''',
'''    void noteOn(int ch,int note,int velocity,const Parameters& p) noexcept {
        // Same protection as render(): the publication epoch is read from the
        // exact frame loaded under the existing reader reclamation guard.
        readers.fetch_add(1,std::memory_order_acq_rel);
        const auto* noteFrame=published.load(std::memory_order_acquire);
        syncGeneration(noteFrame);
        readers.fetch_sub(1,std::memory_order_release);''',"noteOn guard")
once('''    void render(float* left,float* right,int start,int count,const Parameters& p) noexcept {
        syncGeneration();
        if(!left || count<=0) return;
        readers.fetch_add(1,std::memory_order_acq_rel);
        const auto* frame=published.load(std::memory_order_acquire);''',
'''    void render(float* left,float* right,int start,int count,const Parameters& p) noexcept {
        if(!left || count<=0) return;
        readers.fetch_add(1,std::memory_order_acq_rel);
        const auto* frame=published.load(std::memory_order_acquire);
        syncGeneration(frame);''',"render guard")
once('''    void syncGeneration() noexcept {
        const auto gen=generation.load(std::memory_order_acquire);''',
'''    void syncGeneration(const Frame* frame) noexcept {
        const auto gen=frame ? frame->publicationId :
            generation.load(std::memory_order_acquire);''',"generation source")
once('if(transitionPublished.load(std::memory_order_acquire)) {',
     'if(frame && frame->hasPrevious) {',"transition source")
if 'syncGeneration()' in s:
    raise SystemExit("Unconverted syncGeneration() calls remain")
path.write_text(s,"utf-8")
print("v0.55: spectral preset publication uses consistent frame epoch under reader guard.")
