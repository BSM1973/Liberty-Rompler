#!/usr/bin/env python3
"""Verify and extract the editable v0.48 Spectral FFT sources into the CI build.

Run only after the v0.47 source overlays have been applied. The generated
JUCE .cpp files are changed solely inside the runner and never committed.
"""
from pathlib import Path
import base64
import hashlib
import io
import lzma
import subprocess
import sys
import tarfile

ROOT=Path(__file__).resolve().parents[1]
ARCHIVE=ROOT/"assets/v048/bundle.b64"
EXPECTED_SIZE=14932
EXPECTED_SHA="21bede894a97152420eddfb8ae3cd473ccbe2569d421d141768c72d2a092e1a1"
FILES={
    "Source/SpectralEngineV048.h",
    "tests/spectral_engine_v048_test.cpp",
    "tests/spectral_native_v048.cpp",
    "tests/test_v048_spectral.py",
    "tools/verify_spectral_v048.py",
    "docs/SPECTRAL_ENGINE_V048.md",
    "scripts/apply_v048_spectral.py",
}

binary=base64.b64decode("".join(ARCHIVE.read_text("ascii").split()),validate=True)
if len(binary)!=EXPECTED_SIZE or hashlib.sha256(binary).hexdigest()!=EXPECTED_SHA:
    raise SystemExit("Liberty v0.48 Spectral bundle checksum mismatch")
payload=lzma.decompress(binary)
if len(payload)!=61440: raise SystemExit("Liberty v0.48 unpacked bundle size mismatch")
with tarfile.open(fileobj=io.BytesIO(payload),mode="r:") as archive:
    names={item.name for item in archive}
    if names!=FILES or any(not item.isfile() for item in archive):
        raise SystemExit("Liberty v0.48 unexpected bundle paths")
    for entry in archive:
        path=ROOT/entry.name
        if path.exists():
            raise SystemExit("Refusing to overwrite pre-existing source: "+entry.name)
        path.parent.mkdir(parents=True,exist_ok=True)
        path.write_bytes(archive.extractfile(entry).read())
subprocess.run([sys.executable,str(ROOT/"scripts/apply_v048_spectral.py")],cwd=ROOT,check=True)
for p in ("Source/PluginProcessor.cpp","Source/PluginEditor.cpp",
          "Source/SampleEngine.cpp","Source/SpectralEngineV048.h"):
    if b"\xef\xbf\xbd" in (ROOT/p).read_bytes():
        raise SystemExit("Invalid Unicode replacement in "+p)
print("Liberty v0.48 spectral FFT, polyphonic resynthesis and all QA sources integrated")
