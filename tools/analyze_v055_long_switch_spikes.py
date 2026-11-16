#!/usr/bin/env python3
"""Analyze long JUCE preset-switch overruns relative to preset boundaries.

A spike immediately adjacent to selectPreset may implicate loading; a spike
elsewhere can indicate preemption. Correlation is NOT a causal attribution.
No qualification threshold is weakened.
"""
import argparse
import json
import math
from pathlib import Path
from tools.analyze_v055_callback_spikes import classify

def analyze(report):
    if report.get("scenario")!="switch":
        raise ValueError("requires a switch stress report")
    summary=classify({"scenarios":{
        "poly64":{"rate_hz":report["rate_hz"],"block_frames":report["block_frames"],
                   "callback_spikes":[]},
        "switch":report,
        "slice":{"rate_hz":report["rate_hz"],"block_frames":report["block_frames"],
                 "callback_spikes":[]}
    }})
    times=sorted(float(x["at_sec"]) for x in report.get("switch_rss_trace",[]))
    for item in summary["spikes"]:
        when=item.get("at_sec")
        if when is None or not times:
            item["nearest_preset_switch_sec"]=None
            item["distance_to_preset_switch_ms"]=None
            item["within_25ms_of_preset_switch"]=None
        else:
            when=float(when)
            if not math.isfinite(when):raise ValueError("invalid spike timestamp")
            nearest=min(times,key=lambda t:abs(t-when))
            distance=1000.*abs(nearest-when)
            item["nearest_preset_switch_sec"]=nearest
            item["distance_to_preset_switch_ms"]=round(distance,3)
            item["within_25ms_of_preset_switch"]=distance<=25.
    # Cluster adjacent overruns to distinguish isolated outliers from bursts
    # of off-CPU scheduler starvation. Report-only, NOT a qualification waiver.
    ordered=sorted((x for x in summary["spikes"] if x["at_sec"] is not None),
                   key=lambda x:float(x["at_sec"]))
    bursts=[]
    for event in ordered:
        at=float(event["at_sec"])
        if not math.isfinite(at):raise ValueError("invalid spike timestamp")
        if not bursts or at-bursts[-1]["last_at_sec"]>0.5:
            bursts.append({"first_at_sec":at,"last_at_sec":at,"count":0,
                           "total_non_cpu_wall_ms":0.0,"max_callback_wall_ms":0.0})
        group=bursts[-1]
        group["last_at_sec"]=at
        group["count"]+=1
        group["total_non_cpu_wall_ms"]+=event["non_cpu_wall_ms"] or 0.0
        group["max_callback_wall_ms"]=max(group["max_callback_wall_ms"],event["callback_wall_ms"])
    for group in bursts:
        group["total_non_cpu_wall_ms"]=round(group["total_non_cpu_wall_ms"],3)
    summary["callback_overrun_bursts_500ms"]=bursts
    summary["largest_burst_overruns"]=max((x["count"] for x in bursts),default=0)
    summary["overruns_in_final_10s"]=sum(float(x["at_sec"])>=50.0 for x in ordered)
    summary["spikes_near_preset_switch"]=sum(
        x["within_25ms_of_preset_switch"] is True for x in summary["spikes"])
    summary["long_run_callback_over_budget"]=int(report.get("callback_over_budget",0))
    summary["counter_matches_spikes"]=summary["observed_spikes"]==summary["long_run_callback_over_budget"]
    summary["scope"]="native macOS JUCE offline runner, not hardware/DAW scheduling"
    summary["causality_proven"]=False
    return summary

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("report",type=Path)
    parser.add_argument("--output",type=Path,required=True)
    args=parser.parse_args()
    result=analyze(json.loads(args.report.read_text("utf8")))
    args.output.write_text(json.dumps(result,indent=2)+"\n",encoding="utf8")
    print(json.dumps(result,indent=2))
if __name__=="__main__":
    main()
