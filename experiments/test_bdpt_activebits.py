"""Regression tests for BDPT active-bit presets."""

from __future__ import annotations

import shutil
import subprocess
import tempfile
import textwrap
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class BdptActiveBitsTests(unittest.TestCase):
    @unittest.skipIf(shutil.which("g++") is None, "g++ is required for this active-bit test")
    def test_rectangle_63_matches_appendix_f10_plaintext_layout(self) -> None:
        source = textwrap.dedent(
            r"""
            #include "division/BdptActiveBits.h"

            #include <algorithm>
            #include <vector>

            int main() {
                const std::vector<int> active =
                    resolveBdptActiveBitVars("Rectangle", 64, "63");

                if (active.size() != 63) return 1;
                if (std::find(active.begin(), active.end(), 16) != active.end()) return 2;
                for (int i = 1; i <= 64; ++i) {
                    if (i == 16) continue;
                    if (std::find(active.begin(), active.end(), i) == active.end()) return 3;
                }
                return 0;
            }
            """
        )
        self._compile_and_run(source)

    def _compile_and_run(self, source: str) -> None:
        with tempfile.TemporaryDirectory() as td:
            test_cpp = Path(td) / "bdpt_activebits_test.cpp"
            exe = Path(td) / "bdpt_activebits_test.exe"
            test_cpp.write_text(source, encoding="utf-8")

            compile_cmd = [
                "g++",
                "-std=c++14",
                "-I",
                str(ROOT / "include"),
                str(test_cpp),
                str(ROOT / "lib" / "division" / "BdptActiveBits.cpp"),
                "-o",
                str(exe),
            ]
            compiled = subprocess.run(
                compile_cmd,
                cwd=ROOT,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            self.assertEqual(compiled.returncode, 0, msg=compiled.stdout + compiled.stderr)

            completed = subprocess.run(
                [str(exe)],
                cwd=ROOT,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            self.assertEqual(completed.returncode, 0, msg=completed.stdout + completed.stderr)


if __name__ == "__main__":
    unittest.main()
