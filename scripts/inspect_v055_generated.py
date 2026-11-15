#!/usr/bin/env python3
from pathlib import Path
p=Path("Source/PluginProcessor.cpp").read_text()
h=Path("Source/PluginProcessor.h").read_text()
for label,source,terms in [
    ("processor",p,["spectralParams.mix=", "morphLayerGain","orbitEngine.next", "morphXParam", "morphYParam", "for (int layer=","spectral.render("]),
    ("header",h,["morphLayerGain","morphXParam","morphYParam","spectralEngines","orbitEngine"]) ]:
    lines=source.splitlines()
    print("=== GENERATED "+label+" ===")
    seen=set()
    for term in terms:
        found=[i for i,x in enumerate(lines) if term in x]
        print("TERM",term,"MATCHES",len(found))
        for pos in found[:3]:
            for i in range(max(0,pos-4),min(len(lines),pos+5)):
                if i not in seen:
                    print(f"{i+1:04d} {lines[i]}")
                    seen.add(i)
