#!/usr/bin/env python3
"""
EasyBC 2-subset division-property benchmark runner.

Reads a YAML matrix from configs/, runs ./EasyBC -div for each cell
the configured number of times, parses the structured [BENCH] lines
that the patched C++ writes to stderr, and emits a CSV.

Output columns are stable so make_tables.py can join across runs.

Usage
-----
    python bench.py --config configs/correctness.yaml
    python bench.py --config configs/perf.yaml --repeat 5
    python bench.py --config configs/reduction.yaml --threads 1

Implementation notes
--------------------
- The EasyBC binary is expected at ../build/EasyBC
  (override via --binary or the EASYBC_BINARY env var).
- Working dir for the subprocess is the repo's `build/`
  directory because EasyBC reads `../benchmarks/...` and writes
  `../data/...` relative to its CWD.
- We capture full stderr and persist it to results/logs/<run_id>.log so
  one-off failures can be diagnosed after the fact.
- CSV is appended; bench.py never overwrites a prior run.
"""

from __future__ import annotations

import argparse
import csv
import datetime as _dt
import json
import os
import re
import shlex
import statistics
import subprocess
import sys
import time
from pathlib import Path
from typing import Any, Iterable

try:
    import yaml  # type: ignore
except ImportError:
    print("ERROR: PyYAML required. pip install pyyaml", file=sys.stderr)
    sys.exit(2)


HERE = Path(__file__).resolve().parent
REPO_ROOT = HERE.parent
DEFAULT_BINARY = REPO_ROOT / "build" / "EasyBC"
DEFAULT_CWD = REPO_ROOT / "build"
RESULTS_DIR = HERE / "results"
LOG_DIR = RESULTS_DIR / "logs"

BENCH_LINE_RE = re.compile(r"^\[BENCH\]\s+(.*)$")
KV_RE = re.compile(r"(\w+)=([^\s]+)")

VAR_RE = re.compile(r"x\d+")


# --------------------------------------------------------------------------
# YAML loading
# --------------------------------------------------------------------------


def load_config(path: Path) -> dict[str, Any]:
    with open(path) as f:
        cfg = yaml.safe_load(f)
    if not isinstance(cfg, dict) or "runs" not in cfg:
        raise SystemExit(f"{path}: missing top-level 'runs' list")
    cfg.setdefault("defaults", {})
    return cfg


def expand_runs(cfg: dict[str, Any]) -> list[dict[str, Any]]:
    """Cross-product the run list with optional sweep dimensions.

    Sweeps recognized in `defaults`:
      reduction_sweep : list[int]   (overrides per-run `reduction`)
      threads_sweep   : list[int]   (overrides per-run `threads`)
      rounds_sweep    : list[int]   (overrides per-run `rounds`)

    A run may also specify `round_start` and `round_end` to expand an
    inclusive round range, which is useful for boundary-sweep tables.
    """
    defaults = cfg.get("defaults", {})
    base_reductions: list[int] = defaults.get("reduction_sweep") or [
        defaults.get("reduction", 1)
    ]
    base_threads: list[int] = defaults.get("threads_sweep") or [
        defaults.get("threads", 8)
    ]
    repeat: int = int(defaults.get("repeat", 1))
    timer_sec: int = int(defaults.get("timer_sec", 86400))

    expanded = []
    for run in cfg["runs"]:
        cipher = run["cipher"]
        if "rounds_sweep" in run:
            per_rounds = [int(r) for r in run["rounds_sweep"]]
        elif "round_start" in run and "round_end" in run:
            start = int(run["round_start"])
            end = int(run["round_end"])
            if end < start:
                raise SystemExit(f"{cipher}: round_end must be >= round_start")
            per_rounds = list(range(start, end + 1))
        elif "rounds_sweep" in defaults:
            per_rounds = [int(r) for r in defaults["rounds_sweep"]]
        else:
            per_rounds = [int(run["rounds"])]
        activebits = str(run["activebits"])
        per_reds = [run["reduction"]] if "reduction" in run else base_reductions
        per_thrs = [run["threads"]] if "threads" in run else base_threads
        for rounds in per_rounds:
            for red in per_reds:
                for thr in per_thrs:
                    extras = {
                        k: v for k, v in run.items() if k not in
                        ("cipher", "rounds", "rounds_sweep", "round_start",
                         "round_end", "activebits", "reduction", "threads")
                    }
                    extras.setdefault("published_round", run.get("rounds", ""))
                    expanded.append(
                        {
                            "cipher": cipher,
                            "rounds": rounds,
                            "activebits": activebits,
                            "reduction": int(red),
                            "threads": int(thr),
                            "repeat": repeat,
                            "timer_sec": timer_sec,
                            "extras": extras,
                        }
                    )
    return expanded


