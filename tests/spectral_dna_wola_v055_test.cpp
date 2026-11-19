#include "../Source/SpectralDNAWolaV055.h"
#include <algorithm>
#include <cassert>
#include <cmath>
using namespace liberty::spectral_dna_v055;
int main() {
    constexpr float tau=6.2831853071795864769f;
    std::vector<float> src(4096);
    for(std::size_t i=0;i<src.size();++i)
        src[i]=0.25f*std::sin(tau*8.f*float(i)/256.f)
              +0.25f*std::sin(tau*24.f*float(i)/256.f);
    Layers layers{};
    for(auto& p:layers) p.fill(1.f);
    Settings s;
    OrbitControl orbit;
    auto bypass=renderOffline(src,layers,s,orbit,48000.);
    assert(bypass.audio==src && bypass.transformFrames==0);
    s.enabled=true; s.wet=1.f;
    // Spectral gain when all target values=1 is 2.0.
    auto doubled=renderOffline(src,layers,s,orbit,48000.);
    assert(doubled.audio.size()==src.size());
    float error=0.f;
    for(std::size_t i=256;i<src.size()-256;++i) {
        assert(std::isfinite(doubled.audio[i]));
        error=std::max(error,std::abs(doubled.audio[i]-2.f*src[i]));
    }
    assert(error<0.002f);
    assert(doubled.transformFrames>0);
    orbit.mode=1; orbit.rate=0.75f; orbit.depth=1.f;
    layers[0].fill(0.f); layers[1].fill(0.25f);
    layers[2].fill(0.5f); layers[3].fill(1.f);
    auto moving=renderOffline(src,layers,s,orbit,48000.);
    assert(moving.trajectory.size()==moving.transformFrames);
    assert(moving.audio.size()==src.size());
    bool hasMotion=false;
    for(std::size_t i=1;i<moving.trajectory.size();++i) {
        hasMotion |= std::abs(moving.trajectory[i].x-moving.trajectory[0].x)>0.00001f;
        assert(std::isfinite(moving.audio[std::min(i*64,src.size()-1)]));
    }
    assert(hasMotion);
    auto silent=renderOffline({},layers,s,orbit,48000.);
    assert(silent.audio.empty());
    assert(renderOffline(src,layers,s,orbit,0.).audio.size()==src.size());
}
