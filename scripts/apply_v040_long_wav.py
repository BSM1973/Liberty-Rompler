#!/usr/bin/env python3
"""Liberty v0.40: integrate actual long-WAV streaming after v0.39 sources.
The previous <=20 s in-memory voice remains byte-for-byte in its rendering path.
The long-sample voice is experimental; it never opens or reads disk in callback.
"""
from pathlib import Path
ROOT = Path(__file__).resolve().parent.parent
S = ROOT / "Source"

def patch(file, changes):
    p = S / file
    data = p.read_text(encoding="utf-8")
    for before, after in changes:
        count = data.count(before)
        if count != 1:
            raise SystemExit(f"v0.40: {file} expected one anchor, got {count}: {before[:100]!r}")
        data = data.replace(before, after, 1)
    p.write_text(data, encoding="utf-8")


patch("SampleEngine.h", [
    (
        '#include "CreativeEngineV038.h"',
        '#include "CreativeEngineV038.h"\n#include "StreamedZoneV040.h"\n#include <cstdint>',
    ),
    (
        'MappedSampleSound(const ZoneDescriptor& zone, juce::AudioFormatReader& reader,',
        'MappedSampleSound(const ZoneDescriptor& zone,\n'
        '                      std::unique_ptr<juce::AudioFormatReader> source,',
    ),
    (
        '    bool isLoaded() const { return data.getNumSamples() > 1; }',
        '''    bool isLoaded() const { return data.getNumSamples() > 1; }
    bool isStreamed() const noexcept { return streamed != nullptr; }
    int getTotalFrames() const noexcept { return totalFrames; }
    void requestStreamAhead(double pos) const noexcept {
        if (streamed && pos >= 0.0 && pos < (double)totalFrames)
            streamed->requestAhead((std::int64_t)pos);
    }
    // Entirely lock-free, no JUCE decoder touches audio callback.
    bool readStereoAt(double position, float& left, float& right) const noexcept;
''',
    ),
    (
        '    juce::AudioBuffer<float> data;\n    double sourceRate = 44100.0;',
        '    juce::AudioBuffer<float> data;\n'
        '    std::unique_ptr<liberty::stream_v040::StreamedZone> streamed;\n'
        '    int totalFrames = 0, attackFrames = 0;\n'
        '    double sourceRate = 44100.0;',
    ),
    (
        '    void updatePitch();',
        '    void updatePitch();\n'
        '    void renderStreamedNextBlock(juce::AudioBuffer<float>& out, int start, int count);\n'
        '    float lastStreamLeft=0.0f, lastStreamRight=0.0f, streamGain=1.0f;',
    ),
])

constructor_old = '''MappedSampleSound::MappedSampleSound(const ZoneDescriptor& z, juce::AudioFormatReader& reader,
                                     const LayerControls& layer, int layerIndex,
                                     const PerformanceControls& global, bool sliceLeader)
    : sourceRate(reader.sampleRate), rootNote(z.rootNote), lowNote(z.lowNote),'''
constructor_new = '''MappedSampleSound::MappedSampleSound(const ZoneDescriptor& z,
                                     std::unique_ptr<juce::AudioFormatReader> source,
                                     const LayerControls& layer, int layerIndex,
                                     const PerformanceControls& global, bool sliceLeader)
    : sourceRate(source ? source->sampleRate : 44100.0),
      rootNote(z.rootNote), lowNote(z.lowNote),'''
body_old = '''    // Prototype memory cap: up to 20 seconds per sample zone.
    const auto count = (int)juce::jmin(reader.lengthInSamples,
                                      (juce::int64)(reader.sampleRate * 20.0));
    if (count < 2 || reader.numChannels < 1 || reader.numChannels > 2) return;
    data.setSize((int)reader.numChannels, count + 1);
    data.clear();
    if (!reader.read(&data, 0, count, 0, true, true)) data.setSize(0, 0);
    if (loopStart < 0 || loopEnd <= loopStart + 10 || loopEnd > count) {'''
