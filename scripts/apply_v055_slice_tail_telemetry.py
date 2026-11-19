#!/usr/bin/env python3
"""Extend existing native SLICE JSON with final-second, per-block diagnostics.
Read-only telemetry: zero changes to playback, underflow counters, or pass gates.
Apply AFTER reconstructed Source and BEFORE building LibertyStressV044.
"""
from pathlib import Path
p=Path("tests/rompler_audio_stress_v044.cpp")
s=p.read_text(encoding="utf-8")
def one(old,new):
    global s
    if s.count(old)!=1:raise SystemExit(f"SLICE telemetry anchor mismatch {s.count(old)}: {old[:90]}")
    s=s.replace(old,new,1)
one('        underflowTimeline.reserve(32);',
'''        underflowTimeline.reserve(32);
        struct LateSample { double sec; std::uint64_t underflows,misses,dropped,urgent; };
        std::vector<LateSample> finalSecond;
        finalSecond.reserve(256);
        std::vector<LateSample> fullRun;
        fullRun.reserve(1200);''')
one('''            if(mode=="poly64" && nominal>.5''',
'''            // For the final second of SLICE, observe every callback instead
            // of every 16th block. This captures the true counter edge
            // (the previous timeline at 5.991s may only be its first poll).
            if(mode=="slice" && (duration<=12.0 || nominal>=duration-1.0)) {
                const auto snap=processor.getStreamingDiagnostics();
                const auto& total=snap.totals;
                const LateSample sample{nominal,total.underflowBlocks,
                    total.cacheMisses,total.droppedPages,total.urgentDropped};
                if(duration<=12.0) fullRun.push_back(sample);
                if(nominal>=duration-1.0) finalSecond.push_back(sample);
            }
            if(mode=="poly64" && nominal>.5''')
one('''        std::ostringstream timeline;
        timeline << "[";''',
'''        std::ostringstream finalTrace;
        finalTrace << "[";
        for(size_t i=0;i<finalSecond.size();++i) {
            if(i) finalTrace << ",";
            const auto& x=finalSecond[i];
            finalTrace << "{\\"at_sec\\":" << std::fixed << std::setprecision(4)
                       << x.sec << ",\\"underflows\\":" << x.underflows
                       << ",\\"cache_misses\\":" << x.misses
                       << ",\\"dropped_pages\\":" << x.dropped
                       << ",\\"urgent_dropped\\":" << x.urgent << "}";
        }
        finalTrace << "]";
        std::ostringstream fullTrace;
        fullTrace << "[";
        for(size_t i=0;i<fullRun.size();++i) {
            if(i) fullTrace << ",";
            const auto& x=fullRun[i];
            fullTrace << "{\\"at_sec\\":" << std::fixed << std::setprecision(4)
                      << x.sec << ",\\"underflows\\":" << x.underflows
                      << ",\\"cache_misses\\":" << x.misses
                      << ",\\"dropped_pages\\":" << x.dropped
                      << ",\\"urgent_dropped\\":" << x.urgent << "}";
        }
        fullTrace << "]";
        std::ostringstream timeline;
        timeline << "[";''')
# The JSON field is appended to the existing stream expression, preserving
# its original escaped newline and closing brace.
one(r'<<timeline.str()',
    r'<<timeline.str()<<",\n\"slice_last_second_diagnostics\":"<<finalTrace.str()<<",\n\"slice_full_run_diagnostics\":"<<fullTrace.str()')

p.write_text(s,encoding="utf-8")
print("v0.55 native SLICE: full-resolution last-second telemetry enabled; qualification unchanged.")
