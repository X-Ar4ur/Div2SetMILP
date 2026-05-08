#!/usr/bin/env python3
"""
Aggregate one or more bench.py CSVs into LaTeX tables and matplotlib plots.

Three table builders, matched to the three YAML configs:

  table1_correctness  — pass/fail vs golden balanced-bit set
  table2_performance  — phase-by-phase median(min, max) timing per cipher
  table3_reduction    — N_ineq, T_reduce, T_solve across reduction methods

Usage
-----
    python make_tables.py --table 1 results/correctness_*.csv > tables/table1.tex
    python make_tables.py --table 2 results/perf_*.csv        > tables/table2.tex
    python make_tables.py --table 3 results/reduction_*.csv   > tables/table3.tex
    python make_tables.py --plot scaling results/perf_*.csv   # writes tables/scaling.png

Golden balanced-bit JSON files live at experiments/golden/<key>.json
where key = "<cipher>_R<rounds>_<activebits>".
"""

from __future__ import annotations

import argparse
import csv
import json
import statistics
import sys
from collections import defaultdict
from pathlib import Path
from typing import Any, Iterable

HERE = Path(__file__).resolve().parent
GOLDEN_DIR = HERE / "golden"
TABLES_DIR = HERE / "tables"


# --------------------------------------------------------------------------
# CSV ingest
# --------------------------------------------------------------------------


def load_rows(paths: Iterable[Path]) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for p in paths:
        with open(p) as f:
            r = csv.DictReader(f)
            rows.extend(r)
    return rows


def to_int(s: str, default: int = 0) -> int:
    try:
        return int(s)
    except (ValueError, TypeError):
        return default


def to_float(s: str, default: float = float("nan")) -> float:
    try:
        return float(s)
    except (ValueError, TypeError):
        return default


def cell_key(row: dict[str, str]) -> tuple:
    """Identity for a (cipher, rounds, activebits, reduction, threads) cell."""
    return (
        row.get("cipher", ""),
        row.get("rounds", ""),
        row.get("activebits", ""),
        row.get("reduction", ""),
        row.get("threads", ""),
    )


def aggregate_cells(rows: list[dict[str, str]]) -> dict[tuple, list[dict[str, str]]]:
    groups: dict[tuple, list[dict[str, str]]] = defaultdict(list)
    for r in rows:
        groups[cell_key(r)].append(r)
    return groups


def stat_ms(rows: list[dict[str, str]], col: str) -> tuple[float, float, float]:
    vals = [to_float(r.get(col, "")) for r in rows]
    vals = [v for v in vals if v == v]  # drop NaN
    if not vals:
        return float("nan"), float("nan"), float("nan")
    return statistics.median(vals), min(vals), max(vals)


# --------------------------------------------------------------------------
# Golden comparison (Table 1)
# --------------------------------------------------------------------------


def golden_key(cipher: str, rounds: str, activebits: str) -> str:
    return f"{cipher}_R{rounds}_{activebits}"


def load_golden(cipher: str, rounds: str, activebits: str) -> dict[str, Any] | None:
    p = GOLDEN_DIR / f"{golden_key(cipher, rounds, activebits)}.json"
    if not p.exists():
        return None
    with open(p) as f:
        return json.load(f)


def read_balanced_bits(cipher: str, rounds: str, activebits: str) -> list[str]:
    """Reread result_*.txt to get the ours balanced-bit set."""
    import re
    p = (HERE.parent / "data" / "division" / cipher / "milp"
         / f"result_{rounds}_{activebits}.txt")
    if not p.exists():
        return []
    bal = []
    pat = re.compile(r"^(x\d+)=(\d+)$")
    with open(p) as f:
        for line in f:
            m = pat.match(line.strip())
            if m and m.group(2) == "1":
                bal.append(m.group(1))
    return sorted(set(bal))


