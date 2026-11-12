#!/usr/bin/env python3
"""Liberty ROMpler v0.45: apply SHA256-verified Sampling Studio source overlay.

Apply AFTER the complete v0.44 integrations, BEFORE validating banks, JUCE
compilation, or running Python/C++ regression tests. Never commit the generated
v0.45 Source/ files back to the branch; leave the v0.39 cache pristine.
"""
from pathlib import Path
import base64
import hashlib
import lzma
import subprocess

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "assets/v045/source-patch.b64"
EXPECTED_SIZE = 13280
EXPECTED_SHA256 = "d3408ff0fcf8635480cd368f6ae5c72aca3c56bda1042227493788eee0a5af19"
encoded = "".join(SOURCE.read_text(encoding="ascii").split())
archive = base64.b64decode(encoded, validate=True)
if len(archive) != EXPECTED_SIZE or hashlib.sha256(archive).hexdigest() != EXPECTED_SHA256:
    raise SystemExit("v0.45 Sampling Studio patch SHA-256/length mismatch")
patch = lzma.decompress(archive)
if len(patch) != 54031:
    raise SystemExit("v0.45 patch decoded size mismatch")
subprocess.run(["patch", "--batch", "--forward", "-p1"],
               input=patch, cwd=ROOT, check=True)
for relative in ("Source/KeyVelocityMapV045.h",
                 "tests/key_velocity_v045_test.cpp",
                 "tests/test_v045_sampling_studio.py",
                 "Source/BankLibrary.cpp", "Source/SampleZoneView.h",
                 "tools/smart_import_v037.py"):
    if not (ROOT / relative).is_file():
        raise SystemExit("v0.45 missing integrated source: " + relative)
if b"\xef\xbf\xbd" in (ROOT / "Source/SampleZoneView.h").read_bytes():
    raise SystemExit("v0.45 corrupt UTF-8 in GUI")
print("Liberty ROMpler v0.45: SHA-256 verified Sampling Studio applied.")
