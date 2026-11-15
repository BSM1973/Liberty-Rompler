#include "../Source/SpectralDNAFormantsV055.h"
#include <cassert>
#include <cmath>
#include <limits>
using namespace liberty::spectral_dna_v055;
int main(){
    Spectrum a{},b{};
    a[15]=1.f;b[30]=1.f;
    const auto shifted=shiftEnvelope(b,1.5f);
    assert(std::abs(shifted[45]-1.f)<0.001f);
    assert(shifted[30]<0.001f);
    const auto identity=shiftEnvelope(b,1.f);
    assert(identity==b);
    assert(shiftEnvelope(b,std::numeric_limits<float>::quiet_NaN())==b);
    const auto atA=morphEnvelope(a,b,0.f,1.5f);
    const auto atB=morphEnvelope(a,b,1.f,1.5f);
    assert(atA==a && atB==shifted);
    auto midpoint=morphEnvelope(a,b,0.5f,1.5f);
    assert(std::abs(midpoint[15]-0.5f)<0.001f);
    assert(std::abs(midpoint[45]-0.5f)<0.001f);
    for(float v:midpoint)assert(std::isfinite(v) && v>=0.f && v<=1.f);
}
