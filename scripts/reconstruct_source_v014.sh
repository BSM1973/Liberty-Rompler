#!/usr/bin/env bash
set -euo pipefail
# GitHub-only reconstruction of the v0.14 source snapshot; NO compiler invoked.
# Extracted verbatim from build-rompler.yml, except Linux base64 -d.

echo '=== Verify and unpack prototype source ==='
set -euo pipefail
echo '0da09c77c326780c5f972a45984be0f345ff3a39639c340192e04c2a33476e9e  assets/rompler-source-v0.1.tar.xz' | shasum -a 256 --check
tar -xJf assets/rompler-source-v0.1.tar.xz
test -f CMakeLists.txt
test -f Source/PluginProcessor.cpp
test -f tools/make_demo_banks.py

echo '=== Apply new Nexus-style Liberty UI, audio filter and regression checks ==='
set -euo pipefail
cat assets/ui-v02/part-{1..5}.b64 | base64 -d > /tmp/liberty-ui-v02.tar.xz
echo '3cfe2791987fd1a9e4b51abb60696b24b24da4ef09d45a9ce93e59652d9fe061  /tmp/liberty-ui-v02.tar.xz' | shasum -a 256 --check
tar -xJf /tmp/liberty-ui-v02.tar.xz
test -f tests/test_ui_regression.py
test -f Source/PluginEditor.cpp
test -f Source/SampleEngine.h

echo '=== Apply UI v0.3 tabs, effects, modulation and full-width keyboard ==='
set -euo pipefail
cat assets/ui-v03/part-{1..4}.b64 | base64 -d > /tmp/liberty-ui-v03.patch.xz
echo '2048ee0ec90574a07c52fa738a907f7e1570cbf9f373e9305aa0ee6147041c94  /tmp/liberty-ui-v03.patch.xz' | shasum -a 256 --check
xz -dc /tmp/liberty-ui-v03.patch.xz > /tmp/liberty-ui-v03.patch
patch -p1 < /tmp/liberty-ui-v03.patch
test -f Source/PluginEditor.cpp
test -f Source/PluginProcessor.cpp
test -f tests/test_ui_regression.py

echo '=== Apply v0.4 Delay Distortion + full French-text fixes ==='
set -euo pipefail
cat assets/ui-v04/part-{1..3}.b64 | base64 -d > /tmp/liberty-ui-v04.patch.xz
echo '37fce78e5a9eb0dfdb4c6106b25540ad26f1e5f93276ed51d467f47e07950d1d  /tmp/liberty-ui-v04.patch.xz' | shasum -a 256 --check
xz -dc /tmp/liberty-ui-v04.patch.xz > /tmp/liberty-ui-v04.patch
patch -p1 < /tmp/liberty-ui-v04.patch
test -f tools/audit_ui_text.py
test -f tests/test_ui_v04.py

echo '=== Apply v0.5 modulators, three-band EQ and compressor ==='
set -euo pipefail
cat assets/ui-v05/part-{1..5}.b64 | base64 -d > /tmp/liberty-ui-v05.patch.xz
echo '103afbbefa19e00f88128e6f21d2980dd6cd7862f410ca09f4987d2cf683fc57  /tmp/liberty-ui-v05.patch.xz' | shasum -a 256 --check
xz -dc /tmp/liberty-ui-v05.patch.xz > /tmp/liberty-ui-v05.patch
patch -p1 < /tmp/liberty-ui-v05.patch
test -f tests/test_ui_v05.py
test -f Source/PluginEditor.cpp
test -f Source/PluginProcessor.cpp

echo '=== Apply v0.6 four-layer multisample engine and UI ==='
set -euo pipefail
cat assets/ui-v06/part-{1..6}.b64 | base64 -d > /tmp/liberty-ui-v06.patch.xz
echo 'fdea4e597422beba2f9dbf2a902e53a81e204d579dca786211d1583c8dfd1f1a  /tmp/liberty-ui-v06.patch.xz' | shasum -a 256 --check
xz -dc /tmp/liberty-ui-v06.patch.xz > /tmp/liberty-ui-v06.patch
patch -p1 < /tmp/liberty-ui-v06.patch
python3 scripts/fix_v06_header.py
test -f tests/test_layers_v06.py
test -f Source/SampleEngine.cpp
test -f Source/BankLibrary.cpp

echo '=== Apply v0.7 graphical routing and real per-layer effect buses ==='
set -euo pipefail
cat assets/ui-v07/part-{1..5}.b64 | base64 -d > /tmp/liberty-ui-v07.patch.xz
echo '8dc6ce8ec8633453f4f06f27757f4613898a17c7f83166c6f3011cb5f4c61e8d  /tmp/liberty-ui-v07.patch.xz' | shasum -a 256 --check
xz -dc /tmp/liberty-ui-v07.patch.xz > /tmp/liberty-ui-v07.patch
patch --batch -p1 < /tmp/liberty-ui-v07.patch
test -f Source/RoutingDSP.h
test -f Source/RoutingGraph.h
test -f tests/test_routing_v07.py

echo '=== Apply v0.8 arpeggiator MIDI scheduler and 32-step editor ==='
set -euo pipefail
cat assets/ui-v08/part-{1..6}.b64 | base64 -d > /tmp/liberty-ui-v08.patch.xz
echo '8eaf16d5c4366d8442c555e00085dfdde3866353b55a7a96c9f3780f0ffd279a  /tmp/liberty-ui-v08.patch.xz' | shasum -a 256 --check
xz -dc /tmp/liberty-ui-v08.patch.xz > /tmp/liberty-ui-v08.patch
patch --batch -p1 < /tmp/liberty-ui-v08.patch
test -f Source/Arpeggiator.h
test -f Source/ArpStepGrid.h
test -f tests/test_arp_v08.py
test -f tests/arp_midi_test.cpp

