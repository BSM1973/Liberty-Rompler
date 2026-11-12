#pragma once
// Liberty ROMpler v0.41: four background streaming lanes shared by all zones.
// Registration and destruction happen OFF audio. The callback only uses
// the existing v0.39 lock-free page cache. No disk operation under audio.
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <mutex>
#include <thread>

namespace liberty::stream_v041 {
class ZoneService {
public:
    virtual bool serviceOneRequest() noexcept = 0;
protected:
    virtual ~ZoneService() = default;
};

// Shared across all loaded zones in the plugin module; four disk workers
// instead of one thread per zone. Each zone belongs to one worker only.
class StreamingPool final {
public:
    static constexpr std::size_t lanesCount = 4;
    static constexpr std::size_t zonesPerLane = 16;
    static constexpr std::size_t maxZones = lanesCount * zonesPerLane;

    static StreamingPool& instance() {
        static StreamingPool value;
        return value;
    }
    StreamingPool(const StreamingPool&) = delete;
    StreamingPool& operator=(const StreamingPool&) = delete;

    // Message/loading thread only. Safe: lane lock guards each pointer's
    // membership. A zone may be destroyed after unregisterZone returns.
    bool registerZone(ZoneService* zone) {
        if (zone == nullptr) return false;
        const std::lock_guard<std::mutex> regLock(registrationMutex);
        std::size_t best = lanesCount, load = zonesPerLane + 1;
        for (std::size_t i = 0; i < lanesCount; ++i) {
            auto& lane = lanes[i];
            const std::lock_guard<std::mutex> guard(lane.mutex);
            for (std::size_t j = 0; j < lane.count; ++j)
                if (lane.zones[j] == zone) return true;
            if (lane.count < load && lane.count < zonesPerLane) {
                load = lane.count;
                best = i;
            }
        }
        if (best == lanesCount) return false;
        auto& lane = lanes[best];
        const std::lock_guard<std::mutex> guard(lane.mutex);
        if (lane.count >= zonesPerLane) return false;
        lane.zones[lane.count++] = zone;
        return true;
    }

    // Message/loading thread only. Blocks until an in-progress disk read
    // finishes on the corresponding worker, preventing use-after-free.
    void unregisterZone(ZoneService* zone) {
        if (zone == nullptr) return;
        const std::lock_guard<std::mutex> regLock(registrationMutex);
        for (auto& lane : lanes) {
            const std::lock_guard<std::mutex> guard(lane.mutex);
            for (std::size_t j = 0; j < lane.count; ++j) {
                if (lane.zones[j] != zone) continue;
                for (std::size_t k = j + 1; k < lane.count; ++k)
                    lane.zones[k - 1] = lane.zones[k];
                lane.zones[--lane.count] = nullptr;
                return;
            }
        }
    }

    std::size_t registeredZones() {
        std::size_t count = 0;
        for (auto& lane : lanes) {
            const std::lock_guard<std::mutex> guard(lane.mutex);
            count += lane.count;
        }
        return count;
    }

private:
    struct Lane final {
        std::mutex mutex;
        std::array<ZoneService*, zonesPerLane> zones {};
        std::size_t count = 0, cursor = 0;
        std::thread worker;
    };
    std::array<Lane, lanesCount> lanes;
    std::mutex registrationMutex;
    std::atomic<bool> stopping {false};

    StreamingPool() {
        for (std::size_t i = 0; i < lanesCount; ++i)
            lanes[i].worker = std::thread([this, i] { runLane(i); });
    }
    ~StreamingPool() {
        stopping.store(true, std::memory_order_release);
        for (auto& lane : lanes)
            if (lane.worker.joinable()) lane.worker.join();
    }
    void runLane(std::size_t index) noexcept {
        auto& lane = lanes[index];
        while (!stopping.load(std::memory_order_acquire)) {
            bool processed = false;
            {
                const std::lock_guard<std::mutex> guard(lane.mutex);
                if (lane.count > 0) {
                    const auto first = lane.cursor % lane.count;
                    for (std::size_t i = 0; i < lane.count; ++i) {
                        const auto slot = (first + i) % lane.count;
                        if (lane.zones[slot] && lane.zones[slot]->serviceOneRequest()) {
                            lane.cursor = (slot + 1) % lane.count;
                            processed = true;
                            break;
                        }
                    }
                }
            }
            if (!processed) std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
};
} // namespace liberty::stream_v041
