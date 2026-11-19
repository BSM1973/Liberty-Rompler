#include "../Source/SpectralDNASessionV055.h"
#include <cassert>
using namespace liberty::spectral_dna_v055;
struct FakeProcessor {
    int latency=-1;
    void setLatencySamples(int value) { latency=value; }
};
int main(){
    FakeProcessor host;
    SessionControl session;
    assert(session.activeGeneration()==1);
    session.configureHostLatency(host);
    assert(host.latency==256);
    const auto next=session.beginPresetChange();
    assert(next==2 && session.activeGeneration()==2);
    assert(session.beginPresetChange()==3);
}
