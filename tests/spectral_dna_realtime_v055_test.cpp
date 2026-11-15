#include "../Source/SpectralDNARealtimeV055.h"
#include <array>
#include <cassert>
#include <cmath>
using liberty::spectral_dna_v055::RealtimeBlend;
int main(){
    RealtimeBlend engine;
    engine.prepare(256);
    assert(engine.getLatencySamples()==256);
    for(int i=0;i<1024;++i) {
        const float x=float(i)*0.001f;
        assert(engine.process(x,10.f,true,1.f,false)==x);
    }
    engine.reset();
    for(int i=0;i<512;++i) {
        const float dry=i==0?1.f:0.f;
        const float actual=engine.process(dry,0.f,false,1.f,true);
        assert(std::isfinite(actual));
        if(i==256) assert(actual==1.f);
    }
    assert(engine.diagnostics().wetMissing==512);
    engine.reset();
    float previous=0.f;
    for(int i=0;i<256;++i){
        float y=engine.process(0.f,1.f,true,1.f,true);
        assert(y>=previous && y<=1.f);
        assert(y-previous<=1.f/128.f+1.e-6f);
        previous=y;
    }
    assert(previous>0.99f);
    // Missing wet initiates bounded fade from last good sample, not a click.
    float firstMissing=engine.process(0.f,99.f,false,1.f,true);
    assert(firstMissing>0.f && firstMissing<1.f);
    float previousMissing=firstMissing;
    for(int i=0;i<127;++i) {
        float y=engine.process(0.f,99.f,false,1.f,true);
        assert(y<=previousMissing+1.e-6f);
        previousMissing=y;
    }
    assert(previousMissing<0.001f);
    assert(engine.diagnostics().wetMissing==128);
    // Saturation and non-finite guard.
    engine.prepare(999999);
    assert(engine.getLatencySamples()==int(RealtimeBlend::kCapacity-1));
    assert(std::isfinite(engine.process(0.f,0.f,true,1.f,true)));
    // Simulate 64 independent voices (no shared mutable state).
    std::array<RealtimeBlend,64> voices{};
    for(auto& voice:voices) voice.prepare(256);
    float sum=0.f;
    for(int block=0;block<100;++block)
      for(int frame=0;frame<128;++frame)
        for(auto& voice:voices)
          sum+=voice.process(0.005f,0.007f,true,0.5f,true);
    assert(std::isfinite(sum));
}