body_new = '''    if (!source) return;
    auto& reader = *source;
    const bool longSample = reader.lengthInSamples > (juce::int64)(reader.sampleRate * 20.0);
    // The v0.40 pilot uses int32 sample/loop endpoints elsewhere in JUCE.
    // Explicitly REJECT >INT_MAX-2 sources instead of truncating them.
    if (reader.lengthInSamples > (juce::int64)std::numeric_limits<int>::max()-2 ||
        reader.lengthInSamples < 2 || reader.numChannels < 1 ||
        reader.numChannels > 2 || reader.sampleRate <= 0) return;
    totalFrames = static_cast<int>(reader.lengthInSamples);
    // Legacy small WAVs retain their entire original RAM loading path.
    // Large WAVs preload 0.75 seconds to cover seek and thread scheduling.
    const auto count = longSample
        ? (int)juce::jmin(reader.lengthInSamples,
                         (juce::int64)(reader.sampleRate * 0.75))
        : (int)juce::jmin(reader.lengthInSamples,
                         (juce::int64)(reader.sampleRate * 20.0));
    if (count < 2) return;
    data.setSize((int)reader.numChannels, count + 1);
    data.clear();
    if (!reader.read(&data, 0, count, 0, true, true)) {
        data.setSize(0, 0); return;
    }
    attackFrames = count;
    if (longSample) {
        streamed = std::make_unique<liberty::stream_v040::StreamedZone>(std::move(source));
        if (!streamed->isValid()) {
            streamed.reset(); data.setSize(0, 0); return;
        }
        streamed->start(attackFrames);
    }
    const int validFrames = longSample ? totalFrames : count;
    if (loopStart < 0 || loopEnd <= loopStart + 10 || loopEnd > validFrames) {'''
