#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
gurobi_home="${GUROBI_HOME:-/home/xuws/gurobi1302/linux64}"
jobs="${JOBS:-$(nproc)}"

cd "${repo_root}"
python3 tests/run_no_gurobi_tests.py
cmake -S . -B build -DGUROBI_HOME="${gurobi_home}"
cmake --build build -j"${jobs}"

echo "Built ${repo_root}/build/EasyBC"
