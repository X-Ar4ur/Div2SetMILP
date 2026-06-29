"""Regression tests for BDPT semantic Key-XOR scheduling."""

from __future__ import annotations

import shutil
import subprocess
import tempfile
import textwrap
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class BdptSemanticSchedulerTests(unittest.TestCase):
    @unittest.skipIf(shutil.which("g++") is None, "g++ is required for this scheduler test")
    def test_round_input_keyxor_skips_initial_layer(self) -> None:
        source = textwrap.dedent(
            r"""
            #include "division/BdptSemanticScheduler.h"

            #include <vector>

            int main() {
                std::vector<BdptKeyXorLayer> layers;
                for (int i = 0; i < 4; ++i) {
                    BdptKeyXorLayer layer;
                    layer.id = i;
                    layer.round = i + 1;
                    layer.beforeRoundCore = true;
                    layers.push_back(layer);
                }

                std::vector<BdptScheduledCrossLayer> scheduled =
                    scheduleBdptCrossLayers(layers, 4);

                if (scheduled.size() != 3) return 1;
                if (scheduled[0].modelNumber != 1 || scheduled[0].layer.id != 1) return 2;
                if (scheduled[1].modelNumber != 2 || scheduled[1].layer.id != 2) return 3;
                if (scheduled[2].modelNumber != 3 || scheduled[2].layer.id != 3) return 4;
                if (scheduled[0].feRoundsBeforeCross != 1) return 5;
                if (scheduled[2].feRoundsBeforeCross != 3) return 6;
                return 0;
            }
            """
        )
        self._compile_and_run(source)

    @unittest.skipIf(shutil.which("g++") is None, "g++ is required for this scheduler test")
    def test_round_output_keyxor_ignores_final_layer(self) -> None:
        source = textwrap.dedent(
            r"""
            #include "division/BdptSemanticScheduler.h"

            #include <vector>

            int main() {
                std::vector<BdptKeyXorLayer> layers;
                for (int i = 0; i < 4; ++i) {
                    BdptKeyXorLayer layer;
                    layer.id = i;
                    layer.round = i + 1;
                    layer.beforeRoundCore = false;
                    layers.push_back(layer);
                }

                std::vector<BdptScheduledCrossLayer> scheduled =
                    scheduleBdptCrossLayers(layers, 4);

                if (scheduled.size() != 3) return 1;
                if (scheduled[0].modelNumber != 1 || scheduled[0].layer.id != 0) return 2;
                if (scheduled[1].modelNumber != 2 || scheduled[1].layer.id != 1) return 3;
                if (scheduled[2].modelNumber != 3 || scheduled[2].layer.id != 2) return 4;
                if (scheduled[0].feRoundsBeforeCross != 1) return 5;
                if (scheduled[2].feRoundsBeforeCross != 3) return 6;
                return 0;
            }
            """
        )
        self._compile_and_run(source)

    def _compile_and_run(self, source: str) -> None:
        with tempfile.TemporaryDirectory() as td:
            test_cpp = Path(td) / "bdpt_scheduler_test.cpp"
            exe = Path(td) / "bdpt_scheduler_test.exe"
            test_cpp.write_text(source, encoding="utf-8")

            compile_cmd = [
                "g++",
                "-std=c++14",
                "-I",
                str(ROOT / "include"),
                "-I",
                str(ROOT / "include" / "preprocessing"),
                str(test_cpp),
                str(ROOT / "lib" / "division" / "BdptSemanticScheduler.cpp"),
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
            self.assertEqual(
                compiled.returncode,
                0,
                msg=compiled.stdout + compiled.stderr,
            )

            completed = subprocess.run(
                [str(exe)],
                cwd=ROOT,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            self.assertEqual(
                completed.returncode,
                0,
                msg=completed.stdout + completed.stderr,
            )


if __name__ == "__main__":
    unittest.main()
