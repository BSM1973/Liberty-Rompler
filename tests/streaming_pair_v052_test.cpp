#include "../Source/StreamingPageCacheV039.h"
#include <array>
#include <atomic>
#include <cassert>
#include <cmath>
#include <iostream>
#include <thread>
using Cache = liberty::stream_v039::PageCache<64, 4, 8>;
int main() {
    Cache cache;
    std::array<float,64> l{},r{};
    for(int i=0;i<64;++i) { l[i]=float(i)*.25f; r[i]=-float(i)*.25f; }
    assert(cache.publishPage(1,l.data(),r.data(),64));
    float l0=-1.f,r0=-1.f,l1=-1.f,r1=-1.f;
    assert(cache.tryReadFramePair(64+17,l0,r0,l1,r1));
    assert(std::abs(l0-4.25f)<1e-5f && std::abs(l1-4.5f)<1e-5f);
    assert(r0==-l0 && r1==-l1);
    assert(!cache.tryReadFramePair(64+63,l0,r0,l1,r1));
    assert(!cache.tryReadFramePair(-1,l0,r0,l1,r1));
    assert(cache.publishPage(2,l.data(),r.data(),30));
    assert(cache.tryReadFramePair(128+28,l0,r0,l1,r1));
    assert(!cache.tryReadFramePair(128+29,l0,r0,l1,r1));
    assert(cache.tryReadFrame(128+29,l0,r0));
    std::atomic<bool> ready{false};
    std::thread worker([&] {
        ready.store(true);
        for(int p=3;p<120;++p) {
            std::array<float,64> w{};
            w.fill(float(p));
            assert(cache.publishPage(p,w.data(),w.data(),64));
        }
    });
    while(!ready.load()) std::this_thread::yield();
    for(int k=0;k<50000;++k) {
        float a=0,b=0,c=0,d=0;
        if(cache.tryReadFramePair(64+17,a,b,c,d))
            assert(a==4.25f && c==4.5f && b==-a && d==-c);
    }
    worker.join();
    const auto stats=cache.stats();
    assert(stats.hit>=2);
    std::cout<<"v0.52 pinned adjacent PCM interpolation PASS\n";
}
