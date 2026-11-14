#include "../Source/SliceWarmupV049.h"
#include <cassert>
#include <cstdint>
#include <iostream>
int main() {
    using liberty::stream_v049::planSliceWarmup;
    const int frames=44100*24;
    assert(planSliceWarmup(frames,0,44100).frames==0);
    assert(planSliceWarmup(frames,8,44100).frames==0);
    assert(planSliceWarmup(frames,-1,44100).frames==0);
    for(int i=1;i<8;++i){
        const auto w=planSliceWarmup(frames,i,44100);
        const auto region=liberty::creative_v038::slice(frames-1,i);
        assert(w.begin==region.begin);
        assert(w.frames==region.end-region.begin);
        assert(w.begin+w.frames<=region.end);
    }
    const auto high=planSliceWarmup(2'147'483'000,7,192000);
    assert(high.frames==147456);
    assert((std::int64_t)high.begin+high.frames<2'147'483'000);
    assert(planSliceWarmup(frames,7,0.).frames==0);
    std::cout<<"v0.49 seven bounded SLICE warmups: ranges, interpolation, 32bit safe OK\n";
}
