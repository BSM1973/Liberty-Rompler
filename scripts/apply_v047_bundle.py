#!/usr/bin/env python3
"""Apply checksum-verified v0.47 granular overlay after v0.46 restoration.

No generated JUCE source is committed to main. The compressed editable patch
script is kept in assets/v047, verified before use, then run once.
"""
from pathlib import Path
import base64
import hashlib
import lzma
import subprocess
import sys

ROOT=Path(__file__).resolve().parent.parent
source=ROOT/"assets/v047/apply-script.b64"
raw=base64.b64decode("".join(source.read_text("ascii").split()),validate=True)
if len(raw)!=3520 or hashlib.sha256(raw).hexdigest()!="811073204ae7a1c7a5f29ac59e5eaf13a469d1e35ef17267b86677c194659f93":
    raise SystemExit("Liberty v0.47 granular source integrity check failed")
script=lzma.decompress(raw)
if len(script)!=13863:
    raise SystemExit("Liberty v0.47 granular decoded size mismatch")
temporary=ROOT/"scripts/_integrate_v047_generated.py"
if temporary.exists():
    raise SystemExit("Refusing to replace existing v0.47 integration script")
try:
    temporary.write_bytes(script)
    subprocess.run([sys.executable,str(temporary)],cwd=ROOT,check=True)
finally:
    temporary.unlink(missing_ok=True)
for path in ("Source/GranularBlendV047.h","tests/granular_blend_v047_test.cpp",
             "tests/test_v047_granular.py","tests/granular_native_v047.cpp",
             "docs/HYBRID_GRANULAR_V047.md"):
    if not (ROOT/path).is_file():
        raise SystemExit("Liberty v0.47 missing source: "+path)
for path in ("Source/PluginProcessor.cpp","Source/SampleEngine.cpp","Source/PluginEditor.cpp"):
    if not (ROOT/path).read_bytes().isascii():
        raise SystemExit("Non-ASCII in runtime C++ source: "+path)
print("Liberty v0.47 native sample/grain engine integrated.")
