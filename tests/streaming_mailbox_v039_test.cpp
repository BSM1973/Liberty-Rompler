#include "../Source/StreamingPageCacheV039.h"
#include <atomic>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <thread>
#include <vector>

using Cache = liberty::stream_v039::PageCache<16,2,8>;
int main() {
    Cache bounded;
    for(std::int64_t i=0;i<8;++i) assert(bounded.requestPage(i));
    assert(!bounded.requestPage(999));
    assert(bounded.stats().droppedRequest==1);
    std::int64_t page=-1;
    int received=0;
    while(bounded.pollRequest(page)) {
        assert(page>=0 && page<8);
        ++received;
    }
    assert(received==8);
    assert(bounded.requestPage(999));
    assert(bounded.pollRequest(page) && page==999);

    Cache concurrent;
    std::atomic<unsigned> accepted{0};
    std::vector<std::thread> producers;
    for(int id=0;id<4;++id) {
        producers.emplace_back([&concurrent,&accepted,id] {
            for(int i=0;i<20000;++i) {
                if(concurrent.requestPage(static_cast<std::int64_t>(id)*50000+i))
                    accepted.fetch_add(1,std::memory_order_relaxed);
            }
        });
    }
    for(auto& producer : producers) producer.join();
    unsigned receivedConcurrent=0;
    while(concurrent.pollRequest(page)) ++receivedConcurrent;
    assert(receivedConcurrent>0);
    const auto stats=concurrent.stats();
    assert(stats.request==accepted.load());
    assert(stats.request>=receivedConcurrent);
    std::cout<<"v0.39 bounded multi-producer cache mailbox: overflow & concurrent request safety OK\n";
}