read_method = '''
bool MappedSampleSound::readStereoAt(double pos, float& left, float& right) const noexcept {
    left = right = 0.0f;
    if (!std::isfinite(pos) || pos < 0.0 || pos >= (double)(totalFrames-1))
        return false;
    const auto frame = (std::int64_t)pos;
    const auto alpha = (float)(pos - (double)frame);
    auto get = [&](std::int64_t n, float& l, float& r) noexcept -> bool {
        if (n < 0 || n >= totalFrames) return false;
        if (n < attackFrames) {
            const int index = (int)n;
            l = data.getReadPointer(0)[index];
            r = data.getReadPointer(data.getNumChannels()>1 ? 1 : 0)[index];
            return true;
        }
        return streamed != nullptr && streamed->tryReadFrame(n, l, r);
    };
    float l0=0.0f, r0=0.0f, l1=0.0f, r1=0.0f;
    if (!get(frame, l0, r0) || !get(frame+1, l1, r1)) return false;
    left = l0 + alpha*(l1-l0);
    right = r0 + alpha*(r1-r0);
    return true;
}
'''
stream_render = r'''
void SampleVoice::renderStreamedNextBlock(juce::AudioBuffer<float>& out,
                                          int start, int count) {
    auto* s = dynamic_cast<MappedSampleSound*>(getCurrentlyPlayingSound().get());
    if (!s || !s->isStreamed()) return;
    const int last = s->getTotalFrames() - 1;
    if (last < 2) { clearCurrentNote(); return; }
    const int channels = juce::jmin(2, out.getNumChannels());
    if (channels <= 0) return;
    const auto& layer = s->getLayer();
    const float sampleRate = (float)juce::jmax(8000.0, getSampleRate());
    const float cutoff = juce::jlimit(80.0f, sampleRate*0.45f,
        layer.cutoffHz.load(std::memory_order_relaxed));
    const bool filtered = layer.cutoffHz.load(std::memory_order_relaxed)<19500.0f;
    if (filtered && std::abs(cutoff-previousCutoff)>0.1f) {
        const auto coefficients = juce::IIRCoefficients::makeLowPass(sampleRate,cutoff,0.707f);
        for(auto& f: layerFilter) f.setCoefficients(coefficients);
        previousCutoff=cutoff;
    }
    updatePitch();
    const float volume=layer.gain.load(std::memory_order_relaxed);
    const float pan=juce::jlimit(-1.0f,1.0f,layer.pan.load(std::memory_order_relaxed));
    const float angle=(pan+1.0f)*juce::MathConstants<float>::pi*0.25f;
    const float leftGain=std::cos(angle)*std::sqrt(2.0f);
    const float rightGain=std::sin(angle)*std::sqrt(2.0f);
    const bool granular=controls.granularActive.load(std::memory_order_relaxed);
    const int grainFrames=liberty::creative_v038::grainLength(
        controls.grainMilliseconds.load(std::memory_order_relaxed),getSampleRate());
    const double timeSpeed=juce::jlimit(0.5,2.0,(double)controls.timeSpeed.load(
        std::memory_order_relaxed));
    const double timeRatio=granular?ratio*timeSpeed:ratio;

    // Request current and near-future pages once per render block rather
    // than enqueuing from the inner per-sample hot path.
    s->requestStreamAhead(position);
    s->requestStreamAhead(position+count*timeRatio+grainFrames*ratio);
    if (s->getLoopStart()>=0) s->requestStreamAhead(s->getLoopStart());
    for(int i=0;i<count;++i) {
        if (position>=last || (sliceVoice && position>=sliceEnd)) {
            clearCurrentNote(); break;
        }
        const float amp=envelope.getNextSample()*velocityGain;
        float left=0.0f,right=0.0f;
        bool ok=false;
        if(granular) {
            const auto grain=liberty::creative_v038::grainAt(
                position,grainElapsed,grainFrames,ratio,timeRatio);
            const auto readGrain=[&](double sourcePosition,float& l,float& r) noexcept {
                const double readPos=sliceVoice?
                    juce::jlimit((double)sliceBegin,(double)sliceEnd-1.00001,sourcePosition)
                  :liberty::creative_v038::wrapFrame(sourcePosition,
                    s->getLoopStart(),s->getLoopEnd(),last);
                return s->readStereoAt(readPos,l,r);
            };
            float aL=0.0f,aR=0.0f,bL=0.0f,bR=0.0f;
            ok=readGrain(grain.positionA,aL,aR) &&
               readGrain(grain.positionB,bL,bR);
            if(ok) {
                left=aL*grain.gainA+bL*grain.gainB;
                right=aR*grain.gainA+bR*grain.gainB;
            }
        } else {
            ok=s->readStereoAt(position,left,right);
            const auto blend=sliceVoice?liberty::loopfade::Blend{}:
                liberty::loopfade::at(position,s->getLoopStart(),
                                      s->getLoopEnd(),s->getLoopCrossfadeFrames());
            if (ok && blend.fraction>0.0f) {
                float hL=0.0f,hR=0.0f;
                if (s->readStereoAt(blend.headPosition,hL,hR)) {
                    left+=(hL-left)*blend.fraction;
                    right+=(hR-right)*blend.fraction;
                } else ok=false;
            }
        }
        // On underflow hold and fade the last sample toward silence smoothly.
        // When the cache recovers, ramp back for 64 samples; never block audio.
        if (ok) {
            lastStreamLeft=left; lastStreamRight=right;
            streamGain=juce::jmin(1.0f,streamGain+1.0f/64.0f);
        } else {
            streamGain=juce::jmax(0.0f,streamGain-1.0f/64.0f);
            left=lastStreamLeft; right=lastStreamRight;
        }
        const float sliceTail=sliceVoice?juce::jlimit(0.0f,1.0f,
            (float)((sliceEnd-position)/juce::jmax(1.0,64.0*ratio))):1.0f;
        float a=left*amp*volume*streamGain*sliceTail*leftGain;
        float b=right*amp*volume*streamGain*sliceTail*rightGain;
        if(filtered) {
            a=layerFilter[0].processSingleSampleRaw(a);
            b=layerFilter[1].processSingleSampleRaw(b);
        }
        out.addSample(0,start+i,a);
        if(channels>1) out.addSample(1,start+i,b);
        position+=timeRatio;
        if(granular) ++grainElapsed;
        if(!sliceVoice && s->getLoopStart()>=0 && position>=s->getLoopEnd())
            position=liberty::loopfade::wrap(position,s->getLoopStart(),
                                             s->getLoopEnd(),s->getLoopCrossfadeFrames());
        if(!envelope.isActive()) { clearCurrentNote(); break; }
    }
}
'''
patch("SampleEngine.cpp", [
    (constructor_old, constructor_new),
    (body_old, body_new),
    ('#include <cmath>', '#include <cmath>\n#include <limits>'),
    ('bool MappedSampleSound::appliesToNote(int n) {', read_method + '\nbool MappedSampleSound::appliesToNote(int n) {'),
    ('    sliceEnd = s->getAudio().getNumSamples()-1;',
     '    sliceEnd = s->getTotalFrames()-1;\n'
     '    lastStreamLeft = lastStreamRight = 0.0f;\n'
     '    streamGain = 1.0f;'),
    ('    updatePitch();\n}\nvoid SampleVoice::updatePitch() {',
     '    updatePitch();\n'
     '    if (s->isStreamed()) {\n'
     '        s->requestStreamAhead(position);\n'
     '        s->requestStreamAhead(s->getTotalFrames()>0 ? (double)s->getAudio().getNumSamples()-1.0 : 0.0);\n'
     '    }\n'
     '}\nvoid SampleVoice::updatePitch() {'),
    ('    if (!s || !s->isLoaded()) return;\n    const auto& audio = s->getAudio();',
     '    if (!s || !s->isLoaded()) return;\n'
     '    if (s->isStreamed()) { renderStreamedNextBlock(out,start,count); return; }\n'
     '    const auto& audio = s->getAudio();'),
    ('} // namespace liberty', stream_render + '\n} // namespace liberty'),
])

