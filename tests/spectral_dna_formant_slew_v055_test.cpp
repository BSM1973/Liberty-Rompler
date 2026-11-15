#include "../Source/SpectralDNAFormantSlewV055.h"
#include <cassert>
#include <cmath>
#include <limits>
using liberty::spectral_dna_v055::FormantSlew;
int main() {
    FormantSlew slew;
    assert(slew.value()==0.f);
    float previous=0.f;
    for(int i=0;i<128;++i) {
        const float v=slew.next(1.f);
        assert(v>=previous && v-previous<=2.f/256.f+0.000001f);
        previous=v;
    }
    assert(std::abs(slew.value()-1.f)<0.00001f);
    for(int i=0;i<256;++i) {
        const float v=slew.next(-1.f);
        assert(v<=previous && previous-v<=2.f/256.f+0.000001f);
        previous=v;
    }
    assert(std::abs(previous+1.f)<0.00001f);
    for(int i=0;i<128;++i) slew.next(0.f);
    assert(slew.value()==0.f);
    slew.reset();
    assert(slew.value()==0.f);
    assert(std::isfinite(slew.next(std::numeric_limits<float>::quiet_NaN())));
    assert(slew.value()==0.f);
}
