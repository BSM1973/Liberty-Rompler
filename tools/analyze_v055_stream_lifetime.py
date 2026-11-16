#!/usr/bin/env python3
"""Verify captured native StreamedZone instance lifecycle without equating RSS to leaks."""
import argparse
import json
import math
from pathlib import Path

def evaluate(d):
    required=("prior_rss_mb","after_single_preset_rss_mb","idle_rss_mb",
              "live_stream_zones_before","live_stream_zones_after_switch",
              "live_stream_zones_after_750ms","streamed_zones_after_switch")
    for key in required:
        if key not in d or isinstance(d[key],bool) or not isinstance(d[key],(int,float)) or not math.isfinite(d[key]) or d[key]<0:
            raise ValueError(f"Invalid or missing {key}")
    for key in ("live_stream_zones_before","live_stream_zones_after_switch",
                "live_stream_zones_after_750ms","streamed_zones_after_switch"):
        if int(d[key])!=d[key]:raise ValueError(f"Non-integral {key}")
    alloc_fields=("malloc_in_use_before_mb","malloc_in_use_after_switch_mb",
                  "malloc_in_use_idle_mb","malloc_reserved_before_mb",
                  "malloc_reserved_after_switch_mb","malloc_reserved_idle_mb")
    alloc_present=all(x in d for x in alloc_fields)
    if alloc_present:
        for key in alloc_fields:
            value=d[key]
            if isinstance(value,bool) or not isinstance(value,(int,float)) or not math.isfinite(value) or value<0:
                raise ValueError(f"Invalid macOS allocator counter {key}")
    count=int(d["live_stream_zones_after_750ms"])
    # Stress source contains 16 streamed zones per layer, but async
    # retirement can retain old owners; report evidence, do not guess.
    return {
      "allocator_measurements_available":alloc_present,
      "malloc_in_use_released_on_return_mb":round(d["malloc_in_use_before_mb"]-d["malloc_in_use_after_switch_mb"],3) if alloc_present else None,
      "malloc_reserved_released_on_return_mb":round(d["malloc_reserved_before_mb"]-d["malloc_reserved_after_switch_mb"],3) if alloc_present else None,
      "malloc_in_use_delta_during_idle_mb":round(d["malloc_in_use_idle_mb"]-d["malloc_in_use_after_switch_mb"],3) if alloc_present else None,
      "malloc_reserved_at_idle_mb":round(d["malloc_reserved_idle_mb"],3) if alloc_present else None,
      "expected_single_layer_zone_count":16,
      "live_instances_before":int(d["live_stream_zones_before"]),
      "live_instances_just_after_return":int(d["live_stream_zones_after_switch"]),
      "live_instances_after_750ms":count,
      "engine_reported_streamed_zones":int(d["streamed_zones_after_switch"]),
      "count_matches_single_layer":count==16,
      "excess_live_instances_after_750ms":max(0,count-16),
      "rss_change_on_return_mb":round(d["after_single_preset_rss_mb"]-d["prior_rss_mb"],3),
      "rss_change_during_idle_mb":round(d["idle_rss_mb"]-d["after_single_preset_rss_mb"],3),
      "rss_at_idle_mb":round(d["idle_rss_mb"],3),
      "leak_proven":False,
      "leak_excluded":False,
      "notes":"A single steady-state snapshot does not establish causality. Count 16 is expected for one 16-zone stress preset; count above 16 warrants further investigation."
    }

def main():
    p=argparse.ArgumentParser()
    p.add_argument("path",type=Path)
    p.add_argument("--output",type=Path,required=True)
    args=p.parse_args()
    result=evaluate(json.loads(args.path.read_text("utf-8")))
    args.output.write_text(json.dumps(result,indent=2)+"\n",encoding="utf-8")
    print(json.dumps(result,indent=2))
if __name__=="__main__":
    main()