patch("PluginProcessor.cpp", [
    ('new liberty::MappedSampleSound(zone, *reader, controls.layers[(size_t)i], i,',
     'new liberty::MappedSampleSound(zone, std::move(reader), controls.layers[(size_t)i], i,'),
])
# Two pre-existing historical tests check the exact old constructor syntax,
# not playback behavior. Their assertions are updated to the move-owned reader.
for test in ("tests/test_v038_creative.py", "tests/test_samples_v011.py"):
    p=ROOT/test
    data=p.read_text(encoding="utf-8")
    old='new liberty::MappedSampleSound(zone, *reader'
    new='new liberty::MappedSampleSound(zone, std::move(reader)'
    if data.count(old)!=1:
        raise SystemExit(f"v0.40 cannot migrate old test anchor: {test}")
    p.write_text(data.replace(old,new,1),encoding="utf-8")

# Long WAVs are now accepted by the offline manifest validator and the
# standalone Smart Import used by the v0.40 release.  The int32 voice loop
# implementation rejects >2,147,483,645 frames rather than truncating.
validator=ROOT/"tools/validate_banks.py"
code=validator.read_text(encoding="utf-8")
a="if frames < 2 or frames > wav.getframerate() * 20:"
b="if frames < 2 or frames > 2147483645:"
if code.count(a)!=1: raise SystemExit("v0.40: validator length anchor changed")
validator.write_text(code.replace(a,b,1),encoding="utf-8")

importer=ROOT/"tools/smart_import_v037.py"
code=importer.read_text(encoding="utf-8")
a="""    # The v0.36 engine retains its 20-second per-zone memory cap.
    # Never silently truncate long sounds and call it disk streaming.
    if frames > rate * 20:
        raise ImportErrorWithContext(
            "WAV longer than 20 s: dedicated disk streaming not active yet")"""
b="""    # v0.40: long WAV pilot uses a streaming reader; source sample offsets
    # remain int32 for loop edit compatibility. Never silently truncate.
    if frames > 2147483645:
        raise ImportErrorWithContext(
            "WAV longer than supported int32 frame limit (2,147,483,645)")"""
if code.count(a)!=1: raise SystemExit("v0.40: smart importer length anchor changed")
importer.write_text(code.replace(a,b,1),encoding="utf-8")

print("v0.40: real long-WAV streaming connected; RAM legacy unchanged.")
