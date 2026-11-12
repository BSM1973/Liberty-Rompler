#!/usr/bin/env python3
"""v0.44: reserve transition headroom for two full 64-zone banks.

The per-preset 64-zone limit stays untouched; this expands only the global
non-RT worker registry from 64 to 128 simultaneous registered zone objects.
Run AFTER v0.43 integration and BEFORE unit tests / native JUCE build.
"""
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
def patch(path,changes):
    p=ROOT/path
    s=p.read_text(encoding="utf-8")
    for old,new in changes:
        n=s.count(old)
        if n<1:
            raise SystemExit(f"v0.44 missing anchor in {path}: {old[:100]!r}")
        s=s.replace(old,new)
    p.write_text(s,encoding="utf-8")

patch("Source/StreamingPoolV041.h",[
    ("static constexpr std::size_t zonesPerLane = 16;",
     "static constexpr std::size_t zonesPerLane = 32; // 64 active + up to 64 staged"),
    ("// Shared across all loaded zones in the plugin module; four disk workers",
     "// During preset swaps we need room for active AND staged zones.\n"
     "// Shared across all loaded zones in the plugin module; four disk workers")
])

patch("tests/streaming_pool_v041_test.cpp",[
    ("std::array<Zone, 65> zones {};", "std::array<Zone, 129> zones {};"),
    ("i < 64", "i < 128"),
    ("pool.registeredZones() == 64", "pool.registeredZones() == 128"),
    ("zones[64]", "zones[128]"),
    ("four-worker shared streaming pool 64 zones OK",
     "four-worker shared streaming pool 128 registrations OK")
])
patch("tests/test_v041_shared_pool.py",[
    ('"zonesPerLane = 16"', '"zonesPerLane = 32"'),
    ('four-worker shared streaming pool 64 zones OK',
     'four-worker shared streaming pool 128 registrations OK')
])
patch("tests/test_v042_priority_health.py",[
    ('"zonesPerLane = 16"', '"zonesPerLane = 32"')
])
print("v0.44: 128 global worker registrations including temporary transition headroom.")
