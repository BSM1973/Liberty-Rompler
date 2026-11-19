#include "../Source/SpectralDNAV055.h"
#include <cassert>
#include <cmath>
#include <limits>
using namespace liberty::spectral_dna_v055;
static bool near(float a,float b) { return std::abs(a-b)<0.0001f; }
int main() {
    Layers p{};
    for (std::size_t i=0;i<4;++i) p[i][i] = 1.f;
    Settings s;
    assert(near(calculate(p,s,0.f).wet,0.f));
    s.enabled=true; s.wet=1.f;
    s.x=0.f; s.y=0.f;
    auto a=calculate(p,s,0.f);
    assert(near(a.target[0],1.f) && near(a.target[1],0.f));
    s.x=1.f; s.y=1.f;
    assert(near(calculate(p,s,0.f).target[3],1.f));
    s.x=0.5f; s.y=0.5f;
    auto centre=calculate(p,s,0.f);
    for (int i=0;i<4;++i) assert(near(centre.target[i],0.25f));
    assert(near(calculate(p,s,1.f).wet,0.f));
    s.transientProtection=0.f;
    assert(near(calculate(p,s,1.f).wet,1.f));
    Spectrum dirty{};
    dirty[0]=2.f; dirty[1]=-1.f;
    dirty[2]=std::numeric_limits<float>::quiet_NaN();
    auto clean=normalizeProfile(dirty);
    assert(near(clean[0],1.f) && near(clean[1],0.f) && near(clean[2],0.f));
    s.x=std::numeric_limits<float>::quiet_NaN(); s.y=-5.f;
    assert(near(calculate(p,s,0.f).target[0],1.f));
    auto empty=interpolateProfiles(p,Weights{{0.f,0.f,0.f,0.f}});
    assert(near(empty[0],0.f));
}
