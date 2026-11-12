// Native JUCE: real sample voices, per-layer granular wet/dry and project recall.
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
static void require(bool b,const char* what) {
    if(!b) throw std::runtime_error(what);
}
int main(int argc,char** argv) {
    if(argc!=3) {std::cerr<<"Usage: liberty-granular-v047 <Banks-folder> <report.json>\n";return 2;}
    try {
        ::setenv("LIBERTY_ROMPLER_BANK_DIR",argv[1],1);
        juce::ScopedJuceInitialiser_GUI init;
        constexpr double rate=44100.0;
        constexpr int block=512;
        LibertyRomplerAudioProcessor dry, wet;
        dry.prepareToPlay(rate,block);
        wet.prepareToPlay(rate,block);
        require(dry.selectPreset("factory-starter","aurora-pad").wasOk(),"missing dry factory preset");
        require(wet.selectPreset("factory-starter","aurora-pad").wasOk(),"missing wet factory preset");
        for(int layer=1;layer<4;++layer) {
            dry.setLayerSource(layer,{},{});
            wet.setLayerSource(layer,{},{});
        }
        auto& switchParam=wet.getLayerGrainEnabled(0);
        auto& wetParam=wet.getLayerGrainWet(0);
        static_cast<juce::AudioProcessorParameter&>(switchParam).setValue(1.f);
        static_cast<juce::AudioProcessorParameter&>(wetParam).setValue(.75f);
        static_cast<juce::AudioProcessorParameter&>(wet.getTimeSpeed()).setValue(2.f/3.f);
        require(wet.getLayerGrainEnabled(0).get(),"layer granular mode not enabled");
        juce::AudioBuffer<float> dryAudio(2,block),wetAudio(2,block);
        double difference=0.,dryEnergy=0.,wetEnergy=0.,maximum=0.;
        int nonfinite=0;
        for(int b=0;b<45;++b) {
            juce::MidiBuffer first,second;
            if(b==0) {
                first.addEvent(juce::MidiMessage::noteOn(1,48,(juce::uint8)110),0);
                second.addEvent(juce::MidiMessage::noteOn(1,48,(juce::uint8)110),0);
            }
            dryAudio.clear();wetAudio.clear();
            dry.processBlock(dryAudio,first);
            wet.processBlock(wetAudio,second);
            for(int ch=0;ch<2;++ch) for(int frame=0;frame<block;++frame) {
                const auto d=dryAudio.getSample(ch,frame),w=wetAudio.getSample(ch,frame);
                if(!std::isfinite(d)||!std::isfinite(w)) ++nonfinite;
                else {
                    difference+=std::abs(double(d-w));
                    dryEnergy+=std::abs(double(d));
                    wetEnergy+=std::abs(double(w));
                    maximum=std::max(maximum,std::max(std::abs(double(d)),std::abs(double(w))));
                }
            }
        }
        juce::MemoryBlock state;
        wet.getStateInformation(state);
        LibertyRomplerAudioProcessor restored;
        restored.prepareToPlay(rate,block);
        restored.setStateInformation(state.getData(),int(state.getSize()));
        const bool recall=restored.getLayerGrainEnabled(0).get()
            && std::abs(restored.getLayerGrainWet(0).get()-.75f)<.002f
            && !restored.getLayerGrainEnabled(1).get()
            && restored.getHybridMode(0).getIndex()==0;
        std::ofstream file(argv[2]);
        file<<"{\"version\":\"0.47\",\"test\":\"per-layer-granular-native\","
            <<"\"dry_energy\":"<<dryEnergy<<",\"wet_energy\":"<<wetEnergy<<","
            <<"\"sum_difference\":"<<difference<<",\"peak\":"<<maximum<<","
            <<"\"nonfinite_samples\":"<<nonfinite<<","
            <<"\"project_recall\":"<<(recall?"true":"false")<<"}\n";
        file.close();
        require(bool(file),"cannot write JSON");
        require(nonfinite==0,"nonfinite samples");
        require(dryEnergy>1. && wetEnergy>1.,"silent sample audio");
        require(difference>0.1,"granular did not change the audio");
        require(maximum<4.,"unbounded audio amplitude");
        require(recall,"granular project recall failed");
        std::cout<<"v0.47 native granular audio, stereo, wet/dry and recall passed\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr<<"v0.47 granular native FAILED: "<<e.what()<<"\n";
        return 4;
    }
}
