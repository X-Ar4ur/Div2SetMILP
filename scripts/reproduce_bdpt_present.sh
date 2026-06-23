#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${repo_root}"

python3 experiments/run_bdpt_repro.py \
  --binary build/EasyBC \
  --mode baseline \
  "$@"
