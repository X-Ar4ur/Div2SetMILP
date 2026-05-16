#!/usr/bin/env python3
"""
Search EasyBC 2-subset input structures by enumerating explicit hex masks.

The script keeps the MILP semantics unchanged: every candidate is evaluated by
calling `./EasyBC -div <cipher> <reduction> <rounds> hex:<mask>`.
"""

from __future__ import annotations

import argparse
import csv
import datetime as _dt
import itertools
import json
import os
import random
import re
import subprocess
import sys
import time
from pathlib import Path
from typing import Any, Iterable, NamedTuple

try:
    import yaml  # type: ignore
except ImportError:
    print("ERROR: PyYAML required. pip install pyyaml", file=sys.stderr)
    sys.exit(2)

import bench


HERE = Path(__file__).resolve().parent
REPO_ROOT = HERE.parent
DEFAULT_BINARY = Path(os.environ.get("EASYBC_BINARY", bench.DEFAULT_BINARY))
DEFAULT_CWD = bench.DEFAULT_CWD
RESULTS_DIR = HERE / "results"
LOG_DIR = RESULTS_DIR / "logs"


class ActiveMask(NamedTuple):
    hex_spec: str
    weight: int
    inactive_positions: list[int]


class Candidate(NamedTuple):
    mask: ActiveMask
    source_strategy: str


CSV_FIELDS = [
    "run_id", "ts", "cipher", "rounds", "published_round", "mask", "weight",
    "inactive_positions", "reduction", "threads", "trial", "source_strategy",
    "balanced_bits", "n_balanced", "is_improvement", "runtime", "total_ms",
    "gurobi_status", "distinguisher_found", "exit_code", "log_path",
]


def mask_from_active_vars(block_size: int, active_vars: Iterable[int]) -> ActiveMask:
    active = sorted(set(int(v) for v in active_vars), reverse=True)
    if any(v < 1 or v > block_size for v in active):
        raise ValueError(f"active variable outside x1..x{block_size}: {active}")
    value = 0
    for xi in active:
        value |= 1 << (xi - 1)
    hex_len = (block_size + 3) // 4
    inactive = [xi for xi in range(block_size, 0, -1) if xi not in set(active)]
    return ActiveMask(f"hex:{value:0{hex_len}x}", len(active), inactive)


