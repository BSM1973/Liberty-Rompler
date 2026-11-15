#!/usr/bin/env python3
"""Read-only inspect v0.48 bundled sources without running the JUCE overlay."""
import base64,io,lzma,tarfile,pathlib,re
root=pathlib.Path(__file__).resolve().parents[1]
blob=base64.b64decode("".join((root/"assets/v048/bundle.b64").read_text().split()))
with tarfile.open(fileobj=io.BytesIO(lzma.decompress(blob)),mode="r:") as arc:
    for name in ("Source/SpectralEngineV048.h","scripts/apply_v048_spectral.py","docs/SPECTRAL_ENGINE_V048.md"):
        raw=arc.extractfile(name).read().decode("utf-8")
        lines=raw.splitlines()
        print("====",name,"====")
        if name.endswith(".h"):
            for i,line in enumerate(lines,1):
                if i<=270: print(f"{i:03d} {line}")
        elif name.endswith(".py"):
            for i,line in enumerate(lines,1):
                if re.search(r"processBlock|spectral\.render|spectral\.prepare|setLatency|Spectral|prepareToPlay|render\(",line,re.I):
                    lo=max(0,i-3); hi=min(len(lines),i+4)
                    print(" | ".join(f"{j+1}:{lines[j]}" for j in range(lo,hi)))
        else:
            print("\n".join(lines[:110]))
