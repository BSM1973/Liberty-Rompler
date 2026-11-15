// Verify a rapid second publish starts near the *audible blend* of old
// and current frames instead of abruptly starting from the previous target.
// Uses the genuine v0.48 spectral engine, including optional formant tables.
#include "../Source/SpectralEngineV048.h"
#include <array>
#include <cassert>
#include <cmath>
#include <memory>
using namespace liberty::spectral_v048;

static std::unique_ptr<const Frame> makeFrame(int variation) {
    constexpr double sr=48000.,tau=6.2831853071795864769;
    std::array<float,fftSize> src{};
    for(int i=0;i<fftSize;++i) {
        const double t=double(i)/sr;
        src[std::size_t(i)]=float(0.4*std::sin(tau*220.*t)
            +0.22*std::sin(tau*220.*double(variation+2)*t)
            +0.15*std::sin(tau*220.*double(variation+5)*t));
    }
    auto f=analyse(src.data(),fftSize,sr,57);
    assert(f&&f->valid);
    return f;
}
int main() {
    Engine actual,oldReference,middleReference;
    Parameters p;p.enabled=true;p.mix=1.f;p.formantShift=0.f;
    for(auto* e:{&actual,&oldReference,&middleReference})
        e->prepare(48000.);
    actual.publishAnalysed(makeFrame(0));
    oldReference.publishAnalysed(makeFrame(0));
    middleReference.publishAnalysed(makeFrame(1));
    for(auto* e:{&actual,&oldReference,&middleReference})
        e->noteOn(1,57,110,p);
    std::array<float,512> warm{};
    for(auto* e:{&actual,&oldReference,&middleReference}) {
        warm.fill(0.f);
        e->render(warm.data(),nullptr,0,int(warm.size()),p);
    }
    actual.publishAnalysed(makeFrame(1));
    std::array<float,64> segment{};
    for(auto* e:{&actual,&oldReference,&middleReference}) {
        segment.fill(0.f);
        e->render(segment.data(),nullptr,0,int(segment.size()),p);
    }
    // First fade was 64/256 complete at this point.
    actual.publishAnalysed(makeFrame(2));
    float actualNext=0.f,originalNext=0.f,middleNext=0.f;
    actual.render(&actualNext,nullptr,0,1,p);
    oldReference.render(&originalNext,nullptr,0,1,p);
    middleReference.render(&middleNext,nullptr,0,1,p);
    const float expected=.75f*originalNext+.25f*middleNext;
    const float naive=middleNext; // legacy rapid second transition
    assert(std::isfinite(actualNext));
    assert(std::abs(actualNext-expected)<0.006f);
    // Repeated publications without audio rendering are also safe and
    // should retain the last *audible* previous endpoint.
    actual.publishAnalysed(makeFrame(0));
    actual.publishAnalysed(makeFrame(2));
    std::array<float,512> tail{};
    actual.render(tail.data(),nullptr,0,int(tail.size()),p);
    for(float v:tail) assert(std::isfinite(v));
    (void)naive;
}
