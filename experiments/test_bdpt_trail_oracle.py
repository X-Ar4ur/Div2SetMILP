"""Regression tests for the lightweight BDPT local trail oracle."""

from __future__ import annotations

import shutil
import subprocess
import tempfile
import textwrap
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class BdptTrailOracleTests(unittest.TestCase):
    @unittest.skipIf(shutil.which("g++") is None, "g++ is required for this oracle test")
    def test_oracle_loads_trails_and_filters_outputs(self) -> None:
        source = textwrap.dedent(
            r"""
            #include "division/BdptTrailOracle.h"

            #include <iostream>
            #include <string>

            int main(int argc, char** argv) {
                if (argc != 3) return 10;
                BdptTrailOracle oracle;
                std::string error;
                if (!oracle.load(2, 2, argv[1], argv[2], error)) {
                    std::cerr << error << "\n";
                    return 1;
                }

                if (!oracle.allows(BdptTrailKind::K, 1, 1)) return 2;
                if (oracle.allows(BdptTrailKind::K, 1, 3)) return 3;
                if (!oracle.allows(BdptTrailKind::L, 1, 3)) return 4;
                if (oracle.possibleOutputs(BdptTrailKind::K, 1) != 1) return 5;
                if (oracle.possibleOutputs(BdptTrailKind::L, 1) != 3) return 6;
                if (oracle.inputSize() != 2 || oracle.outputSize() != 2) return 7;
                return 0;
            }
            """
        )

        with tempfile.TemporaryDirectory() as td:
            td_path = Path(td)
            k_file = td_path / "sbox_DivisionTrails.txt"
            l_file = td_path / "sbox_L_DivisionTrails.txt"
            k_file.write_text(
                "Division Trails of sbox:\n"
                "[0, 0, 0, 0]\n"
                "[1, 0, 1, 0]\n"
                "[0, 1, 0, 1]\n",
                encoding="utf-8",
            )
            l_file.write_text(
                "L-Division Trails of sbox:\n"
                "[0, 0, 0, 0]\n"
                "[1, 0, 1, 0]\n"
                "[1, 0, 1, 1]\n"
                "[0, 1, 0, 1]\n",
                encoding="utf-8",
            )
            test_cpp = td_path / "bdpt_trail_oracle_test.cpp"
            exe = td_path / "bdpt_trail_oracle_test.exe"
            test_cpp.write_text(source, encoding="utf-8")

            compile_cmd = [
                "g++",
                "-std=c++14",
                "-I",
                str(ROOT / "include"),
                str(test_cpp),
                str(ROOT / "lib" / "division" / "BdptTrailOracle.cpp"),
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
                [str(exe), str(k_file), str(l_file)],
                cwd=ROOT,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            self.assertEqual(completed.returncode, 0, msg=completed.stdout + completed.stderr)


if __name__ == "__main__":
    unittest.main()
