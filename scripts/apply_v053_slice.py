#!/usr/bin/env python3
"""Apply v0.53 RAM-resident SLICE prefetch cadence to checked v0.52 source."""
from pathlib import Path
import hashlib
import subprocess
ROOT=Path(__file__).resolve().parents[1]
PATCH=ROOT/'assets/v053/source.patch'
raw=PATCH.read_bytes()
if (len(raw)!=2132 or
    hashlib.sha256(raw).hexdigest()!='baab84d74c126c383db733f7dd4eea08c29286390ac881036cccde1b8f5af8a5'):
    raise SystemExit('v0.53 source diff checksum mismatch')
assert (ROOT/'Source/SlicePrefetchCadenceV053.h').is_file()
subprocess.run(['patch','--batch','--forward','-p1'],input=raw,cwd=ROOT,check=True)
for p in ('Source/SampleEngine.cpp','Source/SampleEngine.h'):
    if b'\xef\xbf\xbd' in (ROOT/p).read_bytes():
        raise SystemExit('v0.53 invalid French UTF8: '+p)
print('v0.53 SLICE prefetch safe cadence applied (no changes to poly64 path)')
