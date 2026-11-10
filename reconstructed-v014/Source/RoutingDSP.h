#pragma once
#include "SampleEngine.h"
#include <array>
#include <cmath>

namespace liberty {
// Fixed-size, audio-thread-safe effect rack. Each layer and the master bus owns
// completely independent delay, reverb, chorus, compressor and filter histories.
// IDs: 0 bypass, 1 Drive, 2 EQ, 3 Compressor, 4 Chorus, 5 Delay, 6 Reverb.
class RoutingFXRack final {
public:
    void prepare(double sampleRate, int maxBlock) {
        rate = juce::jmax(8000.0, sampleRate);
        const juce::dsp::ProcessSpec spec { rate, (juce::uint32)juce::jmax(1,maxBlock), 2 };
        chorus.prepare(spec);
        chorus.setCentreDelay(7.0f);
        chorus.setFeedback(0.0f);
        chorus.reset();
        reverb.prepare(spec); reverb.reset();
        compressor.prepare(spec); compressor.reset();
        for (auto* group : {&low, &mid, &high})
            for (auto& filter : *group) filter.reset();
        delay.setSize(2, juce::jmax(2, juce::roundToInt(rate*2.0)+1));
        delay.clear();
        writeIndex=0;
        driveMemory.fill(0.0f);
    }
    void process(int effectId, juce::AudioBuffer<float>& audio, const PerformanceControls& c, int numSamples=-1) {
        const int n = numSamples<0 ? audio.getNumSamples() : juce::jmin(numSamples,audio.getNumSamples());
        const int channels = juce::jmin(2, audio.getNumChannels());
        if (n == 0 || channels != 2) return;
        if (effectId == 1) {
            const auto wet = juce::jlimit(0.0f, 1.0f, c.distortionMix.load());
            if (wet <= 0.0001f) return;
            const float gain=juce::Decibels::decibelsToGain(juce::jlimit(0.0f, 30.0f, c.distortionDriveDb.load()));
            const float cutoff=juce::jlimit(500.0f, (float)rate*0.45f, c.distortionToneHz.load());
            const float coef=std::exp(-juce::MathConstants<float>::twoPi*cutoff/(float)rate);
            for (int ch=0;ch<2;++ch) {
                float memory=driveMemory[(size_t)ch];
                float* samples=audio.getWritePointer(ch);
                for (int i=0;i<n;++i) {
                    const float dry=samples[i];
                    memory=coef*memory+(1.0f-coef)*std::tanh(gain*dry);
                    samples[i]=(1.0f-wet)*dry+wet*memory;
                }
                driveMemory[(size_t)ch]=memory;
            }
        } else if (effectId == 2) {
            const float lowDb=juce::jlimit(-12.0f,12.0f,c.eqLowDb.load());
            const float midDb=juce::jlimit(-12.0f,12.0f,c.eqMidDb.load());
            const float highDb=juce::jlimit(-12.0f,12.0f,c.eqHighDb.load());
            if (std::abs(lowDb)+std::abs(midDb)+std::abs(highDb)<0.001f) return;
            const auto lo=juce::IIRCoefficients::makeLowShelf(rate,140.0,0.707,juce::Decibels::decibelsToGain(lowDb));
            const auto mi=juce::IIRCoefficients::makePeakFilter(rate,1100.0,0.85,juce::Decibels::decibelsToGain(midDb));
            const auto hi=juce::IIRCoefficients::makeHighShelf(rate,7600.0,0.707,juce::Decibels::decibelsToGain(highDb));
            for (int ch=0;ch<2;++ch) {
                low[(size_t)ch].setCoefficients(lo);
                mid[(size_t)ch].setCoefficients(mi);
                high[(size_t)ch].setCoefficients(hi);
                auto* ptr=audio.getWritePointer(ch);
                low[(size_t)ch].processSamples(ptr,n);
                mid[(size_t)ch].processSamples(ptr,n);
                high[(size_t)ch].processSamples(ptr,n);
            }
        } else if (effectId == 3) {
            const float ratio=juce::jlimit(1.0f,12.0f,c.compRatio.load());
            if (ratio > 1.001f) {
                compressor.setThreshold(juce::jlimit(-48.0f,0.0f,c.compThresholdDb.load()));
                compressor.setRatio(ratio);
                compressor.setAttack(15.0f); compressor.setRelease(130.0f);
                juce::dsp::AudioBlock<float> block(audio);
                auto region=block.getSubBlock(0,(size_t)n);
                compressor.process(juce::dsp::ProcessContextReplacing<float>(region));
            }
            const float makeup=juce::jlimit(0.0f,18.0f,c.compMakeupDb.load());
            if (makeup>0.001f) audio.applyGain(0,n,juce::Decibels::decibelsToGain(makeup));
        } else if (effectId == 4) {
            const float wet=juce::jlimit(0.0f,1.0f,c.chorusMix.load());
            if (wet<=0.0001f) return;
            chorus.setRate(juce::jlimit(0.05f,8.0f,c.chorusRate.load()));
            chorus.setDepth(juce::jlimit(0.0f,1.0f,c.chorusDepth.load()));
            chorus.setMix(wet);
            juce::dsp::AudioBlock<float> block(audio);
            auto region=block.getSubBlock(0,(size_t)n);
            chorus.process(juce::dsp::ProcessContextReplacing<float>(region));
        } else if (effectId == 5) {
            if (delay.getNumSamples()<2) return;
            const int cap=delay.getNumSamples();
            const int offset=juce::jlimit(1,cap-1,juce::roundToInt(juce::jlimit(60.0f,1500.0f,c.delayTimeMs.load())*(float)rate*0.001f));
            const float fb=juce::jlimit(0.0f,0.85f,c.delayFeedback.load());
            const float wet=juce::jlimit(0.0f,1.0f,c.delayMix.load());
            auto* left=audio.getWritePointer(0),*right=audio.getWritePointer(1);
            auto* dl=delay.getWritePointer(0),*dr=delay.getWritePointer(1);
            for (int i=0;i<n;++i) {
                int read=writeIndex-offset;
                if (read<0) read+=cap;
                const float dryL=left[i],dryR=right[i],echoL=dl[read],echoR=dr[read];
                dl[writeIndex]=dryL+echoL*fb; dr[writeIndex]=dryR+echoR*fb;
                left[i]=dryL*(1.0f-wet)+echoL*wet;
                right[i]=dryR*(1.0f-wet)+echoR*wet;
                if (++writeIndex==cap) writeIndex=0;
            }
        } else if (effectId == 6) {
            const float wet=juce::jlimit(0.0f,1.0f,c.reverbMix.load());
            if (wet<=0.0001f) return;
            juce::Reverb::Parameters p;
            p.roomSize=juce::jlimit(0.0f,1.0f,c.reverbRoom.load());
            p.damping=juce::jlimit(0.0f,1.0f,c.reverbDamping.load());
            p.wetLevel=wet*0.55f;
            p.dryLevel=1.0f;
            p.width=1.0f;
            p.freezeMode=0.0f;
            reverb.setParameters(p);
            juce::dsp::AudioBlock<float> block(audio);
            auto region=block.getSubBlock(0,(size_t)n);
            reverb.process(juce::dsp::ProcessContextReplacing<float>(region));
        }
    }
private:
    double rate=44100.0;
    juce::dsp::Chorus<float> chorus;
    juce::dsp::Reverb reverb;
    juce::dsp::Compressor<float> compressor;
    std::array<juce::IIRFilter,2> low,mid,high;
    juce::AudioBuffer<float> delay;
    int writeIndex=0;
    std::array<float,2> driveMemory {{0.0f,0.0f}};
};
}
