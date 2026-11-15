#include "../Source/SlicePrefetchCadenceV053.h"
#include <cassert>
#include <cmath>
int main() {
    using liberty::slice_v053::PrefetchCadence;
    PrefetchCadence p;
    constexpr double sr=44100.;
    // The prefetch boundary must be entered >= 0.65s early at 1x.
    // A 128-frame callback should request immediately when 0.5s remains.
    assert(p.shouldRequest(200000,177950,178078,128,sr));
    for (int i=0;i<12;++i)
        assert(p.shouldRequest(200000,177950,178078,128,sr));
    // Far from edge, calls should be bounded rather than submitted every block.
    p.reset();
    assert(p.shouldRequest(200000,1000,1128,128,sr));
    assert(!p.shouldRequest(200000,1128,1256,128,sr));
    // About 40ms cadence at 44.1kHz: eventually request on repeated callbacks.
    bool refreshed=false;
    for(int i=0;i<30;++i)
        refreshed |= p.shouldRequest(200000,1000+i*128,1128+i*128,128,sr);
    assert(refreshed);
    assert(p.shouldRequest(0,0,128,128,sr));
    assert(p.shouldRequest(200000,NAN,128,128,sr));
    // A 4x voice must request while farther from the warmup boundary.
    p.reset();
    assert(p.shouldRequest(200000,140000,142048,512,sr));
    return 0;
}
