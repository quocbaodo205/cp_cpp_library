#!/usr/bin/env python3

from __future__ import annotations

import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
BUNDLER = ROOT / "tools" / "bundle.py"


class BundleTest(unittest.TestCase):
    def run_bundle(self, source_text: str) -> subprocess.CompletedProcess[str]:
        temporary_directory = tempfile.TemporaryDirectory()
        self.addCleanup(temporary_directory.cleanup)
        directory = Path(temporary_directory.name)
        source = directory / "main.cpp"
        output = directory / "submission.cpp"
        source.write_text(source_text, encoding="utf-8")

        return subprocess.run(
            [
                sys.executable,
                str(BUNDLER),
                "--source",
                str(source),
                "--strip-ifdef",
                "CP_VIZ",
                "--output",
                str(output),
            ],
            text=True,
            capture_output=True,
            check=False,
        )

    def test_strips_viz_block_and_nested_conditionals(self) -> None:
        result = self.run_bundle(
            """#include <vector>
#ifdef CP_VIZ
#include <cpviz/string.hpp>
#if INNER_DETAIL
void nested_visualization();
#endif
void visualize();
#endif
#ifdef KEEP_THIS
int local_only_value;
#endif
int main() { return 0; }
"""
        )
        self.assertEqual(result.returncode, 0, result.stderr)

        output_path = Path(result.stdout.split(" -> ", 1)[1].split(" (", 1)[0])
        bundled = output_path.read_text(encoding="utf-8")
        self.assertNotIn("CP_VIZ", bundled)
        self.assertNotIn("cpviz", bundled)
        self.assertNotIn("visualize", bundled)
        self.assertIn("#ifdef KEEP_THIS", bundled)
        self.assertIn("int main()", bundled)

    def test_rejects_else_on_stripped_block(self) -> None:
        result = self.run_bundle(
            """#ifdef CP_VIZ
void visualize();
#else
void solve();
#endif
"""
        )
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("containing #else or #elif", result.stderr)


if __name__ == "__main__":
    unittest.main()
