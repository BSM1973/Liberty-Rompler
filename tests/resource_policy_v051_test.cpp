#include "../Source/ResourcePolicyV051.h"
#include <cassert>
#include <iostream>
int main() {
    using namespace liberty::resource_v051;
    static_assert(streamCachePages == 16);
    static_assert(maxCacheBytesPerZone == 524288);
    // Per-64-zone bank: 32 MiB instead of 64 MiB of 32-page cache.
    static_assert(64 * maxCacheBytesPerZone == 33554432);
    assert(canReuseLayer(false,true,16,16));
    assert(canReuseLayer(false,true,128,128));
    assert(!canReuseLayer(false,true,16,32));
    assert(!canReuseLayer(false,false,16,16));
    assert(!canReuseLayer(true,true,16,16));
    assert(!canReuseLayer(false,true,0,16));
    assert(!canReuseLayer(false,true,129,129));
    std::cout << "v0.51 bounded cache and unchanged-layer reuse policy OK\n";
}
