#!/usr/bin/env python3
"""Fix one typo in the v0.48 OFFLINE native test. Never touch DSP or plugin sources.

This is applied after the checksum-verified spectral archive during CI source
reconstruction. The original bundle's SHA-256 remains unchanged.
"""
from pathlib import Path

path=Path(__file__).resolve().parent.parent/"tests/spectral_native_v048.cpp"
source=path.read_text(encoding="utf-8")
wrong="peak=std::max(peak,std::max(std::abs(double(a)),std::abs(double(c)));"
right="peak=std::max(peak,std::max(std::abs(double(a)),std::abs(double(c))));"
if source.count(wrong)!=1:
    raise SystemExit("v0.48 native benchmark correction: unexpected source")
fixed=source.replace(wrong,right)
if fixed.count(right)!=1:
    raise SystemExit("v0.48 native benchmark correction failed")
path.write_text(fixed,encoding="utf-8")
print("v0.48 native spectral benchmark: one missing parenthesis corrected.")
