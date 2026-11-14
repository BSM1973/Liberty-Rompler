#!/usr/bin/env python3
"""Integrate checksum-verified v0.51 JUCE reuse and 16-page memory policy.
Run after v0.50 patches, off audio; never mutate sound banks or GUI.
"""
from pathlib import Path
import base64, hashlib, lzma, subprocess
ROOT=Path(__file__).resolve().parents[1]
path=ROOT/'assets/v051/source-patch.b64'
raw=base64.b64decode(''.join(path.read_text('ascii').split()),validate=True)
if len(raw)!=2536 or hashlib.sha256(raw).hexdigest()!='d869e4989c096384d3637cdec0ec493ac0edf889962e62ec46747a5985151435':
    raise SystemExit('Liberty v0.51 memory patch checksum mismatch')
diff=lzma.decompress(raw)
if len(diff)!=7436 or hashlib.sha256(diff).hexdigest()!='ef8460100e7c64b2493f943a169fb882afa4bc0277b695d7f9e90820fdcc30a2':
    raise SystemExit('Liberty v0.51 memory patch size/hash mismatch')
if not (ROOT/'Source/ResourcePolicyV051.h').is_file():
    raise SystemExit('Missing editable v0.51 resource policy')
subprocess.run(['patch','--batch','--forward','-p1'],input=diff,cwd=ROOT,check=True)
for p in ('Source/PluginProcessor.cpp','Source/PluginProcessor.h','Source/StreamedZoneV040.h'):
    if b'\xef\xbf\xbd' in (ROOT/p).read_bytes():
        raise SystemExit('Broken UTF-8 in '+p)
print('Liberty v0.51 reuse & memory limit verified')
