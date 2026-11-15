#!/usr/bin/env python3
"""Classify JUCE native callback overruns by wall vs audio-thread CPU.

If thread_cpu_ms <= block budget but callback_ms exceeds it, a stalled/off-CPU
callback is suspected. This is diagnostic, not proof of scheduler cause.
"""
import argparse
import json
import math
from pathlib import Path

def classify(report):
    records=[]
    for name in ("poly64","switch","slice"):
        scene=report.get("scenarios",{}).get(name)
        if not isinstance(scene,dict):
            raise ValueError(f"missing {name}")
        budget=1000.0*float(scene["block_frames"])/float(scene["rate_hz"])
        if not math.isfinite(budget) or budget<=0:
            raise ValueError("invalid callback budget")
        for spike in scene.get("callback_spikes",[]):
            wall=float(spike["callback_ms"])
            thread=spike.get("thread_cpu_ms")
            cpu=float(thread) if thread is not None else None
            if not math.isfinite(wall) or wall<0:
                raise ValueError("invalid callback wall time")
            if cpu is not None and (not math.isfinite(cpu) or cpu<0):
                raise ValueError("invalid callback thread CPU")
            kind=("not_recorded" if cpu is None else
                  "cpu_bound" if cpu>budget else "off_cpu_or_blocked")
            records.append({"scenario":name,"at_sec":spike.get("at_sec"),
                "callback_wall_ms":wall,"callback_thread_cpu_ms":cpu,
                "budget_ms":round(budget,3),"classification":kind})
    return {"observed_spikes":len(records),
            "cpu_bound":sum(r["classification"]=="cpu_bound" for r in records),
            "off_cpu_or_blocked":sum(r["classification"]=="off_cpu_or_blocked" for r in records),
            "unclassified":sum(r["classification"]=="not_recorded" for r in records),
            "spikes":records,
            "limits":"Classification by time accounting only; requires host/OS tracing to establish root cause",
            "qualification_override":False}

def main():
    p=argparse.ArgumentParser()
    p.add_argument("qualification",type=Path)
    p.add_argument("--output",type=Path,required=True)
    a=p.parse_args()
    summary=classify(json.loads(a.qualification.read_text(encoding="utf8")))
    a.output.write_text(json.dumps(summary,indent=2)+"\n",encoding="utf8")
    print(json.dumps(summary,indent=2))

if __name__=="__main__":
    main()
