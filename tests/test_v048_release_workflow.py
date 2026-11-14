"""Guard release verifier/workflow consistency across future versions."""
from pathlib import Path
import re
import unittest

ROOT=Path(__file__).resolve().parents[1]
WF=ROOT/'.github/workflows/build-rompler.yml'

class ReleaseWorkflowV048(unittest.TestCase):
    def test_build_and_release_verifier_agree_on_version(self):
        w=WF.read_text(encoding='utf-8')
        match=re.search(r'target="dist/(Liberty-Rompler-macOS-arm64-UI-v(\d+)\.(\d+))"',w)
        self.assertIsNotNone(match,'Release target missing from workflow')
        target,major,minor=match.groups()
        version=f'v{major}{minor.zfill(2)}'
        verifier=f'scripts/verify_release_{version}.py'
        v=(ROOT/verifier).read_text(encoding='utf-8')
        self.assertIn(f'BASE = "{target}"',v)
        self.assertIn(f'git show HEAD:{verifier} > {verifier}',w)
        self.assertIn(f'python3 -m py_compile {verifier}',w)
        self.assertIn(f'python3 {verifier}',w)
        self.assertIn(f'dist/{target}.zip',w)
        self.assertIn(f'dist/{target}.dmg',w)
        self.assertIn(f'Liberty-Rompler-source-v{major}.{minor}.tar.gz',w)
