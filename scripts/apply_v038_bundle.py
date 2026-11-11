#!/usr/bin/env python3
"""Apply the checksum-verified v0.38 JUCE creative source patch.
The seven base64 source parts are reviewable in assets/v038; the same exact
changes passed local C++17 and 157 Python regression tests before upload.
Apply AFTER v0.37 integration, BEFORE any tests or JUCE compilation.
"""
from pathlib import Path
import base64
import hashlib
import lzma
import subprocess

ROOT = Path(__file__).resolve().parent.parent
PARTS = sorted((ROOT / "assets" / "v038").glob("part-*.b64"))
EXPECTED = "0aa61bb7e58e23db52f1d5c8180ff47e2a94c6c2b9b2d9f4c770373a50e36f82"

if len(PARTS) != 7:
    raise SystemExit(f"v0.38 source bundle incomplete: {len(PARTS)}/7")
payload = "".join(p.read_text(encoding="ascii").strip() for p in PARTS)
archive = base64.b64decode(payload, validate=True)
if len(archive) != 13748 or hashlib.sha256(archive).hexdigest() != EXPECTED:
    raise SystemExit("v0.38 creative bundle SHA256 or length mismatch")
patch = lzma.decompress(archive)
subprocess.run(["patch", "--batch", "--forward", "-p1"],
               input=patch, check=True, cwd=ROOT)
for name in (
    "Source/CreativeEngineV038.h",
    "Source/CreativeHarmonyMidiV038.h",
    "Source/LibertyCreativePanelV038.h",
    "tests/test_v038_creative.py",
    "tests/creative_v038_test.cpp",
):
    if not (ROOT / name).is_file():
        raise SystemExit("v0.38 source missing: " + name)
print("Liberty ROMpler v0.38: creative source patch checksum verified and integrated.")