def default_active_vars(cipher: str, block_size: int, weight: int) -> list[int]:
    if weight < 0 or weight > block_size:
        raise ValueError(f"weight {weight} outside [0, {block_size}]")

    if cipher == "Rectangle":
        active = []
        for i in range(weight):
            row = (i + 2) % 4
            col = 15 - (i // 4)
            active.append(row * 16 + col + 1)
        return active

    if cipher == "LBlock":
        word_len = block_size // 2

        def half_idx(i: int) -> int:
            return (7 - (i // 4)) * 4 + (i % 4)

        y_active = min(weight, word_len)
        x_active = max(0, weight - word_len)
        active = [word_len + half_idx(i) + 1 for i in range(y_active)]
        active.extend(half_idx(i) + 1 for i in range(x_active))
        return active

    return [block_size - i for i in range(weight)]


def default_mask(cipher: str, block_size: int, weight: int) -> ActiveMask:
    return mask_from_active_vars(block_size, default_active_vars(cipher, block_size, weight))


def active_vars_from_inactive(block_size: int, inactive_vars: Iterable[int]) -> list[int]:
    inactive = set(int(v) for v in inactive_vars)
    return [xi for xi in range(block_size, 0, -1) if xi not in inactive]


def rotate_vars(block_size: int, active_vars: Iterable[int], shift: int) -> list[int]:
    return [((xi - 1 + shift) % block_size) + 1 for xi in active_vars]


def candidate_key(candidate: Candidate) -> str:
    return candidate.mask.hex_spec


def dedupe(candidates: Iterable[Candidate]) -> list[Candidate]:
    seen: set[str] = set()
    out: list[Candidate] = []
    for cand in candidates:
        key = candidate_key(cand)
        if key in seen:
            continue
        seen.add(key)
        out.append(cand)
    return out


def published_variants(cipher: str, block_size: int, weight: int) -> list[Candidate]:
    base_vars = default_active_vars(cipher, block_size, weight)
    candidates = [Candidate(mask_from_active_vars(block_size, base_vars), "published_variants")]
    shifts = set(range(1, block_size))
    shifts.update(range(4, block_size, 4))
    shifts.update(range(16, block_size, 16))
    for shift in sorted(shifts):
        candidates.append(
            Candidate(mask_from_active_vars(block_size, rotate_vars(block_size, base_vars, shift)),
                      "published_variants")
        )
    return dedupe(candidates)


def structured_inactive(block_size: int, weight: int) -> list[Candidate]:
    inactive_count = block_size - weight
    if inactive_count <= 0:
        return []

    groups: list[list[int]] = []
    ordered = list(range(block_size, 0, -1))
    groups.extend(ordered[i:i + 4] for i in range(0, block_size, 4))
    groups.extend(ordered[i:i + 8] for i in range(0, block_size, 8))
    groups.extend([ordered[: block_size // 2], ordered[block_size // 2:]])
    for mod in (4, 8, 16):
        groups.extend([[xi for xi in ordered if (block_size - xi) % mod == off]
                       for off in range(mod)])

    candidates = []
    for group in groups:
        if len(group) < inactive_count:
            continue
        for inactive in itertools.combinations(group, inactive_count):
            candidates.append(
                Candidate(
                    mask_from_active_vars(block_size, active_vars_from_inactive(block_size, inactive)),
                    "structured_inactive",
                )
            )
    return dedupe(candidates)


def small_inactive_exhaustive(block_size: int, weight: int) -> list[Candidate]:
    inactive_count = block_size - weight
    if inactive_count <= 0 or inactive_count > 2:
        return []
    candidates = []
    for inactive in itertools.combinations(range(block_size, 0, -1), inactive_count):
        candidates.append(
            Candidate(
                mask_from_active_vars(block_size, active_vars_from_inactive(block_size, inactive)),
                "small_inactive_exhaustive",
            )
        )
    return candidates


def random_sample(block_size: int, weight: int, samples: int, seed: int) -> list[Candidate]:
    inactive_count = block_size - weight
    if samples <= 0 or inactive_count <= 0:
        return []
    rng = random.Random(seed)
    all_vars = list(range(block_size, 0, -1))
    candidates = []
    for _ in range(samples):
        inactive = rng.sample(all_vars, inactive_count)
        candidates.append(
            Candidate(
                mask_from_active_vars(block_size, active_vars_from_inactive(block_size, inactive)),
                "random_sample",
            )
        )
    return dedupe(candidates)


def local_search(block_size: int, seeds: list[Candidate], limit: int) -> list[Candidate]:
    if limit <= 0:
        return []
    candidates = []
    for seed in seeds:
        inactive = set(seed.mask.inactive_positions)
        active = [xi for xi in range(block_size, 0, -1) if xi not in inactive]
        for old_inactive in sorted(inactive, reverse=True):
            for new_inactive in active:
                nxt = set(inactive)
                nxt.remove(old_inactive)
                nxt.add(new_inactive)
                candidates.append(
                    Candidate(
                        mask_from_active_vars(block_size, active_vars_from_inactive(block_size, nxt)),
                        "local_search",
                    )
                )
                if len(candidates) >= limit:
                    return dedupe(candidates)
    return dedupe(candidates)


def generate_candidates(run: dict[str, Any], defaults: dict[str, Any]) -> list[Candidate]:
    cipher = run["cipher"]
    block_size = int(run["block_size"])
    weight = int(run["weight"])
    strategies = run.get("strategies", defaults.get("strategies", ["published_variants"]))
    samples = int(run.get("random_samples", defaults.get("random_samples", 1000)))
    seed = int(run.get("seed", defaults.get("seed", 0)))
    local_limit = int(run.get("local_search_limit", defaults.get("local_search_limit", 256)))

    candidates: list[Candidate] = []
    seed_candidates: list[Candidate] = []
    if "published_variants" in strategies:
        seed_candidates = published_variants(cipher, block_size, weight)
        candidates.extend(seed_candidates)
    if "structured_inactive" in strategies:
        candidates.extend(structured_inactive(block_size, weight))
    if "small_inactive_exhaustive" in strategies:
        candidates.extend(small_inactive_exhaustive(block_size, weight))
    if "random_sample" in strategies:
        candidates.extend(random_sample(block_size, weight, samples, seed))
    if "local_search" in strategies:
        if not seed_candidates:
            seed_candidates = [Candidate(default_mask(cipher, block_size, weight), "published_variants")]
        candidates.extend(local_search(block_size, seed_candidates, local_limit))
    return dedupe(candidates)


def rounds_for_run(run: dict[str, Any]) -> list[int]:
    if "rounds_sweep" in run:
        return [int(r) for r in run["rounds_sweep"]]
    if "round_start" in run and "round_end" in run:
        return list(range(int(run["round_start"]), int(run["round_end"]) + 1))
    if "rounds" in run:
        return [int(run["rounds"])]
    published = int(run["published_round"])
    return [published, published + 1]


def expand_search_runs(cfg: dict[str, Any]) -> list[dict[str, Any]]:
    defaults = cfg.get("defaults", {})
    repeat = int(defaults.get("repeat", 1))
    timer_sec = int(defaults.get("timer_sec", 86400))
    reduction = int(defaults.get("reduction", 1))
    threads = int(defaults.get("threads", 8))
    max_candidates = defaults.get("max_candidates")
    expanded: list[dict[str, Any]] = []

    for run in cfg["runs"]:
        candidates = generate_candidates(run, defaults)
        if max_candidates is not None:
            candidates = candidates[: int(max_candidates)]
        per_rounds = rounds_for_run(run)
        for rounds in per_rounds:
            for cand in candidates:
                expanded.append(
                    {
                        "cipher": run["cipher"],
                        "rounds": rounds,
                        "published_round": int(run.get("published_round", rounds)),
                        "activebits": cand.mask.hex_spec,
                        "mask": cand.mask.hex_spec,
                        "weight": cand.mask.weight,
                        "inactive_positions": cand.mask.inactive_positions,
                        "source_strategy": cand.source_strategy,
                        "reduction": int(run.get("reduction", reduction)),
                        "threads": int(run.get("threads", threads)),
                        "repeat": int(run.get("repeat", repeat)),
                        "timer_sec": int(run.get("timer_sec", timer_sec)),
                    }
                )
    return expanded


def load_config(path: Path) -> dict[str, Any]:
    with open(path) as f:
        cfg = yaml.safe_load(f)
    if not isinstance(cfg, dict) or "runs" not in cfg:
        raise SystemExit(f"{path}: missing top-level 'runs' list")
    cfg.setdefault("defaults", {})
    return cfg


def parse_bool_int(value: str) -> int:
    try:
        return int(value)
    except (TypeError, ValueError):
        return 0


def is_improvement(run: dict[str, Any], n_balanced: int) -> str:
    if run["rounds"] > run["published_round"] and n_balanced > 0:
        return "strong"
    if run["rounds"] == run["published_round"] and n_balanced > 0:
        return "candidate"
    return "none"


def run_one(
    binary: Path, cwd: Path, run: dict[str, Any], trial: int, ts_compact: str,
) -> dict[str, Any]:
    run_id = (
        f"{run['cipher']}_R{run['rounds']}_{bench.safe_activebits_id(run['mask'])}_"
        f"m{run['reduction']}_t{run['threads']}_trial{trial}_{ts_compact}"
    )
    cmd = [
        str(binary), "-div", run["cipher"], str(run["reduction"]),
        str(run["rounds"]), run["mask"], "timer", str(run["timer_sec"]),
        "threads", str(run["threads"]),
    ]
    t0 = time.time()
    proc = subprocess.run(cmd, cwd=str(cwd), capture_output=True, text=True, check=False)
    wall = time.time() - t0

    log_path = LOG_DIR / f"{run_id}.log"
    with open(log_path, "w") as lf:
        lf.write("=== CMD ===\n" + " ".join(cmd) + "\n")
        lf.write(f"=== EXIT {proc.returncode} (wall {wall:.2f}s) ===\n")
        lf.write("=== STDOUT ===\n" + proc.stdout + "\n")
        lf.write("=== STDERR ===\n" + proc.stderr + "\n")

    result_info = bench.parse_result_file(
        bench.result_path_for(run["cipher"], run["rounds"], run["mask"])
    )
    balanced_bits = result_info["balanced_bits"]
    phases = bench.parse_bench_lines(proc.stderr)
    solve = phases.get("solve", {})

    return {
        "run_id": run_id,
        "ts": _dt.datetime.now().isoformat(timespec="seconds"),
        "cipher": run["cipher"],
        "rounds": run["rounds"],
        "published_round": run["published_round"],
        "mask": run["mask"],
        "weight": run["weight"],
        "inactive_positions": ",".join(str(x) for x in run["inactive_positions"]),
        "reduction": run["reduction"],
        "threads": run["threads"],
        "trial": trial,
        "source_strategy": run["source_strategy"],
        "balanced_bits": ",".join(balanced_bits),
        "n_balanced": len(balanced_bits),
        "is_improvement": is_improvement(run, len(balanced_bits)),
        "runtime": f"{wall:.3f}",
        "total_ms": solve.get("total_ms", ""),
        "gurobi_status": solve.get("gurobi_status", ""),
        "distinguisher_found": solve.get("distinguisher_found", ""),
        "exit_code": proc.returncode,
        "log_path": str(log_path),
    }


def write_summary(rows: list[dict[str, Any]], path: Path) -> None:
    strong = [r for r in rows if r["is_improvement"] == "strong"]
    best_by_cipher: dict[str, dict[str, Any]] = {}
    for row in rows:
        key = row["cipher"]
        cur = best_by_cipher.get(key)
        if cur is None or int(row["rounds"]) > int(cur["rounds"]) or int(row["n_balanced"]) > int(cur["n_balanced"]):
            best_by_cipher[key] = row
    summary = {
        "n_runs": len(rows),
        "n_strong_improvements": len(strong),
        "strong_improvements": strong,
        "best_by_cipher": best_by_cipher,
    }
    path.write_text(json.dumps(summary, indent=2, sort_keys=True))


def main() -> int:
    ap = argparse.ArgumentParser(description="Search division-property input masks")
    ap.add_argument("--config", required=True, type=Path)
    ap.add_argument("--binary", type=Path, default=DEFAULT_BINARY)
    ap.add_argument("--cwd", type=Path, default=DEFAULT_CWD)
    ap.add_argument("--repeat", type=int, default=None)
    ap.add_argument("--threads", type=int, default=None)
    ap.add_argument("--out", type=Path, default=None)
    ap.add_argument("--summary", type=Path, default=None)
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    cfg = load_config(args.config)
    runs = expand_search_runs(cfg)
    if args.repeat is not None:
        for run in runs:
            run["repeat"] = args.repeat
    if args.threads is not None:
        for run in runs:
            run["threads"] = args.threads

    ts_compact = _dt.datetime.now().strftime("%Y%m%d-%H%M%S")
    out_csv = args.out or RESULTS_DIR / f"{args.config.stem}_{ts_compact}.csv"
    summary_path = args.summary or RESULTS_DIR / f"{args.config.stem}_{ts_compact}.summary.json"

    print(f"[active-search] config: {args.config}")
    print(f"[active-search] binary: {args.binary}")
    print(f"[active-search] cwd:    {args.cwd}")
    print(f"[active-search] out:    {out_csv}")
    print(f"[active-search] {len(runs)} candidates, est. trials: {sum(r['repeat'] for r in runs)}")

    if args.dry_run:
        for run in runs:
            print(
                f"./EasyBC -div {run['cipher']} {run['reduction']} "
                f"{run['rounds']} {run['mask']} timer {run['timer_sec']} "
                f"threads {run['threads']}  # {run['source_strategy']}"
            )
        return 0

    if not args.binary.exists():
        print(f"ERROR: EasyBC binary not found at {args.binary}", file=sys.stderr)
        return 2

    LOG_DIR.mkdir(parents=True, exist_ok=True)
    RESULTS_DIR.mkdir(parents=True, exist_ok=True)
    rows: list[dict[str, Any]] = []
    write_header = not out_csv.exists()
    with open(out_csv, "a", newline="") as csvf:
        writer = csv.DictWriter(csvf, fieldnames=CSV_FIELDS)
        if write_header:
            writer.writeheader()
        for idx, run in enumerate(runs, 1):
            for trial in range(1, run["repeat"] + 1):
                print(f"[active-search] [{idx}/{len(runs)}] trial {trial}/{run['repeat']}: "
                      f"{run['cipher']} R{run['rounds']} {run['mask']}")
                row = run_one(args.binary, args.cwd, run, trial, ts_compact)
                writer.writerow(row)
                csvf.flush()
                rows.append(row)
    write_summary(rows, summary_path)
    print(f"[active-search] summary written: {summary_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
