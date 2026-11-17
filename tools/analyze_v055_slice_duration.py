#!/usr/bin/env python3
"""Diagnostic only: compare SLICE first underflow at 6 versus 8 seconds."""
import argparse
import json
from pathlib import Path
from tools.analyze_v055_slice_first_fault import summarize

def compare(short,long):
    if short.get("scenario")!="slice" or long.get("scenario")!="slice":
        raise ValueError("both reports must be native SLICE")
    for report in (short,long):
        duration=float(report.get("elapsed_wall_sec",0))
        if duration<=0 or not 0<=int(report["underflow_blocks"]):
            raise ValueError("invalid native SLICE report")
    result={}
    for label,source in (("short_6s",short),("long_8s",long)):
        diagnostic=summarize(source)
        duration=float(source["elapsed_wall_sec"])
        first=diagnostic["first_observed"]
        result[label]={
            "duration_seconds":round(duration,3),
            "underflow_blocks":int(source["underflow_blocks"]),
            "cache_misses":int(source["cache_misses"]),
            "first_observed_at_sec":first["at_sec"] if first else None,
            "first_observed_sec_before_end":round(duration-float(first["at_sec"]),4) if first else None,
            "events_before_observed_window":diagnostic["events_before_trace_or_unobserved"]
        }
    return {"by_duration":result,
            "comparison_is_causal_proof":False,
            "qualification_override":False,
            "interpretation":"Repeat on real audio device before identifying any test-boundary effect; every underflow remains a release blocker."}

def main():
    p=argparse.ArgumentParser()
    p.add_argument("short",type=Path)
    p.add_argument("long",type=Path)
    p.add_argument("--output",type=Path,required=True)
    a=p.parse_args()
    result=compare(json.loads(a.short.read_text()),json.loads(a.long.read_text()))
    a.output.write_text(json.dumps(result,indent=2)+"\n")
    print(json.dumps(result,indent=2))
if __name__=="__main__":
    main()
