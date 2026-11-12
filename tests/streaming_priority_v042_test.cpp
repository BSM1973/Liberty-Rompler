#include "../Source/StreamingPoolV041.h"
#include "../Source/StreamingHealthV042.h"
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

struct Busy final : liberty::stream_v041::ZoneService {
    std::atomic<int> normal {0}, served {0};
    bool serviceOneRequest() noexcept override {
        int requested = normal.load(std::memory_order_relaxed);
        while (requested > 0) {
            if (normal.compare_exchange_weak(
                    requested, requested - 1, std::memory_order_relaxed)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(3));
                served.fetch_add(1, std::memory_order_relaxed);
                return true;
            }
        }
        return false;
    }
};
struct Critical final : liberty::stream_v041::ZoneService {
    std::atomic<int> urgent {0}, served {0};
    bool serviceUrgentRequest() noexcept override {
        if (urgent.exchange(0, std::memory_order_acq_rel) == 0) return false;
        served.fetch_add(1, std::memory_order_relaxed);
        return true;
    }
    bool serviceOneRequest() noexcept override { return false; }
};

int main() {
    using namespace std::chrono;
    auto& pool = liberty::stream_v041::StreamingPool::instance();
    Busy busy[4];
    for (auto& zone : busy) assert(pool.registerZone(&zone));
    // Since the first four lanes have one zone each, Critical joins lane 0.
    Critical critical;
    assert(pool.registerZone(&critical));
    busy[0].normal.store(400, std::memory_order_relaxed);
    critical.urgent.store(1, std::memory_order_release);
    bool handled = false;
    for (int attempt = 0; attempt < 1000; ++attempt) {
        if (critical.served.load(std::memory_order_acquire) == 1) {
            handled = true;
            break;
        }
        std::this_thread::sleep_for(milliseconds(1));
    }
    assert(handled); // no starvation behind 400 slower speculative reads
    assert(busy[0].normal.load(std::memory_order_relaxed) > 0);
    pool.unregisterZone(&critical);
    for (auto& zone : busy) pool.unregisterZone(&zone);
    assert(pool.registeredZones() == 0);

    liberty::stream_v042::HealthSnapshot clean;
    assert(clean.hitRatePercent() == 100.0);
    assert(!clean.needsAttention());
    clean.cacheHits = 75;
    clean.cacheMisses = 25;
    assert(std::abs(clean.hitRatePercent() - 75.0) < 1.0e-12);
    clean.underflowBlocks = 1;
    assert(clean.needsAttention());
    clean.underflowBlocks = 0;
    clean.urgentDropped = 1;
    assert(clean.needsAttention());
    std::cout << "v0.42 critical first and health counters OK\n";
}
