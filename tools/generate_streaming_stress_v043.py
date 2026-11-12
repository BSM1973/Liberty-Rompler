#!/usr/bin/env python3
"""Generate ORIGINAL 4-layer/64-zone long WAV bank for manual streaming tests.
No external modules, ~270 MB at 24 s / 44.1 kHz / 64 zones, bounded RAM.
Do not install automatically: writes only under --output.
"""
import argparse
import json
import math
from pathlib import Path
import struct
import wave


def generate(output: Path, seconds: float = 24.0, rate: int = 44100,
             zones_per_layer: int = 16) -> Path:
    if not 0.1 <= seconds <= 600.0 or not 8000 <= rate <= 192000:
        raise ValueError("invalid duration or sampling rate")
    if not 1 <= zones_per_layer <= 16:
        raise ValueError("zones per layer must be between 1 and 16")
    output = output.expanduser().resolve()
    # Never overwrite an existing bank or a previous test run.
    output.mkdir(parents=True, exist_ok=False)
    loop_start = round(rate * seconds * 0.25)
    loop_end = round(rate * seconds * 0.80)
    presets = []
    for layer in range(4):
        zones = []
        for zone in range(zones_per_layer):
            note = 48 + zone
            name = f"layer{layer+1}_n{note}.wav"
            path = output / "Samples" / name
            path.parent.mkdir(exist_ok=True)
            frequency = 440.0 * 2.0 ** ((note - 69) / 12.0)
            frequency *= 1.0 + layer * 0.0014
            with wave.open(str(path), "wb") as wav:
                wav.setnchannels(2)
                wav.setsampwidth(2)
                wav.setframerate(rate)
                for offset in range(0, round(seconds * rate), 4096):
                    samples = min(4096, round(seconds * rate) - offset)
                    frames = bytearray()
                    for frame in range(samples):
                        t = (offset + frame) / rate
                        env = min(1.0, t / 0.015)
                        s = int(11000 * env * math.sin(2 * math.pi * frequency * t))
                        frames += struct.pack("<hh", s, int(s * 0.92))
                    wav.writeframesraw(frames)
            item = {"file": "Samples/" + name, "root_note": note,
                    "low_note": note, "high_note": note,
                    "low_velocity": 1, "high_velocity": 127,
                    "gain": 0.35}
            if seconds > 20.0:
                item.update(loop_start=loop_start, loop_end=loop_end,
                            loop_crossfade_frames=round(rate * 0.07))
            zones.append(item)
        presets.append({"id": f"stress-layer-{layer+1}",
                        "name": f"Test longue couche {layer+1}",
                        "category": "Stress", "zones": zones})
    presets.append({"id": "stress-4-layers", "name": "Stress streaming 4 couches",
                    "category": "Stress", "zones": [], "layers": [
                        {"source_bank": "liberty-stress-v043",
                         "source_preset": f"stress-layer-{i+1}",
                         "enabled": True, "gain": 0.36,
                         "low_note": 48, "high_note": 48+zones_per_layer-1}
                        for i in range(4)]})
    manifest = {"format": "liberty-rompler-bank-v1", "id": "liberty-stress-v043",
                "name": "Liberty Streaming Stress v0.43", "version": "0.43.0",
                "license_mode": "demo-unprotected", "presets": presets}
    result = output / "manifest.json"
    result.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n",
                      encoding="utf-8")
    return result


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True,
                        help="New test-bank directory; NEVER points to an existing commercial bank")
    parser.add_argument("--seconds", type=float, default=24.0)
    parser.add_argument("--rate", type=int, default=44100)
    parser.add_argument("--zones-per-layer", type=int, default=16)
    args = parser.parse_args()
    print("Test bank:", generate(args.output, args.seconds, args.rate, args.zones_per_layer))