# --------------------------------------------------------------------------
# Subprocess execution
# --------------------------------------------------------------------------


def run_easybc(
    binary: Path, cwd: Path, cipher: str, reduction: int, rounds: int,
    activebits: str, timer_sec: int, threads: int,
) -> tuple[int, str, str]:
    cmd = [
        str(binary), "-div", cipher, str(reduction), str(rounds), activebits,
        "timer", str(timer_sec), "threads", str(threads),
    ]
    proc = subprocess.run(
        cmd, cwd=str(cwd), capture_output=True, text=True, check=False
    )
    return proc.returncode, proc.stdout, proc.stderr


def parse_bench_lines(stderr: str) -> dict[str, dict[str, str]]:
    """Group [BENCH] lines by phase. Last occurrence wins on duplicate phase."""
    phases: dict[str, dict[str, str]] = {}
    for line in stderr.splitlines():
        m = BENCH_LINE_RE.match(line)
        if not m:
            continue
        kvs = dict(KV_RE.findall(m.group(1)))
        phase = kvs.pop("phase", "config" if "config" in m.group(1) else "")
        if not phase:
            # The "config" header line uses no `phase=` key.
            tokens = m.group(1).split()
            if tokens and tokens[0] == "config":
                phase = "config"
                kvs = dict(KV_RE.findall(" ".join(tokens[1:])))
        if phase:
            phases[phase] = kvs
    # Sum trail/ineq phases across multiple S-boxes (e.g. LBlock has 10).
    return phases


def parse_balanced_bits(result_path: Path) -> list[str]:
    """Read the *latest* result_*.txt and return the balanced-bit names."""
    return parse_result_file(result_path)["balanced_bits"]


def natural_var_key(name: str) -> tuple[str, int]:
    m = re.match(r"([a-zA-Z]+)(\d+)$", name)
    if not m:
        return name, 0
    return m.group(1), int(m.group(2))


def sorted_vars(values: Iterable[str]) -> list[str]:
    return sorted(set(values), key=natural_var_key)


def vars_from_line(line: str) -> list[str]:
    return VAR_RE.findall(line)


def parse_result_file(result_path: Path) -> dict[str, list[str]]:
    if not result_path.exists():
        return {"output_bits": [], "set_zero": [], "balanced_bits": []}
    output_bits: list[str] = []
    set_zero: list[str] = []
    listed_balanced: list[str] = []
    in_set_zero_block = False
    for raw in result_path.read_text().splitlines():
        line = raw.strip()
        if not line:
            in_set_zero_block = False
            continue
        lower = line.lower()
        if lower.startswith("output bits:"):
            output_bits.extend(vars_from_line(line))
            in_set_zero_block = False
        elif lower.startswith("set zero:"):
            set_zero.extend(vars_from_line(line))
            in_set_zero_block = False
        elif lower.startswith("balanced bits:"):
            listed_balanced.extend(vars_from_line(line))
            in_set_zero_block = False
        elif lower.startswith("those are the coordinates set to zero"):
            in_set_zero_block = True
        elif lower.startswith("time used") or lower.startswith("integral"):
            in_set_zero_block = False
        elif in_set_zero_block:
            set_zero.extend(vars_from_line(line))
    if output_bits:
        balanced = set(output_bits) - set(set_zero)
    else:
        balanced = set(listed_balanced)
    return {
        "output_bits": sorted_vars(output_bits),
        "set_zero": sorted_vars(set_zero),
        "balanced_bits": sorted_vars(balanced),
    }


def result_path_for(cipher: str, rounds: int, activebits: str) -> Path:
    return (
        REPO_ROOT / "data" / "division" / cipher / "milp"
        / f"result_{rounds}_{safe_activebits_id(activebits)}.txt"
    )


def safe_activebits_id(activebits: str) -> str:
    """Return the file-name token used by Div2SetMILP for activebitsSpec."""
    return re.sub(r"[^A-Za-z0-9_.-]", "_", str(activebits))


# --------------------------------------------------------------------------
# CSV emission
# --------------------------------------------------------------------------


