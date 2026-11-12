#include "../Source/StreamingPoolV041.h"
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

struct Zone final : liberty::stream_v041::ZoneService {
    std::atomic<int> pending{0}, served{0};
    bool serviceOneRequest() noexcept override {
        int expected = pending.load(std::memory_order_relaxed);
        while (expected > 0) {
            if (pending.compare_exchange_weak(expected, expected - 1)) {
                served.fetch_add(1);
                return true;
            }
        }
        return false;
    }
};

int main() {
    auto& pool = liberty::stream_v041::StreamingPool::instance();
    std::array<Zone, 65> zones {};
    for (int i = 0; i < 64; ++i) {
        assert(pool.registerZone(&zones[(std::size_t)i]));
        zones[(std::size_t)i].pending.store(2);
    }
    assert(pool.registeredZones() == 64);
    assert(!pool.registerZone(&zones[64]));
    assert(pool.registerZone(&zones[0])); // idempotent duplicate registration
    assert(pool.registeredZones() == 64);
    bool done = false;
    for (int attempt = 0; attempt != 2000; ++attempt) {
        done = true;
        for (int i = 0; i < 64; ++i)
            done = done && zones[(std::size_t)i].served.load() == 2;
        if (done) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    assert(done);
    for (int i = 0; i < 64; ++i) pool.unregisterZone(&zones[(std::size_t)i]);
    assert(pool.registeredZones() == 0);
    assert(pool.registerZone(&zones[64]));
    zones[64].pending.store(1);
    for (int attempt = 0; attempt != 200 && zones[64].served.load() != 1; ++attempt)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    assert(zones[64].served.load() == 1);
    pool.unregisterZone(&zones[64]);
    zones[64].pending.store(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    assert(zones[64].served.load() == 1); // unregister waits for in-flight work
    std::cout << "v0.41 four-worker shared streaming pool 64 zones OK\n";
}
