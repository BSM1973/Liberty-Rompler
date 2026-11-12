#!/usr/bin/env python3
"""Apply checksum-verified v0.46 hybrid engine AFTER v0.45 Sampling Studio.

The generated JUCE source is reconstructed and patched within CI. Do not commit
temporary Source/PluginProcessor.cpp or SampleEngine.cpp back to main.
"""
from pathlib import Path
import base64
import hashlib
import lzma
import subprocess

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "assets/v046/source-patch.b64"
EXPECTED_SIZE = 13412
EXPECTED_SHA256 = "6131a961dbd6ae23cc5061c9d825c655fc43ee38f860d0105283039ad00f596d"
encoded = "".join(SOURCE.read_text(encoding="ascii").split())
archive = base64.b64decode(encoded, validate=True)
if len(archive) != EXPECTED_SIZE or hashlib.sha256(archive).hexdigest() != EXPECTED_SHA256:
    raise SystemExit("Liberty v0.46 patch verification failed (size / SHA256)")
patch = lzma.decompress(archive)
if len(patch) != 56565:
    raise SystemExit("Liberty v0.46 patch decoded size mismatch")
subprocess.run(["patch", "--batch", "--forward", "-p1"],
               input=patch, cwd=ROOT, check=True)
for relative in ("Source/HybridEngineV046.h", "tests/hybrid_engine_v046_test.cpp",
                 "tests/hybrid_native_v046.cpp", "tests/test_v046_hybrid.py",
                 "docs/HYBRID_ENGINE_V046.md"):
    if not (ROOT / relative).is_file():
        raise SystemExit("Liberty v0.46 source missing: " + relative)
for relative in ("Source/PluginProcessor.cpp", "Source/PluginEditor.cpp",
                 "Source/HybridEngineV046.h"):
    if b"\xef\xbf\xbd" in (ROOT / relative).read_bytes():
        raise SystemExit("Liberty v0.46 invalid replacement characters: " + relative)
print("Liberty v0.46 hybrid VA / Wavetable / FM integration verified.")
