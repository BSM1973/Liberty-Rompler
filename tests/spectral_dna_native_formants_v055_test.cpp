// Actual v0.48 spectral engine + v0.55 offline-prepared formant variants.
// Numeric A/B validation; not a subjective DAW audition.
#include "../Source/SpectralEngineV048.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <memory>
#include <vector>
using namespace liberty::spectral_v048;
int main() {
    constexpr double sr=48000.,twoPi=6.2831853071795864769;
    std::array<float,fftSize> source{};
    for(int i=0;i<fftSize;++i) {
        const double t=double(i)/sr;
        source[std::size_t(i)]=float(
            0.5*std::sin(twoPi*220.*t)+
            0.25*std::sin(twoPi*660.*t)+
            0.1*std::sin(twoPi*1540.*t));
    }
    auto frame=analyse(source.data(),fftSize,sr,57);
    assert(frame&&frame->valid);
    // Prepared table variations must be finite, bounded, and genuinely different.
    double tableDifference=0.;
    for(int band=0;band<bands;++band)
        for(int i=0;i<tableSize;++i) {
            const auto idx=std::size_t(i);
            assert(std::isfinite(frame->formantDown[std::size_t(band)][idx]));
            assert(std::isfinite(frame->formantUp[std::size_t(band)][idx]));
            tableDifference+=std::abs(double(frame->tables[std::size_t(band)][idx]-
                               frame->formantUp[std::size_t(band)][idx]));
        }
    assert(tableDifference>1.);
    auto render=[&](float formant) {
        Engine e;
        e.prepare(sr);
        auto prepared=analyse(source.data(),fftSize,sr,57);
        e.publishAnalysed(std::move(prepared));
        Parameters p;
        p.enabled=true;p.mix=1.f;p.gain=1.f;p.formantShift=formant;
        e.noteOn(1,57,120,p);
        std::vector<float> buffer(4096,0.f);
        e.render(buffer.data(),nullptr,0,int(buffer.size()),p);
        for(float v:buffer)assert(std::isfinite(v));
        return buffer;
    };
    const auto normal=render(0.f);
    const auto down=render(-1.f);
    const auto up=render(1.f);
    const auto identity=render(0.f);
    assert(normal==identity);
    double deltaDown=0.,deltaUp=0.,power=0.;
    for(std::size_t i=0;i<normal.size();++i){
        deltaDown+=std::abs(double(normal[i]-down[i]));
        deltaUp+=std::abs(double(normal[i]-up[i]));
        power+=std::abs(double(normal[i]));
    }
    assert(power>0.1 && deltaDown>0.1 && deltaUp>0.1);
    return 0;
}
