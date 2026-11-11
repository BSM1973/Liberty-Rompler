#!/usr/bin/env python3
"""Vérifie les trois livrables et le contenu avant upload GitHub Actions."""
from pathlib import Path
import subprocess
import sys
import tarfile
import zipfile

ROOT = Path("dist")
BASE = "Liberty-Rompler-macOS-arm64-UI-v0.30"
DIRECTORY = ROOT / BASE
ZIP = ROOT / (BASE + ".zip")
DMG = ROOT / (BASE + ".dmg")
SOURCE = ROOT / "Liberty-Rompler-source-v0.30.tar.gz"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit("ERREUR distribution : " + message)
    print("OK : " + message)


def verify() -> None:
    files = [
        "Liberty Rompler.app",
        "Liberty Rompler.vst3",
        "Liberty Rompler.component",
        "INSTALLER_MAC.command",
        "DIAGNOSTIC_MAC.command",
        "GUIDE_TEST_BOUCLES_V014.md",
        "GUIDE_TEST_BOUCLES_V015.md",
        "GUIDE_TEST_BOUCLES_V016.md",
        "GUIDE_TEST_BOUCLES_V017.md",
        "GUIDE_TEST_BOUCLES_V018.md",
        "GUIDE_TEST_BOUCLES_V019.md",
        "GUIDE_TEST_BOUCLES_V020.md",
        "GUIDE_TEST_BOUCLES_V021.md",
        "GUIDE_TEST_BOUCLES_V022.md",
        "GUIDE_TEST_BOUCLES_V023.md",
        "GUIDE_TEST_BOUCLES_V024.md",
        "GUIDE_TEST_BOUCLES_V025.md",
        "GUIDE_TEST_BOUCLES_V026.md",
        "GUIDE_TEST_BOUCLES_V027.md",
        "GUIDE_TEST_BOUCLES_V028.md",
        "GUIDE_TEST_BOUCLES_V029.md",
        "GUIDE_TEST_BOUCLES_V030.md",
        "Banks/Factory-Starter/manifest.json",
        "Banks/Expansion-Demo/manifest.json",
    ]
    for relative in files:
        require((DIRECTORY / relative).exists(), "present " + relative)
    for archive in (ZIP, DMG, SOURCE):
        require(archive.is_file() and archive.stat().st_size > 0, "archive " + archive.name)

    for script in ("INSTALLER_MAC.command", "DIAGNOSTIC_MAC.command"):
        target = DIRECTORY / script
        subprocess.run(["bash", "-n", str(target)], check=True)
        require(target.stat().st_mode & 0o111 != 0, "executable " + script)

    for bundle in ("Liberty Rompler.app", "Liberty Rompler.vst3", "Liberty Rompler.component"):
        target = DIRECTORY / bundle
        subprocess.run(["codesign", "--verify", "--deep", str(target)], check=True)
        binary_dir = target / "Contents/MacOS"
        require(binary_dir.is_dir(), "executable dans " + bundle)
        binaries = [file for file in binary_dir.iterdir() if file.is_file()]
        require(bool(binaries), "binaire dans " + bundle)
        for binary in binaries:
            architectures = subprocess.check_output(["lipo", "-archs", str(binary)], text=True).split()
            require("arm64" in architectures, "arm64 " + bundle)

    with zipfile.ZipFile(ZIP) as archive:
        names = set(archive.namelist())
        for relative in files:
            expected = BASE + "/" + relative
            require(any(name == expected or name.startswith(expected + "/") for name in names),
                    "contenu ZIP " + relative)
        require(archive.testzip() is None, "integrite ZIP")

    with tarfile.open(SOURCE, "r:gz") as archive:
        names = set(archive.getnames())
        for name in ("Source/PluginProcessor.cpp", "Source/PluginProcessor.h",
                     "Source/SampleZoneView.h", "Source/PluginEditor.cpp",
                     "docs/SAMPLES_EDITOR_V014.md", "docs/GUIDE_TEST_BOUCLES_V014.md",
                     "Source/LoopEditorV015.h", "Source/LoopPreviewVoiceV015.h",
                     "Source/LoopFrameInputV015.h", "Source/LoopToolbarLayoutV015.h",
                     "Source/LoopPreviewWindowsV016.h", "Source/LoopSeamDifferenceV016.h",
                     "Source/LoopAuditionPlanV017.h", "Source/LoopSeamSearchV018.h",
                     "Source/LoopPairedSeamV019.h",
                     "Source/LoopPairedNudgeV020.h",
                     "Source/LoopDragV021.h",
                     "Source/LoopFocusV022.h",
                     "Source/LoopCursorZoomV023.h",
                     "Source/LoopPanV024.h",
                     "Source/LoopWheelPanV025.h",
                     "Source/LoopCenterViewV026.h",
                     "Source/LoopViewHistoryV027.h",
                     "Source/LoopMarkerFocusV028.h",
                     "Source/LoopMarkerDetailV029.h",
                     "Source/LoopTimingV030.h",
                     "docs/LOOP_EDITOR_V015.md", "docs/GUIDE_TEST_BOUCLES_V015.md",
                     "docs/PREVIEW_WINDOWS_V016.md", "docs/GUIDE_TEST_BOUCLES_V016.md",
                     "docs/LOOP_AUDITION_V017.md", "docs/GUIDE_TEST_BOUCLES_V017.md",
                     "docs/SMART_SEAM_V018.md", "docs/GUIDE_TEST_BOUCLES_V018.md",
                     "docs/PAIRED_LOOP_V019.md", "docs/GUIDE_TEST_BOUCLES_V019.md",
                     "docs/MANUAL_PAIRED_NUDGE_V020.md", "docs/GUIDE_TEST_BOUCLES_V020.md",
                     "docs/WAVEFORM_DRAG_V021.md", "docs/GUIDE_TEST_BOUCLES_V021.md",
                     "docs/LOOP_FOCUS_V022.md", "docs/GUIDE_TEST_BOUCLES_V022.md",
                     "docs/CURSOR_ZOOM_V023.md", "docs/GUIDE_TEST_BOUCLES_V023.md",
                     "docs/WAVEFORM_PAN_V024.md", "docs/GUIDE_TEST_BOUCLES_V024.md",
                     "docs/WHEEL_PAN_V025.md", "docs/GUIDE_TEST_BOUCLES_V025.md",
                     "docs/CENTER_VIEW_V026.md", "docs/GUIDE_TEST_BOUCLES_V026.md",
                     "docs/VIEW_HISTORY_V027.md", "docs/GUIDE_TEST_BOUCLES_V027.md",
                     "docs/LOOP_MARKER_FOCUS_V028.md", "docs/GUIDE_TEST_BOUCLES_V028.md",
                     "docs/MARKER_DETAIL_V029.md", "docs/GUIDE_TEST_BOUCLES_V029.md",
                     "docs/LOOP_TIMING_V030.md", "docs/GUIDE_TEST_BOUCLES_V030.md",
                     "scripts/apply_v017_audition.sh",
                     "scripts/install_mac.command", "scripts/DIAGNOSTIC_MAC.command"):
            require(name in names, "sources " + name)
    subprocess.run(["hdiutil", "verify", str(DMG)], check=True)
    require(True, "integrite DMG")
    print("Distribution v0.30 : toutes les verifications ont reussi.")


if __name__ == "__main__":
    try:
        verify()
    except (OSError, subprocess.CalledProcessError, zipfile.BadZipFile, tarfile.TarError) as exc:
        print("ERREUR distribution :", exc, file=sys.stderr)
        raise SystemExit(1) from exc
