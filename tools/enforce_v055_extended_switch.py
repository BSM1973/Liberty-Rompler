#!/usr/bin/env python3
"""Strict extended-switch real-JUCE gate: no hidden >budget blocks in 60s run.

This test is separate from the three short v0.44 scenarios; it does not
replace them, reduce thresholds or claim DAW/DAC validation.
"""
import argparse
import json
import math
from pathlib import Path

def failures(report,expected_scenario="switch"):
    issues=[]
    if report.get("scenario")!=expected_scenario:
        issues.append(f"expected {expected_scenario} stress scene")
    if float(report.get("elapsed_wall_sec",0))<55.:
        issues.append("extended switch stress lasted less than 55 wall seconds")
    for field in ("callback_over_budget","underflow_blocks","cache_misses",
                  "dropped_pages","urgent_dropped","decoder_errors","nonfinite_samples"):
        value=report.get(field)
        if isinstance(value,bool) or not isinstance(value,(int,float)) or not math.isfinite(value) or value<0:
            issues.append(f"invalid {field}: {value}")
        elif value>0:
            issues.append(f"{field}: {value}")
    if not report.get("signal_peak",0)>0:
        issues.append("no audible test signal")
    return issues

def main():
    p=argparse.ArgumentParser()
    p.add_argument("report",type=Path)
    p.add_argument("--scenario",choices=("switch","poly64"),default="switch")
    args=p.parse_args()
    data=json.loads(args.report.read_text(encoding="utf-8"))
    problems=failures(data,args.scenario)
    if problems:
        for item in problems:
            print("EXTENDED PRESET FAIL:",item)
        raise SystemExit(1)
    print(f"Extended 60-second real-JUCE {args.scenario} stress: PASS")

if __name__=="__main__":
    main()
