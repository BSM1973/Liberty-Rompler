#include "../Source/StreamingReadAheadV039.h"
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>

using Cache = liberty::stream_v039::PageCache<128, 8, 16>;
struct MockDiskReader {
    std::thread::id realTime;
    std::atomic<std::uint64_t> calls {0};
    std::atomic<bool> calledFromAudio {false};
    bool readFrames(std::int64_t first,std::size_t count,float* l,float* r) {
        if (std::this_thread::get_id() == realTime) calledFromAudio.store(true);
        calls.fetch_add(1);
        for(std::size_t n=0;n<count;++n) {
            l[n]=static_cast<float>((first+static_cast<std::int64_t>(n))%10000);
            r[n]=-l[n];
        }
        return true;
    }
};
int main() {
    Cache cache;
    MockDiskReader disk;
    disk.realTime=std::this_thread::get_id();
    // Simulate 3-hour sample at 48 kHz, not a 20-second in-RAM sound.
    constexpr std::int64_t total=48000LL*60*60*3;
    liberty::stream_v039::ReadAheadWorker<MockDiskReader,128,8,16> worker(cache,disk,total);
    worker.start();
    const std::int64_t longPage=(48000LL*60*60*2)/128;
    assert(cache.requestPage(longPage));
    bool found=false; float l=0,r=0;
    for(int wait=0;wait<200;++wait) {
        if(cache.tryReadFrame(longPage*128+13,l,r)) {found=true;break;}
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    assert(found);
    assert(l==static_cast<float>((longPage*128+13)%10000));
    assert(r==-l);
    assert(cache.tryReadFrame((longPage+1)*128+25,l,r)); // ahead
    assert(disk.calls.load()>=2);
    assert(!disk.calledFromAudio.load());
    worker.stop();
    assert(!worker.isRunning());
    assert(worker.readErrors()==0);
    std::cout<<"v0.39 worker: long-offset read-ahead, worker-only I/O, safe shutdown OK\n";
}
