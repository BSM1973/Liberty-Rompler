#!/usr/bin/env python3
"""Apply and validate the v0.2 editor declaration hotfix before compiling.

The v0.2 UI tarball defines toggleFavourite(bank, preset) but declares
toggleFavourite(preset) in the header.  Avoid a full repack by overlaying the
small, auditable fix into the extracted working tree.
"""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[1]
header_path = root / "Source" / "PluginEditor.h"
cpp_path = root / "Source" / "PluginEditor.cpp"

header = header_path.read_text(encoding="utf-8")
cpp = cpp_path.read_text(encoding="utf-8")

incorrect = r"\bvoid\s+toggleFavourite\s*\(\s*int\s*\)\s*;"
correct = r"\bvoid\s+toggleFavourite\s*\(\s*int\s*,\s*int\s*\)\s*;"
definition = (
    r"\bvoid\s+LibertyRomplerAudioProcessorEditor::toggleFavourite\s*"
    r"\(\s*int\s+\w+\s*,\s*int\s+\w+\s*\)"
)
if len(re.findall(definition, cpp)) != 1:
    raise SystemExit("ERREUR : définition C++ toggleFavourite(bank, preset) absente ou multiple")

bad = list(re.finditer(incorrect, header))
good = list(re.finditer(correct, header))
if len(bad) == 1 and not good:
    header = re.sub(incorrect, "void toggleFavourite(int, int);", header, count=1)
    header_path.write_text(header, encoding="utf-8")
    print("Correctif appliqué : toggleFavourite(int, int)")
elif len(good) == 1 and not bad:
    print("Correctif déjà présent : toggleFavourite(int, int)")
else:
    raise SystemExit("ERREUR : déclaration C++ inattendue ; arrêt avant compilation")

if len(re.findall(correct, header_path.read_text(encoding="utf-8"))) != 1:
    raise SystemExit("ERREUR : signature C++ incohérente après correction")
if "\ufffd" in header or "\ufffd" in cpp:
    raise SystemExit("ERREUR : caractère Unicode de remplacement dans l'éditeur")
print("Vérification statique de déclaration / définition : OK")
