// Native offline stress of the ACTUAL Liberty JUCE audio engine, no sound device.
// Usage: LibertyStressV044 <banks-root> <poly64|switch|slice> <duration-s> <report.json>
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/resource.h>
#include <thread>
#include <utility>
#include <vector>
using Clock = std::chrono::steady_clock;
static double secs(Clock::duration x) { return std::chrono::duration<double>(x).count(); }
static double usedCpu(const rusage& r) {
    return r.ru_utime.tv_sec+r.ru_utime.tv_usec*1.e-6
         + r.ru_stime.tv_sec+r.ru_stime.tv_usec*1.e-6;
}
static double peakRssMb(const rusage& r) {
#ifdef __APPLE__
    return double(r.ru_maxrss)/1.e6; // bytes on macOS
#else
    return double(r.ru_maxrss)/1.e3; // kB on Linux
#endif
}
static void insist(bool ok, const juce::String& msg) {
    if (!ok) throw std::runtime_error(msg.toStdString());
}
int main(int argc, char** argv) {
    if(argc!=5) {
        std::cerr<<"Usage: LibertyStressV044 <bank-root> <poly64|switch|slice> <seconds> <output.json>\n";
        return 2;
    }
    try {
        const juce::String mode(argv[2]);
        insist(mode=="poly64" || mode=="switch" || mode=="slice", "Scenario invalide");
        const double duration=std::stod(argv[3]), rate=44100.;
        insist(std::isfinite(duration) && duration>=1. && duration<=180., "Duree invalide");
        const auto banks=juce::File(juce::String::fromUTF8(argv[1]));
        insist(banks.isDirectory(), "Dossier de banques absent");
        ::setenv("LIBERTY_ROMPLER_BANK_DIR", banks.getFullPathName().toRawUTF8(), 1);
        juce::ScopedJuceInitialiser_GUI juceGui;
        LibertyRomplerAudioProcessor processor;
        constexpr int frames=512;
        processor.prepareToPlay(rate,frames);
        auto result=processor.selectPreset("liberty-stress-v044","stress-4-layers");
        insist(result.wasOk(), "Impossible de charger 4 couches: "+result.getErrorMessage());
        const auto before=processor.getStreamingDiagnostics();
        insist(before.streamedZones==64,
               "Attendu: 64 zones longues, obtenu "+juce::String(int(before.streamedZones)));
        if(mode=="slice") processor.getSliceActive().setValueNotifyingHost(1.0f);
        juce::AudioBuffer<float> audio(2,frames);
        juce::MidiBuffer midi;
        const auto start=Clock::now();
        auto deadline=start;
        rusage rStart{},rEnd{};
        getrusage(RUSAGE_SELF,&rStart);
        const int blocks=int(std::ceil(duration*rate/frames));
        std::uint64_t overBudget=0, nonFinite=0, silentSustain=0;
        std::uint64_t lastMeasuredUnderflow=0;
        std::vector<std::pair<double,std::uint64_t>> underflowTimeline;
        underflowTimeline.reserve(32);
        double callbackSeconds=0.,maxCallback=0.,peakSignal=0.;
        for(int block=0;block<blocks;++block) {
            midi.clear();
            const double nominal=block*frames/rate;
            if(mode=="poly64") {
                if(block==0)
                    for(int note=48;note<=63;++note)
                        midi.addEvent(juce::MidiMessage::noteOn(1,note,(juce::uint8)105),0);
            } else if(mode=="switch") {
                const int period=int(rate/frames);
                if(block%period==0) {
                    const bool four=((block/period)%2)==0;
                    result=processor.selectPreset("liberty-stress-v044",
                             four?"stress-4-layers":"stress-layer-1");
                    insist(result.wasOk(),"Changement preset impossible: "+result.getErrorMessage());
                    for(int note=48;note<=63;++note)
                        midi.addEvent(juce::MidiMessage::noteOn(1,note,(juce::uint8)105),0);
                }
            } else {
                const int period=int(rate/frames/2);
                if(block%period==0) {
                    const int slice=(block/period)%8;
                    midi.addEvent(juce::MidiMessage::noteOn(1,36+slice,(juce::uint8)105),0);
                }
            }
            const auto t0=Clock::now();
            processor.processBlock(audio,midi);
            const double t=secs(Clock::now()-t0);
            callbackSeconds+=t;
            maxCallback=std::max(maxCallback,t);
            if(t>frames/rate) ++overBudget;
            double peak=0.;
            for(int channel=0;channel<2;++channel) {
                const auto* samples=audio.getReadPointer(channel);
                for(int i=0;i<frames;++i) {
                    if(!std::isfinite(samples[i])) ++nonFinite;
                    else peak=std::max(peak,std::abs(double(samples[i])));
                }
            }
            peakSignal=std::max(peakSignal,peak);
            // Read counters outside processBlock every ~186 ms. Keep only changes.
            if(block%16==0 || block==blocks-1) {
                const auto snapshot=processor.getStreamingDiagnostics();
                if(snapshot.totals.underflowBlocks>lastMeasuredUnderflow) {
                    lastMeasuredUnderflow=snapshot.totals.underflowBlocks;
                    underflowTimeline.emplace_back(nominal,lastMeasuredUnderflow);
                }
            }
            if(mode=="poly64" && nominal>.5 && nominal<duration-.25 && peak<1.e-7)
                ++silentSustain;
            // Pace against real wall time so background disk workers can operate.
            deadline+=std::chrono::duration_cast<Clock::duration>(
                std::chrono::duration<double>(frames/rate));
            std::this_thread::sleep_until(deadline);
        }
        getrusage(RUSAGE_SELF,&rEnd);
        const double wall=secs(Clock::now()-start);
        const double cpu=std::max(0.,usedCpu(rEnd)-usedCpu(rStart));
        const auto health=processor.getStreamingDiagnostics();
        const auto& c=health.totals;
        std::ostringstream timeline;
        timeline << "[";
        for(std::size_t i=0;i<underflowTimeline.size();++i) {
            if(i) timeline << ",";
            timeline << "{\"at_sec\":" << std::fixed << std::setprecision(3)
                     << underflowTimeline[i].first << ",\"cumulative\":"
                     << underflowTimeline[i].second << "}";
        }
        timeline << "]";
        std::ostringstream j;
        j<<std::fixed<<std::setprecision(4)
         <<"{\n"
         <<"\"version\":\"0.44\",\n"
         <<"\"scenario\":\""<<mode.toStdString()<<"\",\n"
         <<"\"rate_hz\":"<<rate<<",\n"
         <<"\"block_frames\":"<<frames<<",\n"
         <<"\"zones_at_start\":"<<before.streamedZones<<",\n"
         <<"\"blocks\":"<<blocks<<",\n"
         <<"\"elapsed_wall_sec\":"<<wall<<",\n"
         <<"\"process_cpu_pct\":"<<(100.*cpu/wall)<<",\n"
         <<"\"callback_budget_avg_pct\":"<<(100.*callbackSeconds/(blocks*frames/rate))<<",\n"
         <<"\"callback_peak_ms\":"<<(1000.*maxCallback)<<",\n"
         <<"\"callback_over_budget\":"<<overBudget<<",\n"
         <<"\"peak_rss_mb\":"<<peakRssMb(rEnd)<<",\n"
         <<"\"signal_peak\":"<<peakSignal<<",\n"
         <<"\"nonfinite_samples\":"<<nonFinite<<",\n"
         <<"\"suspicious_silent_sustain_blocks\":"<<silentSustain<<",\n"
         <<"\"streamed_zones_end\":"<<health.streamedZones<<",\n"
         <<"\"cache_misses\":"<<c.cacheMisses<<",\n"
         <<"\"dropped_pages\":"<<c.droppedPages<<",\n"
         <<"\"urgent_dropped\":"<<c.urgentDropped<<",\n"
         <<"\"decoder_errors\":"<<c.decoderErrors<<",\n"
         <<"\"underflow_blocks\":"<<c.underflowBlocks<<",\n"
         <<"\"underflow_timeline\":"<<timeline.str()<<"\n}\n";
        const auto path=juce::File(juce::String::fromUTF8(argv[4]));
        insist(path.replaceWithText(juce::String::fromUTF8(j.str().c_str())),
               "Impossible de sauvegarder le rapport JSON");
        std::cout<<j.str();
        processor.releaseResources();
        return nonFinite==0 ? 0 : 3;
    } catch(const std::exception& e) {
        std::cerr<<"LIBERTY V044 FAILURE: "<<e.what()<<"\n";
        return 4;
    }
}
