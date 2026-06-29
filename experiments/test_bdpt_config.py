"""Regression tests for -div3 BDPT optional argument parsing."""

from __future__ import annotations

import shutil
import subprocess
import tempfile
import textwrap
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class BdptConfigParserTests(unittest.TestCase):
    @unittest.skipIf(shutil.which("g++") is None, "g++ is required for this parser test")
    def test_strict_reproduction_options_are_parseable(self) -> None:
        source = textwrap.dedent(
            r"""
            #include "division/BdptConfig.h"

            #include <iostream>
            #include <string>
            #include <vector>

            int main() {
                BdptRunConfig config;
                std::string error;
                std::vector<std::string> args = {
                    "cross", "paper",
                    "solver", "per-bit",
                    "sign", "1",
                    "repro", "1",
                    "timer", "42",
                    "threads", "3",
                };

                if (!parseBdptOptionalArgs(args, config, error)) {
                    std::cerr << error << "\n";
                    return 1;
                }
                if (config.crossMode != BdptCrossMode::Paper) return 2;
                if (config.unitSearchMode != BdptUnitSearchMode::PerBit) return 3;
                if (!config.signLabeling) return 4;
                if (!config.reproduction) return 5;
                if (config.timerSeconds != 42) return 6;
                if (config.threads != 3) return 7;
                return 0;
            }
            """
        )

        with tempfile.TemporaryDirectory() as td:
            test_cpp = Path(td) / "bdpt_config_test.cpp"
            exe = Path(td) / "bdpt_config_test.exe"
            test_cpp.write_text(source, encoding="utf-8")

            compile_cmd = [
                "g++",
                "-std=c++14",
                "-I",
                str(ROOT / "include"),
                str(test_cpp),
                str(ROOT / "lib" / "division" / "BdptConfig.cpp"),
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
