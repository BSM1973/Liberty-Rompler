#!/usr/bin/env python3
"""Apply reviewed v0.52 interpolation patch after all v0.51 reconstruction.
The XZ bundle stores a unified diff of four source files; SHA-256 locked.
The native tests themselves remain independently editable and tracked.
"""
from pathlib import Path
import base64
import hashlib
import lzma
import subprocess

ROOT=Path(__file__).resolve().parents[1]
PACK=ROOT/'assets/v052/rt-interpolation.b64'
COMPRESSED=base64.b64decode(''.join(PACK.read_text('ascii').split()),validate=True)
if (len(COMPRESSED)!=2820 or
    hashlib.sha256(COMPRESSED).hexdigest()!='c791c299d917c0a4ad50acb3e50e0a7b685badbd4deb82116a4ae6cdc59a5c31'):
    raise SystemExit('Liberty v0.52 audio interpolation bundle checksum mismatch')
DIFF=lzma.decompress(COMPRESSED)
if (len(DIFF)!=8765 or
    hashlib.sha256(DIFF).hexdigest()!='e5a7fd53e5418c1a890c4eb3237761c62d562bbd8bbb0b3ab451943bbe966ec6'):
    raise SystemExit('Liberty v0.52 uncompressed audio patch checksum mismatch')
subprocess.run(['patch','--batch','--forward','-p1'],input=DIFF,cwd=ROOT,check=True)
for name in ('Source/StreamingPageCacheV039.h', 'Source/StreamedZoneV040.h',
             'Source/SampleEngine.cpp', 'tests/rompler_audio_stress_v044.cpp'):
    if b'\xef\xbf\xbd' in (ROOT/name).read_bytes():
        raise SystemExit('Invalid UTF-8 text in '+name)
print('Liberty v0.52 adjacent sample pin, RAM fast-path and benchmark CPU clock integrated')
