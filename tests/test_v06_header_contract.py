"""Prevent reintroduction of duplicate layer declarations in C++ headers."""
import re
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class LayerHeaderContract(unittest.TestCase):
    def test_unique_four_layer_structure(self):
        source = (ROOT / 'Source' / 'BankLibrary.h').read_text(encoding='utf-8')
        self.assertEqual(len(re.findall(r'\bstruct\s+LayerPresetDefinition\s*\{', source)), 1)
        self.assertEqual(len(re.findall(r'\bstd::vector\s*<\s*LayerPresetDefinition\s*>\s*layers\s*;', source)), 1)
        self.assertNotIn('\ufffd', source)


if __name__ == '__main__':
    unittest.main()
