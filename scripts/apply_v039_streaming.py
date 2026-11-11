#!/usr/bin/env python3
"""Liberty v0.39: materialize reviewed C++ streaming prototype only after v0.38.
Does not modify SampleVoice, engine behavior, presets or existing bank files.
"""
from pathlib import Path
import base64
import hashlib
import io
import lzma
import tarfile

ROOT = Path(__file__).resolve().parent.parent
ARCHIVE = ROOT / "assets/v039/streaming-sources.tar.xz.b64"
EXPECTED = "aff35f416c41c676c65c19a507ba9efe81f9f540d70114af30218e1f61e34a4b"
ALLOWED = {
    "Source/StreamingPageCacheV039.h",
    "Source/StreamingReadAheadV039.h",
    "Source/JuceStreamingReaderV039.h",
    "tests/streaming_page_cache_v039_test.cpp",
    "tests/streaming_worker_v039_test.cpp",
    "tests/streaming_mailbox_v039_test.cpp",
}
encoded = ARCHIVE.read_text(encoding="ascii").strip()
compressed = base64.b64decode(encoded, validate=True)
if hashlib.sha256(compressed).hexdigest() != EXPECTED:
    raise SystemExit("Liberty v0.39: incorrect source bundle SHA256")
with tarfile.open(fileobj=io.BytesIO(lzma.decompress(compressed)), mode="r:") as tar:
    members = tar.getmembers()
    if {m.name for m in members} != ALLOWED or any(not m.isfile() for m in members):
        raise SystemExit("Liberty v0.39: unexpected archive members")
    for member in members:
        path = ROOT / member.name
        if path.exists():
            if path.read_bytes() != tar.extractfile(member).read():
                raise SystemExit("Liberty v0.39: refusing to replace modified " + member.name)
            continue
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_bytes(tar.extractfile(member).read())
print("Liberty v0.39: verified C++ streaming source materialized; not connected to JUCE audio.")
