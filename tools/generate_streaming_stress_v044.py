#!/usr/bin/env python3
"""Original v1 four-layer / 64-zone stress bank; never overwrite existing data."""
import argparse
import json
import math
from pathlib import Path
import struct
import wave

BANK = 'liberty-stress-v044'

def generate(output: Path, seconds: float = 24., rate: int = 44100,
             zones_per_layer: int = 16) -> Path:
    if not (0.1 <= seconds <= 120 and 22050 <= rate <= 192000 and 1 <= zones_per_layer <= 16):
        raise ValueError('invalid duration, rate or number of zones')
    output = output.expanduser().resolve()
    output.mkdir(parents=True, exist_ok=False)
    sample_dir = output / 'Samples'
    sample_dir.mkdir()
    length = round(seconds * rate)
    presets = []
    for layer in range(4):
        zones = []
        for index in range(zones_per_layer):
            note = 48 + index
            name = f'layer{layer + 1}_n{note}.wav'
            hz = 440 * 2 ** ((note - 69) / 12) * (1 + layer * .0014)
            with wave.open(str(sample_dir / name), 'wb') as w:
                w.setnchannels(2)
                w.setsampwidth(2)
                w.setframerate(rate)
                for offset in range(0, length, 4096):
                    frames = bytearray()
                    for frame in range(offset, min(offset + 4096, length)):
                        t = frame / rate
                        amp = min(1., t / .015)
                        value = round(11000 * amp * math.sin(2 * math.pi * hz * t))
                        frames.extend(struct.pack('<hh', value, round(value * .92)))
                    w.writeframesraw(frames)
            zone = {'file': 'Samples/' + name, 'root_note': note,
                    'low_note': note, 'high_note': note,
                    'low_velocity': 1, 'high_velocity': 127, 'gain': .35}
            if seconds > 20:
                zone.update(loop_start=round(length * .25),
                            loop_end=round(length * .8))
            zones.append(zone)
        presets.append({'id': f'stress-layer-{layer + 1}',
                        'name': f'Test longue couche {layer + 1}',
                        'category': 'Stress', 'zones': zones})
    presets.append({
        'id': 'stress-4-layers', 'name': 'Stress 4 couches / 64 voix',
        'category': 'Stress', 'zones': [],
        'layers': [
            {'source_bank': BANK, 'source_preset': f'stress-layer-{layer + 1}',
             'enabled': True, 'gain': .36, 'pan': 0., 'cutoff': 20000.,
             'transpose': 0, 'low_note': 48,
             'high_note': 48 + zones_per_layer - 1,
             'low_velocity': 1, 'high_velocity': 127}
            for layer in range(4)]
    })
    manifest = {'format': 'liberty-rompler-bank-v1', 'id': BANK,
                'name': 'Liberty Streaming Stress v0.44', 'version': '0.44.0',
                'license_mode': 'demo-unprotected', 'presets': presets}
    path = output / 'manifest.json'
    path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    return path

if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--seconds', type=float, default=24.)
    p.add_argument('--rate', type=int, default=44100)
    p.add_argument('--zones-per-layer', type=int, default=16)
    a = p.parse_args()
    print('Bank generated:', generate(a.output, a.seconds, a.rate, a.zones_per_layer))
