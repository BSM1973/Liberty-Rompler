"""Prevent distributing the wrong Liberty ROMpler release or using stale verifiers."""
from pathlib import Path
import unittest

ROOT=Path(__file__).resolve().parents[1]
WF=ROOT/".github/workflows/build-rompler.yml"
RELEASE=ROOT/"scripts/verify_release_v048.py"

class ReleaseWorkflowV048(unittest.TestCase):
    def test_build_and_release_verifier_agree_on_version(self):
        w=WF.read_text(encoding="utf-8")
        v=RELEASE.read_text(encoding="utf-8")
        target="Liberty-Rompler-macOS-arm64-UI-v0.48"
        verifier="scripts/verify_release_v048.py"
        self.assertIn(f'BASE = "{target}"',v)
        self.assertIn(f'target="dist/{target}"',w)
        self.assertIn(f'git show HEAD:{verifier} > {verifier}',w)
        self.assertIn(f'python3 -m py_compile {verifier}',w)
        self.assertIn(f'python3 {verifier}',w)
        self.assertNotIn("verify_release_v047.py",w)
        self.assertIn(f'dist/{target}.zip',w)
        self.assertIn(f'dist/{target}.dmg',w)
        self.assertIn("Liberty-Rompler-source-v0.48.tar.gz",w)
