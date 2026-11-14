#include "../Source/ResourcePolicyV051.h"
#include <cassert>
#include <iostream>
int main() {
    using namespace liberty::resource_v051;
    static_assert(streamCachePages == 32);
    static_assert(maxCacheBytesPerZone == 1048576);
    // Per-64-zone bank: 64 MiB retained to avoid SLICE eviction regressions.
    static_assert(64 * maxCacheBytesPerZone == 67108864);
    assert(canReuseLayer(false,true,16,16));
    assert(canReuseLayer(false,true,128,128));
    assert(!canReuseLayer(false,true,16,32));
    assert(!canReuseLayer(false,false,16,16));
    assert(!canReuseLayer(true,true,16,16));
    assert(!canReuseLayer(false,true,0,16));
    assert(!canReuseLayer(false,true,129,129));
    std::cout << "v0.51 bounded cache and unchanged-layer reuse policy OK\n";
}
