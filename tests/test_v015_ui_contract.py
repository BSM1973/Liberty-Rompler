"""Static gate for Liberty ROMpler v0.15 JUCE UI integration components.

This does not replace a JUCE compilation or a visual inspection on macOS.
It prevents missing files, mangled French characters, or an incomplete
callback contract from silently entering the final paid macOS build.
"""
from __future__ import annotations

from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parent.parent
SOURCES = [
    "Source/LoopEditorV015.h",
    "Source/LoopFrameInputV015.h",
    "Source/LoopPreviewVoiceV015.h",
    "Source/LoopToolbarLayoutV015.h",
    "Source/LoopEditorPanelV015.h",
    "Source/LoopFadeOverlayV015.h",
]


class LoopEditorUiContractV015(unittest.TestCase):
    def test_utf8_files_are_valid_and_not_corrupted(self):
        for name in SOURCES:
            with self.subTest(name=name):
                data = (ROOT / name).read_bytes()
                text = data.decode("utf-8", errors="strict")
                self.assertNotIn("\ufffd", text)
                self.assertNotIn("\u25a1", text)
                self.assertNotIn("\x00", text)

    def test_juce_panel_exposes_host_owned_audio_callbacks(self):
        text = (ROOT / "Source/LoopEditorPanelV015.h").read_text(encoding="utf-8")
        for expected in (
            "class LoopEditorPanelV015",
            "onPreviewRequested",
            "onApplyRequested",
            "onPendingChanged",
            "pending.cancelPending()",
            "pending.markApplied()",
            "makeLoopToolbarLayout",
            "setInputRestrictions",
            r'fromUTF8 (u8"D\u00c9BUT")',
            r'fromUTF8 (u8"PR\u00c9\u00c9COUTE")',
        ):
            with self.subTest(expected=expected):
                self.assertIn(expected, text)
        self.assertNotIn("AudioDeviceManager", text)
        self.assertNotIn("AudioTransportSource", text)

    def test_noninteractive_fade_overlay(self):
        text = (ROOT / "Source/LoopFadeOverlayV015.h").read_text(encoding="utf-8")
        for expected in (
            "setInterceptsMouseClicks (false, false)",
            "crossfadeBands (loop, fadeFrames, visible)",
            "getWidth()",
            "getHeight()",
        ):
            self.assertIn(expected, text)

    def test_renderer_is_off_audio_thread(self):
        math = (ROOT / "Source/LoopEditorV015.h").read_text(encoding="utf-8")
        voice = (ROOT / "Source/LoopPreviewVoiceV015.h").read_text(encoding="utf-8")
        self.assertIn("renderSeamPreview", math)
        self.assertIn("Call prepare()", voice)
        self.assertIn("void process (float* const* outputs", voice)


if __name__ == "__main__":
    unittest.main()
