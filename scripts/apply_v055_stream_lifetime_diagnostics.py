#!/usr/bin/env python3
"""v0.55 diagnostic only: count live StreamedZone objects and sample RSS after
switching back to a one-layer preset, without touching the existing RT pass gate.
Apply to reconstructed sources after all v0.54 + v0.55 audio overlays.
"""
from pathlib import Path
h=Path("Source/StreamedZoneV040.h")
s=h.read_text("utf-8")
def only(txt,old,new,label):
    n=txt.count(old)
    if n!=1:raise SystemExit(f"v0.55 lifetime anchor {label}: {n}")
    return txt.replace(old,new,1)
# Constructor and destructor signatures are modified by v0.52/v0.54. Instead
# track the lifetime of a member RAII sentinel. This needs no knowledge of
# the constructor's initializer list, and cannot miss an early destructor.
import re
match=re.search(r'class\s+StreamedZone\s+final\s*:\s*public\s+[^{]+\{\s*public:',s)
if not match:
    raise SystemExit("v0.55 cannot locate reconstructed StreamedZone class")
sentinel=r'''public:
    static int liveCount() noexcept {
        return liveInstances.load(std::memory_order_relaxed);
    }
private:
    inline static std::atomic<int> liveInstances{0};
    struct LifetimeSentinel {
        LifetimeSentinel() noexcept {
            liveInstances.fetch_add(1,std::memory_order_relaxed);
        }
        ~LifetimeSentinel() noexcept {
            liveInstances.fetch_sub(1,std::memory_order_relaxed);
        }
    };
    LifetimeSentinel lifetimeSentinel;
public:'''
s=s[:match.start()]+s[match.start():match.end()].replace('public:',sentinel,1)+s[match.end():]
h.write_text(s,"utf-8")
p=Path("tests/rompler_audio_stress_v044.cpp")
s=p.read_text("utf-8")
# Apple malloc statistics: size_in_use approximates the active allocation
# footprint; size_allocated shows allocator-reserved heap (not total RSS).
# Collect only outside callback, after original timing metrics.
s=only(s,'#include <sys/resource.h>',
r'''#include <sys/resource.h>
#ifdef __APPLE__
#include <malloc/malloc.h>
#endif''',"malloc-header")
s=only(s,'static void insist(bool ok, const juce::String& msg) {',
r'''struct AllocatorStatsV055 {
    double inUseMb=-1., reservedMb=-1.;
};
static AllocatorStatsV055 allocatorStatsV055() noexcept {
#ifdef __APPLE__
    malloc_statistics_t stats{};
    malloc_zone_statistics(malloc_default_zone(),&stats);
    return {double(stats.size_in_use)/1.e6,
            double(stats.size_allocated)/1.e6};
#else
    return {};
#endif
}
static void insist(bool ok, const juce::String& msg) {''',"malloc-helper")
s=only(s,'#include "PluginProcessor.h"',
'''#include "PluginProcessor.h"
#include "StreamedZoneV040.h"''',"include")
s=only(s,'        std::cout<<j.str();\n        processor.releaseResources();',
r'''        std::cout<<j.str();
        if(mode=="switch" && duration>=55.) {
            // Diagnostics are AFTER all timing/underflow measurements and the
            // original strict report. No impact on callback qualifications.
            using liberty::stream_v040::StreamedZone;
            const auto beforeRss=currentRssMb();
            const auto beforeAllocator=allocatorStatsV055();
            const auto beforeZones=StreamedZone::liveCount();
            const auto back=processor.selectPreset(
                "liberty-stress-v044","stress-layer-1");
            insist(back.wasOk(),"Retour au preset simple impossible");
            const auto afterSwitchRss=currentRssMb();
            const auto afterAllocator=allocatorStatsV055();
            const auto afterSwitchZones=StreamedZone::liveCount();
            std::this_thread::sleep_for(std::chrono::milliseconds(750));
            const auto idleRss=currentRssMb();
            const auto idleAllocator=allocatorStatsV055();
            const auto idleZones=StreamedZone::liveCount();
            const auto afterDiagnostics=processor.getStreamingDiagnostics();
            std::ostringstream lifetime;
            lifetime<<std::fixed<<std::setprecision(3)
                <<"{\"source\":\"real-JUCE native switch 60s post-test\","
                <<"\"prior_rss_mb\":"<<beforeRss<<","
                <<"\"after_single_preset_rss_mb\":"<<afterSwitchRss<<","
                <<"\"idle_rss_mb\":"<<idleRss<<","
                <<"\\\"malloc_in_use_before_mb\\\":"<<beforeAllocator.inUseMb<<","
                <<"\\\"malloc_in_use_after_switch_mb\\\":"<<afterAllocator.inUseMb<<","
                <<"\\\"malloc_in_use_idle_mb\\\":"<<idleAllocator.inUseMb<<","
                <<"\\\"malloc_reserved_before_mb\\\":"<<beforeAllocator.reservedMb<<","
                <<"\\\"malloc_reserved_after_switch_mb\\\":"<<afterAllocator.reservedMb<<","
                <<"\\\"malloc_reserved_idle_mb\\\":"<<idleAllocator.reservedMb<<","
                <<"\"live_stream_zones_before\":"<<beforeZones<<","
                <<"\"live_stream_zones_after_switch\":"<<afterSwitchZones<<","
                <<"\"live_stream_zones_after_750ms\":"<<idleZones<<","
                <<"\"streamed_zones_after_switch\":"<<afterDiagnostics.streamedZones<<","
                <<"\"notes\":\"RSS retained by allocator is not proof of live StreamedZone retention\"}\n";
            const auto detail=path.getSiblingFile("switch-lifetime-v055.json");
            insist(detail.replaceWithText(juce::String::fromUTF8(lifetime.str().c_str())),
                   "Impossible d'enregistrer le diagnostic du cache");
            std::cout<<lifetime.str();
        }
        processor.releaseResources();''',"post test")
p.write_text(s,"utf-8")
print("v0.55 lifetime diagnostics instrumented: atomic live StreamedZone instances and post-switch RSS.")
