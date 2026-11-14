#pragma once
// Liberty v0.51: pure, bounded memory policy; no JUCE / allocation required.
#include <cstddef>
namespace liberty::resource_v051 {
constexpr std::size_t streamCachePages = 32;
constexpr std::size_t framesPerCachePage = 4096;
constexpr std::size_t maxPresetZones = 128;
// Only reuse identical source names with the same full zone count.
// Explicit edits/session restores must force a fresh decoded sound pool.
constexpr bool canReuseLayer(bool forceReload, bool identicalSource,
                             int existingZones, int requestedZones) noexcept {
    return !forceReload && identicalSource && existingZones > 0 &&
           existingZones == requestedZones && requestedZones <= (int)maxPresetZones;
}
constexpr std::size_t maxCacheBytesPerZone =
    streamCachePages * framesPerCachePage * 2 * sizeof(float);
}
