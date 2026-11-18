// v0.55 diagnostic-only macOS scheduler reference. No JUCE, DSP or host audio.
// Usage: scheduler-reference-v055 <seconds> <report.json>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#if defined(__APPLE__)
#include <pthread/qos.h>
#endif

namespace {
using Clock=std::chrono::steady_clock;
constexpr double rate=44100., frames=512.;
constexpr double budgetSeconds=frames/rate;
constexpr double workSeconds=.001; // 1ms synthetic CPU: below 11.61ms callback budget.
struct Spike { double atSeconds, wallMs, threadCpuMs; };
double threadCpuSeconds() {
    timespec ts{};
    if(clock_gettime(CLOCK_THREAD_CPUTIME_ID,&ts)!=0)
        throw std::runtime_error("Thread CPU clock unavailable");
    return double(ts.tv_sec)+double(ts.tv_nsec)*1.e-9;
}
} // namespace

int main(int argc,char**argv) {
    try {
        if(argc!=3) throw std::runtime_error("Usage: scheduler-reference-v055 <seconds> <report.json>");
        const double seconds=std::stod(argv[1]);
        if(!std::isfinite(seconds)||seconds<.2||seconds>120.)
            throw std::runtime_error("Invalid reference duration");
        bool qosApplied=false;
#if defined(__APPLE__)
        if(pthread_set_qos_class_self_np(QOS_CLASS_USER_INTERACTIVE,0)!=0)
            throw std::runtime_error("Unable to set interactive QoS");
        qos_class_t actualQos=QOS_CLASS_UNSPECIFIED;
        int relativePriority=0;
        if(pthread_get_qos_class_np(pthread_self(),&actualQos,&relativePriority)!=0 ||
           actualQos!=QOS_CLASS_USER_INTERACTIVE)
            throw std::runtime_error("Interactive QoS not confirmed");
        qosApplied=true;
#endif
        const int blocks=int(std::ceil(seconds/budgetSeconds));
        const auto start=Clock::now();
        auto deadline=start;
        std::uint64_t overBudget=0;
        std::vector<Spike> spikes;
        spikes.reserve(128);
        std::uint64_t scratch=0x12345678;
        double peakWallMs=0.,peakCpuMs=0.;
        for(int block=0;block<blocks;++block) {
            const auto t0=Clock::now();
            const double cpu0=threadCpuSeconds();
            // The work is on this very same thread, using CPU time rather
            // than wall time; preemption cannot make the work complete early.
            do {
                scratch=scratch*1664525u+1013904223u;
            } while(threadCpuSeconds()-cpu0<workSeconds);
            const double cpuMs=(threadCpuSeconds()-cpu0)*1000.;
            const double wallMs=std::chrono::duration<double,std::milli>(Clock::now()-t0).count();
            if(wallMs>peakWallMs) peakWallMs=wallMs;
            if(cpuMs>peakCpuMs) peakCpuMs=cpuMs;
            if(wallMs>budgetSeconds*1000.) {
                ++overBudget;
                if(spikes.size()<128) spikes.push_back({block*budgetSeconds,wallMs,cpuMs});
            }
            deadline+=std::chrono::duration_cast<Clock::duration>(
                std::chrono::duration<double>(budgetSeconds));
            std::this_thread::sleep_until(deadline);
        }
        const double wall=std::chrono::duration<double>(Clock::now()-start).count();
        std::ofstream file(argv[2]);
        if(!file) throw std::runtime_error("Cannot open reference JSON");
        file<<std::fixed<<std::setprecision(4)
            <<"{\n\"scenario\":\"scheduler_reference\",\n"
            <<"\"rate_hz\":"<<rate<<",\n"
            <<"\"block_frames\":"<<frames<<",\n"
            <<"\"elapsed_wall_sec\":"<<wall<<",\n"
            <<"\"qos_interactive_confirmed\":"<<(qosApplied?"true":"false")<<",\n"
            <<"\"synthetic_cpu_work_target_ms\":"<<workSeconds*1000.<<",\n"
            <<"\"callback_peak_ms\":"<<peakWallMs<<",\n"
            <<"\"callback_thread_cpu_peak_ms\":"<<peakCpuMs<<",\n"
            <<"\"callback_over_budget\":"<<overBudget<<",\n"
            <<"\"spikes_truncated\":"<<(overBudget>spikes.size()?"true":"false")<<",\n"
            <<"\"callback_spikes\":[";
        for(std::size_t i=0;i<spikes.size();++i) {
            if(i) file<<",";
            file<<"{\"at_sec\":"<<spikes[i].atSeconds
                <<",\"callback_ms\":"<<spikes[i].wallMs
                <<",\"thread_cpu_ms\":"<<spikes[i].threadCpuMs<<"}";
        }
        file<<"],\n\"scratch_checksum\":"<<scratch<<"\n}\n";
        file.close();
        if(!file) throw std::runtime_error("Unable to flush reference JSON");
        std::cout<<"SCHEDULER_REFERENCE: "<<overBudget<<" over-budget blocks / "<<blocks
                 <<"; interactive QoS confirmed="<<qosApplied<<"\n";
        return 0; // Diagnostic must never turn a strict DSP failure into PASS.
    } catch(const std::exception& e) {
        std::cerr<<"SCHEDULER_REFERENCE_ERROR: "<<e.what()<<"\n";
        return 2;
    }
}
