"""Compile and run BDPT tests that do not require a Gurobi installation."""

from __future__ import annotations

import shutil
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
OUTPUT_DIR = ROOT / "tmp" / "no_gurobi_tests"


def compiler() -> str:
    found = shutil.which("g++")
    if found:
        return found
    windows_msys = Path(r"D:\Program Files\msys64\ucrt64\bin\g++.exe")
    if windows_msys.exists():
        return str(windows_msys)
    raise RuntimeError("g++ was not found")


def compile_and_run(name: str, sources: list[str]) -> None:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    executable = OUTPUT_DIR / (name + (".exe" if sys.platform == "win32" else ""))
    command = [
        compiler(),
        "-std=c++14",
        "-Iinclude",
        *sources,
        "-o",
        str(executable),
    ]
    subprocess.run(command, cwd=ROOT, check=True)
    subprocess.run([str(executable)], cwd=ROOT, check=True)


def main() -> int:
    compile_and_run(
        "test_bdpt_config",
        ["tests/test_bdpt_config.cpp", "lib/division/BdptConfig.cpp"],
    )
    compile_and_run(
        "test_bdpt_constraints",
        [
            "tests/test_bdpt_constraints.cpp",
            "lib/division/BdptMILPcons.cpp",
        ],
    )
    compile_and_run(
        "test_bdpt_solve_result",
        ["tests/test_bdpt_solve_result.cpp"],
    )
    subprocess.run(
        [
            sys.executable,
            "-m",
            "unittest",
            "tests.test_div2_isolation",
            "experiments.test_bdpt_repro",
            "experiments.test_run_bdpt_repro",
            "experiments.test_experiment_tables",
            "-v",
        ],
        cwd=ROOT,
        check=True,
    )
    print("All no-Gurobi BDPT tests passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
