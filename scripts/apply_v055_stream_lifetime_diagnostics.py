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
s=only(s,'    explicit StreamedZone(std::unique_ptr<juce::AudioFormatReader> source)\n        : reader(std::move(source), static_cast<int>(pageFrames)) {}',
'''    explicit StreamedZone(std::unique_ptr<juce::AudioFormatReader> source)
        : reader(std::move(source), static_cast<int>(pageFrames)) {
        liveInstances.fetch_add(1,std::memory_order_relaxed);
    }
    static int liveCount() noexcept {
        return liveInstances.load(std::memory_order_relaxed);
    }''',"constructor")
s=only(s,'    ~StreamedZone() override {\n        // Off the audio thread;',
'''    ~StreamedZone() override {
        // Off the audio thread;''',"destructor")
s=only(s,'        if (registered)\n            liberty::stream_v041::StreamingPool::instance().unregisterZone(this);\n    }',
'''        if (registered)
            liberty::stream_v041::StreamingPool::instance().unregisterZone(this);
        liveInstances.fetch_sub(1,std::memory_order_relaxed);
    }''',"decrement")
s=only(s,'private:\n    bool loadPage',
'''private:
    inline static std::atomic<int> liveInstances{0};
    bool loadPage''',"counter")
h.write_text(s,"utf-8")
p=Path("tests/rompler_audio_stress_v044.cpp")
s=p.read_text("utf-8")
s=only(s,'#include "PluginProcessor.h"',
'''#include "PluginProcessor.h"
#include "StreamedZoneV040.h"''',"include")
s=only(s,'        std::cout<<j.str();\n        processor.releaseResources();',
'''        std::cout<<j.str();
        if(mode=="switch" && duration>=55.) {
            // Diagnostics are AFTER all timing/underflow measurements and the
            // original strict report. No impact on callback qualifications.
            using liberty::stream_v040::StreamedZone;
            const auto beforeRss=currentRssMb();
            const auto beforeZones=StreamedZone::liveCount();
            const auto back=processor.selectPreset(
                "liberty-stress-v044","stress-layer-1");
            insist(back.wasOk(),"Retour au preset simple impossible");
            const auto afterSwitchRss=currentRssMb();
            const auto afterSwitchZones=StreamedZone::liveCount();
            std::this_thread::sleep_for(std::chrono::milliseconds(750));
            const auto idleRss=currentRssMb();
            const auto idleZones=StreamedZone::liveCount();
            const auto afterDiagnostics=processor.getStreamingDiagnostics();
            std::ostringstream lifetime;
            lifetime<<std::fixed<<std::setprecision(3)
                <<"{\\"source\\":\\"real-JUCE native switch 60s post-test\\","
                <<"\\"prior_rss_mb\\":"<<beforeRss<<","
                <<"\\"after_single_preset_rss_mb\\":"<<afterSwitchRss<<","
                <<"\\"idle_rss_mb\\":"<<idleRss<<","
                <<"\\"live_stream_zones_before\\":"<<beforeZones<<","
                <<"\\"live_stream_zones_after_switch\\":"<<afterSwitchZones<<","
                <<"\\"live_stream_zones_after_750ms\\":"<<idleZones<<","
                <<"\\"streamed_zones_after_switch\\":"<<afterDiagnostics.streamedZones<<","
                <<"\\"notes\\":\\"RSS retained by allocator is not proof of live StreamedZone retention\\"}\\n";
            const auto detail=path.getSiblingFile("switch-lifetime-v055.json");
            insist(detail.replaceWithText(juce::String::fromUTF8(lifetime.str().c_str())),
                   "Impossible d'enregistrer le diagnostic du cache");
            std::cout<<lifetime.str();
        }
        processor.releaseResources();''',"post test")
p.write_text(s,"utf-8")
print("v0.55 lifetime diagnostics instrumented: atomic live StreamedZone instances and post-switch RSS.")
