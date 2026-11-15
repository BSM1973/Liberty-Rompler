#include "../Source/SpectralDNAPresetFadeV055.h"
#include <cassert>
#include <cmath>
using liberty::spectral_dna_v055::PresetFade;
int main() {
    PresetFade fade;
    assert(!fade.active());
    assert(fade.advance()==1.f);
    fade.start();
    float previous=0.f;
    for(int i=0;i<PresetFade::kSamples;++i) {
        const float w=fade.advance();
        assert(w>previous && w<=1.f);
        assert(w-previous<=1.f/float(PresetFade::kSamples)+1.e-6f);
        assert(std::isfinite(fade.mix(-0.2f,0.4f,w)));
        previous=w;
    }
    assert(!fade.active() && previous==1.f);
    assert(fade.advance()==1.f);
    fade.start();
    for(int i=0;i<20;++i)fade.advance();
    fade.start();
    assert(fade.advance()==1.f/float(PresetFade::kSamples));
    fade.reset();
    assert(!fade.active());
}
