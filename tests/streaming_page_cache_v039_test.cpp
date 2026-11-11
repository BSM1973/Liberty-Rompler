#include "../Source/StreamingPageCacheV039.h"
#include <cassert>
#include <array>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

using Cache = liberty::stream_v039::PageCache<64, 8, 64>;

int main() {
    Cache c;
    float left=10, right=10;
    assert(!c.tryReadFrame(0, left, right) && left==0 && right==0);
    assert(!c.tryReadFrame(-1, left, right));
    assert(!c.requestPage(-1));
    assert(c.requestPage(4));
    std::int64_t requested=-1;
    assert(c.pollRequest(requested) && requested==4);
    assert(!c.pollRequest(requested));
    std::array<float,64> l{},r{};
    for(int i=0;i<64;++i) {l[static_cast<std::size_t>(i)]=i+2000.0f; r[static_cast<std::size_t>(i)]=i+1000.0f;}
    assert(c.publishPage(4,l.data(),r.data(),64));
    assert(c.tryReadFrame(4*64+17,left,right) && left==2017 && right==1017);
    assert(!c.tryReadFrame(4*64-1,left,right));
    assert(c.requestPage(4)); // already cached, no new request
    assert(!c.pollRequest(requested));
    assert(!c.publishPage(-1,l.data(),r.data(),64));
    assert(!c.publishPage(9,l.data(),r.data(),65));
    assert(!c.publishPage(9,nullptr,r.data(),64));
    assert(c.publishPage(10,l.data(),r.data(),32));
    assert(!c.tryReadFrame(10*64+32,left,right));
    assert(c.tryReadFrame(10*64+31,left,right));

    // 64-bit offsets: over two hours at 48 kHz (no 32-bit wrap).
    constexpr std::int64_t highPage = 4'900'000'000LL / 64;
    assert(c.publishPage(highPage,l.data(),r.data(),64));
    assert(c.tryReadFrame(highPage*64+63,left,right));
    assert(left == 2063 && right == 1063);

    Cache stress;
    std::atomic<bool> running {true};
    std::atomic<unsigned> mismatch {0};
    std::atomic<std::int64_t> newestPage {0};
    std::thread worker([&] {
       std::array<float,64> x{},y{};
       for(std::int64_t page=0;page<12000;++page) {
           for(int i=0;i<64;++i){
               x[static_cast<std::size_t>(i)] = static_cast<float>(page%1000)+i*0.001f;
               y[static_cast<std::size_t>(i)] = -x[static_cast<std::size_t>(i)];
           }
           stress.publishPage(page,x.data(),y.data(),64);
           newestPage.store(page,std::memory_order_release);
           if (page % 64 == 0) std::this_thread::yield();
           std::int64_t ignored=-1;
           (void)stress.pollRequest(ignored);
       }
       running.store(false,std::memory_order_release);
    });
    std::thread audio([&] {
        float x=0,y=0;
        while(running.load(std::memory_order_acquire)) {
            const auto frame=newestPage.load(std::memory_order_acquire)*64+17;
            if(stress.tryReadFrame(frame,x,y)) {
                if(x != -y) mismatch.fetch_add(1);
            } else stress.requestPage(frame/64);
        }
    });
    worker.join(); audio.join();
    assert(mismatch.load()==0); // no torn stereo frames across evictions
    const auto stats=stress.stats();
    assert(stats.published==12000);
    assert(stats.hit>0); // the audio thread actually read concurrently
    std::cout << "v0.39 streaming page cache: preallocated, nonblocking read, 64-bit frames, MPSC mailbox, race-safe stereo OK\n";
}
