#!/usr/bin/env python3
"""Apply opt-in ORBIT v0.54 to v0.53, with source checksum and one-way patch."""
from pathlib import Path
import hashlib
import base64
import lzma
import subprocess
ROOT=Path(__file__).resolve().parents[1]
PATCH=ROOT/'assets/v054/orbit.patch.xz.b64'
packed=base64.b64decode(''.join(PATCH.read_text('ascii').split()),validate=True)
if hashlib.sha256(packed).hexdigest()!='b578517e9e47d67f60ab25491531c7cd53812496971db1dfb5b0e1a6fbf6d69d':
    raise SystemExit('Compressed v0.54 patch checksum mismatch')
data=lzma.decompress(packed)
EXPECTED='78d616a9967ed118ad7a150e0090203aac255316fde9f3204f05ef6c3512a181'
if hashlib.sha256(data).hexdigest()!=EXPECTED:
    raise SystemExit('Liberty ORBIT v0.54 patch checksum mismatch')
if not (ROOT/'Source/OrbitEngineV054.h').is_file():
    raise SystemExit('Missing independently editable ORBIT audio engine')
subprocess.run(['patch','--batch','--forward','-p1'],input=data,cwd=ROOT,check=True)
# Legacy v0.36 checks require ASCII-only JUCE source, while runtime French
# accents must remain visible. Encode É in u8 escapes after applying the patch.
ui=ROOT/'Source/LibertyMorphPadV036.h'
text=ui.read_text('utf8')
old_a='juce::TextButton gravityButton {"GRAVITÉ MIDI OFF"};'
new_a='juce::TextButton gravityButton {juce::String::fromUTF8(u8"GRAVIT\\u00c9 MIDI OFF")};'
old_b='"GRAVITÉ MIDI ON" : "GRAVITÉ MIDI OFF"'
new_b='juce::String::fromUTF8(u8"GRAVIT\\u00c9 MIDI ON") : juce::String::fromUTF8(u8"GRAVIT\\u00c9 MIDI OFF")'
if text.count(old_a)!=1 or text.count(old_b)!=1:
    raise SystemExit('Unexpected GRAVITÉ MIDI UI text patch baseline')
text=text.replace(old_a,new_a).replace(old_b,new_b)
if not text.isascii():
    raise SystemExit('v0.54 Morph UI source must remain ASCII-compatible')
ui.write_text(text,encoding='ascii')

for path in ('Source/PluginProcessor.cpp','Source/PluginProcessor.h',
             'Source/LibertyMorphPadV036.h'):
    if b'\xef\xbf\xbd' in (ROOT/path).read_bytes():
        raise SystemExit('Broken French UTF-8: '+path)
print('Liberty v0.54 ORBIT + INFINITY + CONSTELLATION + MIDI GRAVITY integrated.')
