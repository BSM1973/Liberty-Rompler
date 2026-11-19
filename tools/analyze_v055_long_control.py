#!/usr/bin/env python3
"""Compare 60s native JUCE poly64 constant-preset versus switch stress.

Descriptive control only: different workloads, runner QoS is not deterministic,
and any over-budget callback still fails the existing strict gates.
"""
import argparse
import json
import math
from pathlib import Path

def metrics(data,expected):
    if data.get("scenario")!=expected:
        raise ValueError(f"expected {expected} scenario")
    duration=float(data.get("elapsed_wall_sec",0))
    if not math.isfinite(duration) or duration<55:
        raise ValueError(f"{expected}: invalid short control duration")
    budget=1000*float(data["block_frames"])/float(data["rate_hz"])
    if not math.isfinite(budget) or budget<=0:
        raise ValueError("invalid audio callback budget")
    spikes=data.get("callback_spikes",[])
    over=int(data.get("callback_over_budget",-1))
    if over<0 or len(spikes)!=over:
        raise ValueError("callback counter does not match recorded spikes")
    off=0
    cpu=0
    unknown=0
    for x in spikes:
        wall=float(x["callback_ms"])
        if not math.isfinite(wall) or wall<=budget:
            raise ValueError("invalid spike above-budget wall timing")
        thread=x.get("thread_cpu_ms")
        if thread is None:
            unknown+=1
        else:
            thread=float(thread)
            if not math.isfinite(thread) or thread<0 or thread>wall:
                raise ValueError("invalid thread CPU spike time")
            if thread>budget:cpu+=1
            else:off+=1
    return {"duration_sec":round(duration,3),"overruns":over,
            "overruns_per_min":round(over*60/duration,3),
            "off_cpu_or_blocked":off,"cpu_bound":cpu,"unclassified":unknown,
            "callback_budget_ms":round(budget,3)}

def compare(control,switch):
    c=metrics(control,"poly64")
    s=metrics(switch,"switch")
    return {"constant_64_voices_control":c,"repeated_preset_switch":s,
            "difference_overruns_per_min":round(s["overruns_per_min"]-c["overruns_per_min"],3),
            "control_has_off_cpu_stalls":c["off_cpu_or_blocked"]>0,
            "switch_has_off_cpu_stalls":s["off_cpu_or_blocked"]>0,
            "causality_proven":False,
            "qualification_override":False,
            "notes":"Same macOS runner, different workloads at different times. Never infer scheduling root cause or pass/fail from differences alone."}

def main():
    p=argparse.ArgumentParser()
    p.add_argument("control",type=Path)
    p.add_argument("switch",type=Path)
    p.add_argument("--output",type=Path,required=True)
    a=p.parse_args()
    result=compare(json.loads(a.control.read_text("utf-8")),
                   json.loads(a.switch.read_text("utf-8")))
    a.output.write_text(json.dumps(result,indent=2)+"\n",encoding="utf-8")
    print(json.dumps(result,indent=2))
if __name__=="__main__":
    main()