CSV_FIELDS = [
    "run_id", "ts", "cipher", "rounds", "activebits", "reduction", "threads",
    "trial", "paper_ref", "slice",
    "trail_ms_total", "trail_n_trails_total",
    "ineq_gen_ms_total", "ineq_gen_n_total",
    "reduce_ms_total", "reduce_n_before_total", "reduce_n_after_total",
    "preprocess_ms",
    "build_ms", "n_xvars", "n_dvars", "block_size",
    "model_load_ms", "n_vars", "n_cons",
    "solve_ms", "total_ms",
    "gurobi_status", "distinguisher_found", "n_zero_coords", "n_iter",
    "output_bits", "set_zero", "balanced_bits", "n_balanced",
    "wall_sec_proc", "exit_code",
]


def aggregate_phase(stderr: str, phase: str, fields: Iterable[str]) -> dict[str, int]:
    """Sum numeric fields across all occurrences of a given phase line."""
    sums: dict[str, int] = {f: 0 for f in fields}
    count = 0
    for line in stderr.splitlines():
        m = BENCH_LINE_RE.match(line)
        if not m:
            continue
        kvs = dict(KV_RE.findall(m.group(1)))
        if kvs.get("phase") != phase:
            continue
        count += 1
        for f in fields:
            try:
                sums[f] += int(kvs.get(f, "0"))
            except ValueError:
                pass
    sums["_count"] = count
    return sums


def row_from_run(
    run_id: str, expanded_run: dict[str, Any], trial: int, ts_iso: str,
    stderr: str, exit_code: int, wall_sec: float,
) -> dict[str, Any]:
    phases = parse_bench_lines(stderr)
    trail_agg = aggregate_phase(stderr, "trail", ["elapsed_ms", "n_trails"])
    ineq_agg = aggregate_phase(stderr, "ineq_gen", ["elapsed_ms", "n_ineq"])
    red_agg = aggregate_phase(
        stderr, "reduce", ["elapsed_ms", "n_ineq_before", "n_ineq_after"]
    )

    pre = phases.get("preprocess", {})
    build = phases.get("build", {})
    load = phases.get("model_load", {})
    solve = phases.get("solve", {})

    return {
        "run_id": run_id,
        "ts": ts_iso,
        "cipher": expanded_run["cipher"],
        "rounds": expanded_run["rounds"],
        "activebits": expanded_run["activebits"],
        "reduction": expanded_run["reduction"],
        "threads": expanded_run["threads"],
        "trial": trial,
        "paper_ref": expanded_run.get("extras", {}).get("paper_ref", ""),
        "slice": expanded_run.get("extras", {}).get("slice", ""),
        "trail_ms_total": trail_agg["elapsed_ms"],
        "trail_n_trails_total": trail_agg["n_trails"],
        "ineq_gen_ms_total": ineq_agg["elapsed_ms"],
        "ineq_gen_n_total": ineq_agg["n_ineq"],
        "reduce_ms_total": red_agg["elapsed_ms"],
        "reduce_n_before_total": red_agg["n_ineq_before"],
        "reduce_n_after_total": red_agg["n_ineq_after"],
        "preprocess_ms": pre.get("elapsed_ms", ""),
        "build_ms": build.get("elapsed_ms", ""),
        "n_xvars": build.get("n_xvars", ""),
        "n_dvars": build.get("n_dvars", ""),
        "block_size": build.get("block_size", ""),
        "model_load_ms": load.get("elapsed_ms", ""),
        "n_vars": load.get("n_vars", ""),
        "n_cons": load.get("n_cons", ""),
        "solve_ms": solve.get("elapsed_ms", ""),
        "total_ms": solve.get("total_ms", ""),
        "gurobi_status": solve.get("gurobi_status", ""),
        "distinguisher_found": solve.get("distinguisher_found", ""),
        "n_zero_coords": solve.get("n_zero_coords", ""),
        "n_iter": solve.get("n_iter", ""),
        "wall_sec_proc": f"{wall_sec:.3f}",
        "exit_code": exit_code,
    }


# --------------------------------------------------------------------------
# Main loop
# --------------------------------------------------------------------------


