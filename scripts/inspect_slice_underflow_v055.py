#!/usr/bin/env python3
"""Read-only diagnostic of SLICE endpoint underflows in the rebuilt v0.54 engine."""
from pathlib import Path
tokens={
 "Source/SampleEngine.cpp": ("slicePrefetch.shouldRequest","reportStreamUnderflowBlock","warmEnd","readStereoAt","renderStreamedNextBlock"),
 "Source/StreamedZoneV040.h": ("tryReadFrame","requestStreamCritical","requestCritical","getTotalLength"),
 "tests/rompler_audio_stress_v044.cpp": ("underflow","scenario", "slice", "seconds","duration","processBlock"),
}
for name,terms in tokens.items():
    p=Path(name)
    if not p.exists():
        print("MISSING",name);continue
    lines=p.read_text("utf-8").splitlines()
    print("====",name,len(lines),"lines ====")
    shown=set()
    for term in terms:
        matches=[i for i,line in enumerate(lines) if term in line]
        print("TERM",term,"COUNT",len(matches))
        for i in matches[:8]:
            for j in range(max(0,i-7),min(len(lines),i+9)):
                if j not in shown:
                    print(f"{j+1:04d} {lines[j]}")
                    shown.add(j)

for name,a,b in (("Source/SampleEngine.cpp",433,568),("tests/rompler_audio_stress_v044.cpp",70,155)):
    lines=Path(name).read_text("utf-8").splitlines()
    print("==== FULL RANGE",name,a,b,"====")
    for i in range(a-1,min(b,len(lines))): print(f"{i+1:04d} {lines[i]}")