echo '=== Apply v0.9 JUCE 8 MIDI fix, pattern models and editor tools ==='
set -euo pipefail
cat assets/ui-v09/part-{1..4}.b64 | base64 -d > /tmp/liberty-ui-v09.patch.xz
echo '36d7ad16a83946851159444f5f874fcf336f7271ccb155349bd3a17f64660f7e  /tmp/liberty-ui-v09.patch.xz' | shasum -a 256 --check
xz -dc /tmp/liberty-ui-v09.patch.xz | patch --batch -p1
test -f Source/ArpPatternTools.h
test -f Source/ArpStepGrid.h
test -f tests/test_arp_v09.py
test -f tests/arp_pattern_test.cpp

echo '=== Apply v0.10 nine-tab navigation and multisample waveform viewer ==='
set -euo pipefail
cat assets/ui-v10/part-{1..3}.b64 | base64 -d > /tmp/liberty-ui-v10-overlay.tar.xz
echo '4067f8c49c6c5d2019cb61628eddaa0a8bea1785b17621c9f48dc3a2ac25c0f5  /tmp/liberty-ui-v10-overlay.tar.xz' | shasum -a 256 --check
tar -xJf /tmp/liberty-ui-v10-overlay.tar.xz
python3 scripts/upgrade_ui_v10.py
test -f Source/SampleZoneView.h
test -f tests/test_ui_v10.py

echo '=== Apply v0.11 editable key and velocity maps with non-destructive sample loops ==='
set -euo pipefail
cat assets/ui-v11/part-{1..6}.b64 | base64 -d > /tmp/liberty-ui-v11.patch.xz
echo '3245e4b8b71fb8931eb91a245be0f0688a2bc6c7b3cea2a6d47f2b881c272fbc  /tmp/liberty-ui-v11.patch.xz' | shasum -a 256 --check
xz -dc /tmp/liberty-ui-v11.patch.xz | patch --batch -p1
python3 -c 'from pathlib import Path; p=Path("Source/PluginEditor.cpp"); s=p.read_text(); old="Prototype 0.10  |"; assert s.count(old)==1; p.write_text(s.replace(old, "Prototype 0.11  |"))'
test -f Source/SampleZoneMath.h
test -f tests/sample_zone_math_test.cpp
test -f tests/test_samples_v011.py
test -f docs/SAMPLES_EDITOR_V011.md

echo '=== Apply v0.12 real-time loop crossfade and draggable waveform markers ==='
set -euo pipefail
cat assets/ui-v12/part-{1..4}.b64 | base64 -d > /tmp/liberty-ui-v12.patch.xz
echo '66352c5b1fc793343194754f772af3a8b74bc8350955e001c4bbcfb216939369  /tmp/liberty-ui-v12.patch.xz' | shasum -a 256 --check
xz -dc /tmp/liberty-ui-v12.patch.xz | patch --batch -p1
python3 -c 'from pathlib import Path; p=Path("Source/PluginEditor.cpp"); s=p.read_text(); old="Prototype 0.11  |"; assert s.count(old)==1; p.write_text(s.replace(old, "Prototype 0.12  |"))'
test -f Source/LoopCrossfade.h
test -f tests/loop_crossfade_test.cpp
test -f tests/test_samples_v012.py
test -f docs/SAMPLES_EDITOR_V012.md

echo '=== Apply v0.13 zero crossing loop snapping and pending edit undo ==='
set -euo pipefail
cat assets/ui-v13/part-{1..3}.b64 | base64 -d > /tmp/liberty-ui-v13.patch.xz
echo '23be410ac83f6d2c3ef99a0daf8bb93c9f1d714e4844cd6ac01bbbcdcc65e287  /tmp/liberty-ui-v13.patch.xz' | shasum -a 256 --check
xz -dc /tmp/liberty-ui-v13.patch.xz | patch --batch -p1
python3 -c 'from pathlib import Path; p=Path("Source/PluginEditor.cpp"); s=p.read_text(); old="Prototype 0.12  |"; assert s.count(old)==1; p.write_text(s.replace(old, "Prototype 0.13  |"))'
test -f Source/ZeroCrossSnap.h
test -f tests/test_samples_v013.py
test -f tests/zero_cross_snap_test.cpp
test -f docs/SAMPLES_EDITOR_V013.md

echo '=== Apply v0.14 waveform zoom, navigation and automatic marker zero snap ==='
set -euo pipefail
cat assets/ui-v14/part-{1..4}.b64 | base64 -d > /tmp/liberty-ui-v14.patch.xz
echo '35cb670447a3214f4a8d12a4cec4f3cf0ad43e657a622ff9d445adf733dd834b  /tmp/liberty-ui-v14.patch.xz' | shasum -a 256 --check
xz -dc /tmp/liberty-ui-v14.patch.xz | patch --batch -p1
python3 -c 'from pathlib import Path; p=Path("Source/PluginEditor.cpp"); s=p.read_text(); old="Prototype 0.13  |"; assert s.count(old)==1; p.write_text(s.replace(old, "Prototype 0.14  |"))'
test -f Source/WaveformZoom.h
test -f tests/test_samples_v014.py
test -f tests/waveform_zoom_test.cpp
test -f docs/SAMPLES_EDITOR_V014.md

test -f Source/PluginEditor.cpp
test -f Source/PluginProcessor.cpp
test -f Source/LoopEditorV015.h
printf 'v0.14 reconstructed successfully; ready for offline v0.15 integration.\n'
