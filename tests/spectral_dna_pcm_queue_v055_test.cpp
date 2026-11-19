#include "../Source/SpectralDNAPcmQueueV055.h"
#include <cassert>
#include <cmath>
#include <cstdint>
using namespace liberty::spectral_dna_v055;
int main() {
    SpectralPcmQueue queue;
    SpectralPcmQueue::Frame frame{};
    for(std::uint64_t k=0;k<32;++k) {
        frame.firstSample=k*64;
        frame.generation=1;
        frame.mono.fill(float(k+1));
        assert(queue.push(frame));
    }
    assert(!queue.push(frame));
    assert(queue.availableApprox()==32);
    SpectralPcmCursor cursor;
    float wet=0.f;
    for(std::uint64_t i=0;i<32*64;++i) {
        assert(cursor.next(queue,1,i,wet));
        assert(wet==float(i/64+1));
    }
    assert(queue.availableApprox()==0);
    assert(!cursor.next(queue,1,2048,wet));
    frame.firstSample=2048;
    frame.generation=1;
    frame.mono.fill(1.f);
    assert(queue.push(frame));
    assert(!cursor.next(queue,2,2048,wet)); // preset switched
    frame.firstSample=2112;frame.generation=2;
    assert(queue.push(frame));
    assert(cursor.next(queue,2,2112,wet));
    for(int i=1;i<64;++i) assert(cursor.next(queue,2,2112+std::uint64_t(i),wet));
    frame.firstSample=2176;frame.mono.fill(0.f);frame.mono[0]=NAN;
    assert(queue.push(frame));
    assert(!cursor.next(queue,2,2176,wet));
    frame.firstSample=2240;frame.mono.fill(0.25f);
    assert(queue.push(frame));
    assert(cursor.next(queue,2,2240,wet) && wet==0.25f);
    queue.resetBeforeStart();
    cursor.resetBeforeStart();
    assert(queue.availableApprox()==0);
}