def table1_correctness(rows: list[dict[str, str]]) -> str:
    """Row per (cipher, rounds, activebits): paper match yes/no."""
    seen: set[tuple] = set()
    lines = [
        r"\begin{tabular}{llrrrrl}",
        r"\toprule",
        r"Cipher & Activebits & Rounds & |Bal| (ours) & |Bal| (paper) & Match & Note \\",
        r"\midrule",
    ]
    for r in rows:
        key = (r["cipher"], r["rounds"], r["activebits"])
        if key in seen:
            continue
        seen.add(key)
        cipher, rounds, activebits = key
        ours = read_balanced_bits(cipher, rounds, activebits)
        gold = load_golden(cipher, rounds, activebits)
        if gold is None:
            paper_n = "?"
            match = r"\textcolor{orange}{N/A}"
            note = "no golden file"
        else:
            paper_set = set(gold.get("balanced_bits", []))
            paper_n = len(paper_set)
            ours_set = set(ours)
            if ours_set == paper_set:
                match = r"\textcolor{teal}{\checkmark}"
                note = ""
            else:
                missing = paper_set - ours_set
                extra = ours_set - paper_set
                match = r"\textcolor{red}{\times}"
                note = (f"missing {len(missing)}, extra {len(extra)}"
                        if missing or extra else "")
        lines.append(
            f"{cipher} & {activebits} & {rounds} & {len(ours)} & "
            f"{paper_n} & {match} & {note} \\\\"
        )
    lines += [r"\bottomrule", r"\end{tabular}"]
    return "\n".join(lines)


# --------------------------------------------------------------------------
# Performance table (Table 2)
# --------------------------------------------------------------------------


def fmt_ms(med: float, lo: float, hi: float) -> str:
    if med != med:
        return "—"
    return f"{med/1000:.2f} ({lo/1000:.2f}, {hi/1000:.2f})"


def table2_performance(rows: list[dict[str, str]]) -> str:
    groups = aggregate_cells(rows)
    lines = [
        r"\begin{tabular}{llrrrrrrrr}",
        r"\toprule",
        r"Cipher & Activebits & R & Thr & T\_trail & T\_reduce & T\_build & T\_solve & N\_vars & N\_cons \\",
        r"       &            &   &     & (s)      & (s)       & (s)      & (s)      &        &        \\",
        r"\midrule",
    ]
    for key in sorted(groups):
        cipher, rounds, activebits, red, thr = key
        cell = groups[key]
        t_trail = stat_ms(cell, "trail_ms_total")
        t_reduce = stat_ms(cell, "reduce_ms_total")
        t_build = stat_ms(cell, "build_ms")
        t_solve = stat_ms(cell, "solve_ms")
        n_vars = stat_ms(cell, "n_vars")[0]
        n_cons = stat_ms(cell, "n_cons")[0]
        lines.append(
            f"{cipher} & {activebits} & {rounds} & {thr} & "
            f"{fmt_ms(*t_trail)} & {fmt_ms(*t_reduce)} & "
            f"{fmt_ms(*t_build)} & {fmt_ms(*t_solve)} & "
            f"{int(n_vars) if n_vars == n_vars else '—'} & "
            f"{int(n_cons) if n_cons == n_cons else '—'} \\\\"
        )
    lines += [r"\bottomrule", r"\end{tabular}"]
    return "\n".join(lines)


# --------------------------------------------------------------------------
# Reduction-method ablation (Table 3)
# --------------------------------------------------------------------------


METHOD_NAMES = {
    "1": "greedy\\_sun",
    "2": "sub\\_milp",
    "3": "convex\\_hull",
    "4": "logic\\_cond",
    "5": "comb233",
    "6": "superball",
    "7": "external",
}


