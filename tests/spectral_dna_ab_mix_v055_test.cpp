#include "../Source/SpectralDNAMixV055.h"
#include <cassert>
#include <cmath>
#include <limits>
using liberty::spectral_dna_v055::spectralMixForLayer;
int main() {
    for(float legacy : {0.f,0.2f,0.7f,1.f}) {
        assert(spectralMixForLayer(legacy,0.75f,1.f,false)==legacy);
        assert(spectralMixForLayer(legacy,1.f,0.f,true)==legacy);
    }
    assert(spectralMixForLayer(0.f,0.25f,1.f,true)==1.f);
    assert(spectralMixForLayer(0.f,0.0f,1.f,true)==0.f);
    assert(spectralMixForLayer(0.25f,0.25f,0.5f,true)>0.25f);
    assert(spectralMixForLayer(0.1f,1.f,1.f,true)==1.f);
    assert(spectralMixForLayer(0.1f,-1.f,1.f,true)==0.1f);
    assert(std::isfinite(spectralMixForLayer(
        std::numeric_limits<float>::quiet_NaN(),0.5f,1.f,true)));
}
