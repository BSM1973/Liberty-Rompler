// Synthetic 2ms slow I/O mock: proves urgent scheduling under 64-zone load.
#include "../Source/StreamingPoolV041.h"
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>
struct Zone final : liberty::stream_v041::ZoneService {
    std::atomic<int> urgent {0}, ordinary {0}, served {0};
    bool serviceUrgentRequest() noexcept override {
        if (urgent.exchange(0,std::memory_order_acq_rel)==0) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        served.fetch_add(1,std::memory_order_relaxed);
        return true;
    }
    bool serviceOneRequest() noexcept override {
        int count=ordinary.load(std::memory_order_relaxed);
        while(count>0) {
            if(ordinary.compare_exchange_weak(count,count-1,std::memory_order_relaxed)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                return true;
            }
        }
        return false;
    }
};
int main() {
    using namespace std::chrono;
    auto& pool=liberty::stream_v041::StreamingPool::instance();
    std::array<Zone,64> zones;
    for(auto& z:zones) assert(pool.registerZone(&z));
    assert(pool.registeredZones()==64);
    Zone extra;
    assert(!pool.registerZone(&extra));
    for(auto& z:zones) { z.ordinary.store(400); z.urgent.store(1); }
    auto start=steady_clock::now();
    bool ready=false;
    for(int i=0;i<400;++i) {
        ready=true;
        for(const auto& z:zones) if(z.served.load(std::memory_order_acquire)==0) ready=false;
        if(ready) break;
        std::this_thread::sleep_for(milliseconds(2));
    }
    assert(ready);
    assert(steady_clock::now()-start<seconds(2));
    for(auto& z:zones) assert(z.ordinary.load(std::memory_order_relaxed)>0);
    // Stop synthetic slow work before draining registrations; otherwise a
    // continuous backlog can starve the administrative unregister lock.
    for(auto& z:zones) z.ordinary.store(0,std::memory_order_relaxed);
    for(auto& z:zones) pool.unregisterZone(&z);
    assert(pool.registeredZones()==0);
    std::cout<<"v0.44 64-zone artificial 2ms slow worker priority OK\n";
}
