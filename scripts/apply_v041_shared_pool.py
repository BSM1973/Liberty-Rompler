#!/usr/bin/env python3
"""v0.41: attach the fixed four-worker shared streaming pool to JUCE audio.
Apply after scripts/apply_v040_long_wav.py. Safe on old bank/preset state.
"""
from pathlib import Path
ROOT = Path(__file__).resolve().parent.parent
engine=ROOT/"Source/SampleEngine.cpp"
text=engine.read_text(encoding="utf-8")
old="        streamed->start(attackFrames);"
new="""        if (!streamed->start(attackFrames)) {
            streamed.reset();
            data.setSize(0, 0);
            return; // no worker slot: reject source, never play silence secretly
        }"""
if text.count(old)!=1:
    raise SystemExit(f"v0.41: expected one streamed start site, got {text.count(old)}")
engine.write_text(text.replace(old,new,1),encoding="utf-8")
print("v0.41: 4 shared streaming workers connected, missing slot is explicit failure.")
