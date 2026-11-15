#!/usr/bin/env python3
"""Analyze 20-second real-JUCE preset-switch resident memory samples.
Informational only: no arbitrary leak threshold and no DAW claims.
"""
import argparse
import json
import math
import statistics
from pathlib import Path

def analyze(report):
    values=report.get("switch_rss_trace",[])
    points=[(float(x["at_sec"]),float(x["rss_mb"])) for x in values
            if float(x["rss_mb"])>0 and math.isfinite(float(x["rss_mb"]))]
    if len(points)<8:
        raise ValueError("Not enough valid real-JUCE RSS observations")
    samples=[x[1] for x in points]
    quarter=max(2,len(samples)//4)
    early=statistics.median(samples[:quarter])
    late=statistics.median(samples[-quarter:])
    return {
        "scenario":report.get("scenario"),
        "rss_points":len(points),
        "duration_seconds":max(x[0] for x in points),
        "rss_start_mb":round(samples[0],3),
        "rss_end_mb":round(samples[-1],3),
        "rss_min_mb":round(min(samples),3),
        "rss_max_mb":round(max(samples),3),
        "median_first_quarter_mb":round(early,3),
        "median_final_quarter_mb":round(late,3),
        "median_quarter_growth_mb":round(late-early,3),
        "interpretation":"RSS trend only; no leak verdict without repeat runs and a steady-state baseline",
        "full_plugin_or_daw_qualified":False,
    }

def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("report",type=Path)
    parser.add_argument("--output",type=Path,required=True)
    args=parser.parse_args()
    report=json.loads(args.report.read_text("utf-8"))
    if report.get("scenario")!="switch":
        raise SystemExit("Expected switch benchmark report")
    result=analyze(report)
    args.output.write_text(json.dumps(result,indent=2)+"\n",encoding="utf-8")
    print(json.dumps(result,indent=2))

if __name__=="__main__":
    main()