def main() -> int:
    ap = argparse.ArgumentParser(description="EasyBC 2-subset BDP bench runner")
    ap.add_argument("--config", required=True, type=Path,
                    help="YAML config under experiments/configs/")
    ap.add_argument("--binary", type=Path,
                    default=Path(os.environ.get("EASYBC_BINARY", DEFAULT_BINARY)),
                    help="Path to EasyBC binary")
    ap.add_argument("--cwd", type=Path, default=DEFAULT_CWD,
                    help="Working directory for EasyBC subprocess")
    ap.add_argument("--repeat", type=int, default=None,
                    help="Override repeat from config")
    ap.add_argument("--threads", type=int, default=None,
                    help="Pin a single thread count, ignoring threads_sweep")
    ap.add_argument("--out", type=Path, default=None,
                    help="Output CSV path (default: results/<config>_<ts>.csv)")
    ap.add_argument("--dry-run", action="store_true",
                    help="Print commands without executing")
    args = ap.parse_args()

    if not args.binary.exists() and not args.dry_run:
        print(f"ERROR: EasyBC binary not found at {args.binary}", file=sys.stderr)
        print("       Build with cmake first, or set EASYBC_BINARY.", file=sys.stderr)
        return 2

    cfg = load_config(args.config)
    runs = expand_runs(cfg)
    if args.repeat is not None:
        for r in runs:
            r["repeat"] = args.repeat
    if args.threads is not None:
        for r in runs:
            r["threads"] = args.threads

    LOG_DIR.mkdir(parents=True, exist_ok=True)
    RESULTS_DIR.mkdir(parents=True, exist_ok=True)
    ts_compact = _dt.datetime.now().strftime("%Y%m%d-%H%M%S")
    out_csv = args.out or (
        RESULTS_DIR / f"{args.config.stem}_{ts_compact}.csv"
    )

    print(f"[bench] config: {args.config}")
    print(f"[bench] binary: {args.binary}")
    print(f"[bench] cwd:    {args.cwd}")
    print(f"[bench] out:    {out_csv}")
    print(f"[bench] {len(runs)} configs, est. trials: "
          f"{sum(r['repeat'] for r in runs)}")

    write_header = not out_csv.exists()
    with open(out_csv, "a", newline="") as csvf:
        w = csv.DictWriter(csvf, fieldnames=CSV_FIELDS)
        if write_header:
            w.writeheader()

        for i, run in enumerate(runs, 1):
            for trial in range(1, run["repeat"] + 1):
                run_id = (
                    f"{run['cipher']}_R{run['rounds']}_{run['activebits']}_"
                    f"m{run['reduction']}_t{run['threads']}_trial{trial}_"
                    f"{ts_compact}"
                )
                cmd_str = (
                    f"./EasyBC -div {run['cipher']} {run['reduction']} "
                    f"{run['rounds']} {run['activebits']} "
                    f"timer {run['timer_sec']} threads {run['threads']}"
                )
                print(f"[bench] [{i}/{len(runs)}] trial {trial}/{run['repeat']}: {cmd_str}")
                if args.dry_run:
                    continue

                t0 = time.time()
                rc, stdout, stderr = run_easybc(
                    args.binary, args.cwd, run["cipher"], run["reduction"],
                    run["rounds"], run["activebits"], run["timer_sec"],
                    run["threads"],
                )
                wall = time.time() - t0

                log_path = LOG_DIR / f"{run_id}.log"
                with open(log_path, "w") as lf:
                    lf.write("=== CMD ===\n" + cmd_str + "\n")
                    lf.write(f"=== EXIT {rc} (wall {wall:.2f}s) ===\n")
                    lf.write("=== STDOUT ===\n" + stdout + "\n")
                    lf.write("=== STDERR ===\n" + stderr + "\n")

                ts_iso = _dt.datetime.now().isoformat(timespec="seconds")
                row = row_from_run(run_id, run, trial, ts_iso, stderr, rc, wall)
                result_info = parse_result_file(
                    result_path_for(run["cipher"], run["rounds"], run["activebits"])
                )
                row["output_bits"] = ",".join(result_info["output_bits"])
                row["set_zero"] = ",".join(result_info["set_zero"])
                row["balanced_bits"] = ",".join(result_info["balanced_bits"])
                row["n_balanced"] = len(result_info["balanced_bits"])
                w.writerow(row)
                csvf.flush()

                bal = result_info["balanced_bits"]
                if bal:
                    print(f"        balanced bits ({len(bal)}): "
                          f"{','.join(bal[:8])}{'...' if len(bal) > 8 else ''}")

                if rc != 0:
                    print(f"        WARN: exit code {rc}; see {log_path}")

    print(f"[bench] CSV written: {out_csv}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
