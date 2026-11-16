#!/usr/bin/env python3
"""Identify FIRST native SLICE underflow/cache failure from 512-frame counter traces.

No audio gate override. The previous coarse 16-block poll time is NOT the
event time. For a first event before last-second tracing, report bounded
uncertainty rather than invent a timestamp.
"""
import argparse
import json
from pathlib import Path

COUNTERS=("underflows","cache_misses","dropped_pages","urgent_dropped")

def summarize(report):
    trace=report.get("slice_last_second_diagnostics",[])
    found=[]
    previous={key:0 for key in COUNTERS}
    for event in trace:
        at=float(event["at_sec"])
        for key in COUNTERS:
            now=int(event[key])
            if now>previous[key]:
                found.append({"at_sec":at,"counter":key,
                              "increase":now-previous[key],
                              "cumulative":now})
            previous[key]=now
    final={
        "underflows":int(report.get("underflow_blocks",0)),
        "cache_misses":int(report.get("cache_misses",0)),
        "dropped_pages":int(report.get("dropped_pages",0)),
        "urgent_dropped":int(report.get("urgent_dropped",0)),
    }
    early={key:max(0,final[key]-sum(x["increase"] for x in found if x["counter"]==key))
           for key in COUNTERS}
    return {"scenario":report.get("scenario"),"first_observed":found[0] if found else None,
        "all_observed_in_final_second":found,"events_before_trace_or_unobserved":early,
        "final_counts":final,
        "time_resolution":"One processBlock (512/44100 = ~11.61 ms), not sample exact",
        "qualification_override":False}

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("reports",type=Path,nargs="+")
    ap.add_argument("--output",type=Path,required=True)
    args=ap.parse_args()
    result={p.stem:summarize(json.loads(p.read_text("utf-8"))) for p in args.reports}
    args.output.write_text(json.dumps(result,indent=2)+"\n",encoding="utf-8")
    print(json.dumps(result,indent=2))

if __name__=="__main__":
    main()
