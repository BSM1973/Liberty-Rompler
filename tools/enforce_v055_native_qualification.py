#!/usr/bin/env python3
"""Fail a v0.55 macOS qualification if the native JUCE report is unqualified.

The historical v0.44 reporter intentionally writes a JSON report and exits 0
for a valid (but unqualified) measurement. The v0.55 release gate must not
mistake that report-generation success for real-time qualification.
"""
import argparse
import json
import math
from pathlib import Path

SCENARIOS=("poly64","switch","slice")
COUNTERS=("callback_over_budget","underflow_blocks","decoder_errors",
          "urgent_dropped","dropped_pages","nonfinite_samples")

def disqualifications(report):
    issues=[]
    cases=report.get("scenarios",{})
    if set(cases)!=set(SCENARIOS):
        return ["Missing or unexpected native stress scenario"]
    for name in SCENARIOS:
        data=cases[name]
        for counter in COUNTERS:
            value=data.get(counter)
            if isinstance(value,bool) or not isinstance(value,(int,float)) or not math.isfinite(value) or value<0:
                issues.append(f"{name}: invalid {counter}={value!r}")
            elif value:
                issues.append(f"{name}: {counter}={value}")
        if name=="poly64" and data.get("suspicious_silent_sustain_blocks",0):
            issues.append(f"{name}: silent sustain")
        if data.get("streaming_qualified") is not True:
            issues.append(f"{name}: native scenario not qualified")
    if report.get("qualified_on_ci_machine") is not True:
        issues.append("native JUCE CI qualification is false")
    return issues

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("qualification",type=Path)
    args=ap.parse_args()
    report=json.loads(args.qualification.read_text(encoding="utf-8"))
    issues=disqualifications(report)
    if issues:
        for message in issues:
            print("QUALIFICATION FAIL:",message)
        raise SystemExit(1)
    print("v0.55 native JUCE budget/streaming gate: PASS, no reported incidents")

if __name__=="__main__":
    main()
