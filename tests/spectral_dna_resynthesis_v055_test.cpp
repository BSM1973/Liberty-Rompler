#include "../Source/SpectralDNAResynthesisV055.h"
#include <array>
#include <cassert>
#include <cmath>
#include <limits>
using namespace liberty::spectral_dna_v055;
int main() {
    constexpr float tau=6.2831853071795864769f;
    AudioWindow source{};
    for(std::size_t i=0;i<kFftSize;++i)
        source[i]=0.5f*std::sin(tau*8.f*float(i)/float(kFftSize))
                 +0.5f*std::sin(tau*24.f*float(i)/float(kFftSize));
    Spectrum target{};
    target[8]=1.f; // accentuate harmonic 8 while attenuating harmonic 24
    auto bypass=renderWindow(source,target,0.f);
    for(std::size_t i=0;i<kFftSize;++i) assert(bypass[i]==source[i]);
    auto result=renderWindow(source,target,1.f);
    float delta=0.f;
    for(std::size_t i=0;i<kFftSize;++i) {
        assert(std::isfinite(result[i]));
        delta+=std::abs(result[i]-source[i]);
    }
    assert(delta>5.f); // waveform materially differs (audible offline transformation)
    auto wet=renderWindow(source,target,0.5f);
    float midpointDistance=0.f;
    for(std::size_t i=0;i<kFftSize;++i)
        midpointDistance+=std::abs(wet[i]-result[i]);
    assert(midpointDistance>1.f);
    assert(std::abs(spectralGain(1.f,0.f)-1.f)<0.0001f);
    assert(spectralGain(0.f,1.f)<spectralGain(1.f,1.f));
    assert(spectralGain(std::numeric_limits<float>::quiet_NaN(),1.f)>0.f);
    FFTBuffer a{};
    a[8]=Complex(2.f,1.f);a[kFftSize-8]=std::conj(a[8]);
    applyTimbreEnvelope(a,target,0.f);
    assert(a[8]==Complex(2.f,1.f));
    applyTimbreEnvelope(a,target,1.f);
    assert(std::abs(a[8]-Complex(4.f,2.f))<0.001f);
}
