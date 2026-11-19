#include "../Source/SlicePrefetchCadenceV053.h"
#include <cassert>
#include <iostream>
using liberty::slice_v053::PrefetchCadence;
int main() {
    PrefetchCadence p;
    assert(p.shouldRequest(50000,0,512,512,44100));
    for(int i=1;i<=4;++i)
        assert(!p.shouldRequest(50000,i*512,(i+1)*512,512,44100));
    assert(p.shouldRequest(50000,5*512,6*512,512,44100));
    p.reset();
    assert(p.shouldRequest(40000,8000,8512,512,44100));
    assert(p.shouldRequest(40000,30000,30512,512,44100));
    assert(p.shouldRequest(40000,39000,39400,512,44100));
    assert(p.shouldRequest(40000,40000,40512,512,44100));
    assert(p.shouldRequest(0,100,612,512,44100));
    assert(p.shouldRequest(40000,100,612,512,0));
    p.reset();
    assert(p.shouldRequest(40000,100,612,512,44100));
    assert(p.shouldRequest(80000,20000,21024,512,96000));
    assert(p.shouldRequest(80000,40000,42048,512,96000));
    std::cout << "SLICE prefetch cadence v0.53/v0.55 PASS\n";
}