def table3_reduction(rows: list[dict[str, str]]) -> str:
    # Group by (cipher, rounds, activebits) → method → cell rows
    cells: dict[tuple, dict[str, list[dict[str, str]]]] = defaultdict(lambda: defaultdict(list))
    for r in rows:
        key = (r["cipher"], r["rounds"], r["activebits"])
        cells[key][r["reduction"]].append(r)

    lines = [
        r"\begin{tabular}{llrlrrrr}",
        r"\toprule",
        r"Cipher & Active & R & Method & N\_ineq & T\_reduce (s) & T\_solve (s) & T\_total (s) \\",
        r"\midrule",
    ]
    for key in sorted(cells):
        cipher, rounds, activebits = key
        first = True
        for method in sorted(cells[key], key=lambda x: int(x or "0")):
            cell = cells[key][method]
            n_ineq = stat_ms(cell, "reduce_n_after_total")[0]
            t_red = stat_ms(cell, "reduce_ms_total")
            t_solve = stat_ms(cell, "solve_ms")
            t_total_med = (
                (t_red[0] + t_solve[0]) / 1000
                if t_red[0] == t_red[0] and t_solve[0] == t_solve[0] else float("nan")
            )
            method_label = METHOD_NAMES.get(method, method)
            lines.append(
                f"{cipher if first else ''} & "
                f"{activebits if first else ''} & "
                f"{rounds if first else ''} & "
                f"{method_label} & "
                f"{int(n_ineq) if n_ineq == n_ineq else '—'} & "
                f"{fmt_ms(*t_red)} & "
                f"{fmt_ms(*t_solve)} & "
                f"{t_total_med:.2f} \\\\"
            )
            first = False
        lines.append(r"\midrule")
    if lines[-1] == r"\midrule":
        lines.pop()
    lines += [r"\bottomrule", r"\end{tabular}"]
    return "\n".join(lines)


# --------------------------------------------------------------------------
# Scaling plot
# --------------------------------------------------------------------------


def plot_scaling(rows: list[dict[str, str]], outfile: Path) -> None:
    try:
        import matplotlib.pyplot as plt  # type: ignore
    except ImportError:
        print("matplotlib not installed; skip plot.", file=sys.stderr)
        return

    groups: dict[str, list[tuple[int, float]]] = defaultdict(list)
    for r in rows:
        cipher = r["cipher"]
        rounds = to_int(r["rounds"])
        solve_ms = to_float(r.get("solve_ms", ""))
        if solve_ms != solve_ms:
            continue
        groups[cipher].append((rounds, solve_ms / 1000.0))
    fig, ax = plt.subplots(figsize=(6, 4))
    for cipher, pts in sorted(groups.items()):
        pts.sort()
        xs = [p[0] for p in pts]
        ys = [p[1] for p in pts]
        ax.plot(xs, ys, marker="o", label=cipher)
    ax.set_xlabel("Rounds")
    ax.set_ylabel("Solve time (s)")
    ax.set_yscale("log")
    ax.set_title("MILP solve time vs rounds")
    ax.legend()
    ax.grid(True, which="both", alpha=0.3)
    fig.tight_layout()
    outfile.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(outfile, dpi=120)
    print(f"wrote {outfile}")


# --------------------------------------------------------------------------
# Entry
# --------------------------------------------------------------------------


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("csv", nargs="+", type=Path)
    ap.add_argument("--table", choices=["1", "2", "3"],
                    help="Emit Table N as LaTeX to stdout")
    ap.add_argument("--plot", choices=["scaling"],
                    help="Emit a matplotlib figure under tables/")
    args = ap.parse_args()

    rows = load_rows(args.csv)
    if not rows:
        print("ERROR: no rows loaded", file=sys.stderr)
        return 2

    if args.table == "1":
        print(table1_correctness(rows))
    elif args.table == "2":
        print(table2_performance(rows))
    elif args.table == "3":
        print(table3_reduction(rows))
    elif args.plot == "scaling":
        plot_scaling(rows, TABLES_DIR / "scaling.png")
    else:
        ap.error("specify --table 1|2|3 or --plot scaling")
    return 0


if __name__ == "__main__":
    sys.exit(main())
