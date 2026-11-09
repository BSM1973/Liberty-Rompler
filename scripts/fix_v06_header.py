#!/usr/bin/env python3
"""Idempotent corrective overlay for the v0.6 generated multisample header.

The v0.6 patch duplicated LayerPresetDefinition and PresetDescriptor.layers.
Stop CI if a future source layout differs, rather than blindly deleting code.
"""
from pathlib import Path
import re

SOURCE = Path(__file__).resolve().parents[1] / 'Source' / 'BankLibrary.h'
if not SOURCE.is_file():
    raise SystemExit(f'ERREUR : fichier introuvable : {SOURCE}')
source = SOURCE.read_text(encoding='utf-8')
marker = ("// Optional four-layer preset format. Direct references only (no nested layer presets).\n"
          "struct LayerPresetDefinition {\n"
          "    juce::String bankId, presetId;\n"
          "    bool enabled = true;\n"
          "    float gain = 0.6f, pan = 0.0f, cutoffHz = 20000.0f;\n"
          "    int transpose = 0, lowNote = 0, highNote = 127, lowVelocity = 1, highVelocity = 127;\n"
          "};\n")
member = "    std::vector<LayerPresetDefinition> layers;\n"
counts = (source.count(marker), source.count(member))
if counts == (2, 2):
    if marker + marker not in source or member + member not in source:
        raise SystemExit('ERREUR : doublons C++ non adjacents, correction automatique refusée')
    source = source.replace(marker + marker, marker, 1)
    source = source.replace(member + member, member, 1)
    SOURCE.write_text(source, encoding='utf-8')
    print('Correctif appliqué : une seule structure LayerPresetDefinition et un seul membre layers')
elif counts == (1, 1):
    print('Correctif déjà présent : définitions de couches uniques')
else:
    raise SystemExit(f'ERREUR : signatures inattendues (struct={counts[0]}, member={counts[1]})')

result = SOURCE.read_text(encoding='utf-8')
assert len(re.findall(r'\bstruct\s+LayerPresetDefinition\s*\{', result)) == 1
assert len(re.findall(r'\bstd::vector<LayerPresetDefinition>\s+layers\s*;', result)) == 1
assert '\ufffd' not in result
print('Validation du fichier d’en-tête : OK')
