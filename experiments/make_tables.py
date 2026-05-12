#!/usr/bin/env python3
"""
Aggregate one or more bench.py CSVs into LaTeX tables and matplotlib plots.

Four table builders, matched to the experiment YAML configs:

  correctness  — pass/fail vs golden balanced-bit set
  rounds       — round-by-round boundary scan
  perf         — EasyBC timing plus manually maintained external baseline
  reduction    — N_ineq, T_reduce, T_solve across reduction methods

Usage
-----
    python make_tables.py --table correctness results/correctness_*.csv > tables/table1.tex
    python make_tables.py --table rounds results/round_sweep_*.csv      > tables/table2.tex
    python make_tables.py --table perf results/perf_*.csv               > tables/table3.tex
    python make_tables.py --table reduction results/reduction_*.csv     > tables/table4.tex
    python make_tables.py --plot scaling results/perf_*.csv   # writes tables/scaling.png

Golden balanced-bit JSON files live at experiments/golden/<key>.json
where key = "<cipher>_R<rounds>_<activebits>".
"""

from __future__ import annotations

import argparse
import csv
import json
import re
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
# Result-file parsing
# --------------------------------------------------------------------------


VAR_RE = re.compile(r"x\d+")


def natural_var_key(name: str) -> tuple[str, int]:
    m = re.match(r"([a-zA-Z]+)(\d+)$", name)
    if not m:
        return name, 0
    return m.group(1), int(m.group(2))


def sorted_vars(values: Iterable[str]) -> list[str]:
    return sorted(set(values), key=natural_var_key)


def vars_from_line(line: str) -> list[str]:
    return VAR_RE.findall(line)


def parse_result_file(path: Path) -> dict[str, list[str]]:
    """Parse Div2SetMILP result files.

    New result files explicitly list all output bits and set-zero bits. The
    balanced bits are defined as output bits minus set-zero bits. If a future
    or hand-written result file also lists "Balanced bits:", that line is used
    only when the output-bit universe is absent.
    """
    parsed = {"output_bits": [], "set_zero": [], "balanced_bits": []}
    if not path.exists():
        return parsed

    output_bits: list[str] = []
    set_zero: list[str] = []
    listed_balanced: list[str] = []
    in_set_zero_block = False

    for raw in path.read_text().splitlines():
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

    output_set = set(output_bits)
    zero_set = set(set_zero)
    if output_set:
        balanced = output_set - zero_set
    else:
        balanced = set(listed_balanced)

    parsed["output_bits"] = sorted_vars(output_bits)
    parsed["set_zero"] = sorted_vars(set_zero)
    parsed["balanced_bits"] = sorted_vars(balanced)
    return parsed


def result_path_for(cipher: str, rounds: str, activebits: str) -> Path:
    return (
        HERE.parent / "data" / "division" / cipher / "milp"
        / f"result_{rounds}_{activebits}.txt"
    )


def csv_list(value: str) -> list[str]:
    return sorted_vars(VAR_RE.findall(value or ""))


def balanced_bits_for_row(row: dict[str, str]) -> list[str]:
    bits = csv_list(row.get("balanced_bits", ""))
    if bits:
        return bits
    return read_balanced_bits(row.get("cipher", ""), row.get("rounds", ""),
                              row.get("activebits", ""))


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
    return parse_result_file(result_path_for(cipher, rounds, activebits))["balanced_bits"]


def table1_correctness(rows: list[dict[str, str]]) -> str:
    """Row per (cipher, rounds, activebits): paper match yes/no."""
    seen: set[tuple] = set()
    lines = [
        r"\begin{tabular}{llrlrrrlll}",
        r"\toprule",
        r"Cipher & Ref. & R & Active & |Bal|$_p$ & |Bal|$_o$ & Match & Missing & Extra & Status \\",
        r"\midrule",
    ]
    for r in rows:
        if r.get("paper_ref", "").startswith("Eskandari"):
            continue
        key = (r["cipher"], r["rounds"], r["activebits"])
        if key in seen:
            continue
        seen.add(key)
        cipher, rounds, activebits = key
        ours = balanced_bits_for_row(r)
        gold = load_golden(cipher, rounds, activebits)
        ref = r.get("paper_ref") or r.get("ref") or ""
        status = r.get("gurobi_status", "")
        if gold is None:
            paper_n = "?"
            match = r"\textcolor{orange}{N/A}"
            missing_n = "?"
            extra_n = "?"
            ref = ref or "no golden"
        else:
            ref = gold.get("paper_ref", ref)
            paper_set = set(gold.get("balanced_bits", []))
            paper_n = len(paper_set)
            ours_set = set(ours)
            if ours_set == paper_set:
                match = r"\textcolor{teal}{\checkmark}"
                missing_n = "0"
                extra_n = "0"
            else:
                missing = paper_set - ours_set
                extra = ours_set - paper_set
                match = r"\textcolor{red}{\times}"
                missing_n = str(len(missing))
                extra_n = str(len(extra))
        lines.append(
            f"{cipher} & {ref} & {rounds} & {activebits} & {paper_n} & "
            f"{len(ours)} & {match} & {missing_n} & {extra_n} & {status} \\\\"
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


def fmt_seconds(med_ms: float) -> str:
    if med_ms != med_ms:
        return "N/A"
    seconds = med_ms / 1000.0
    if seconds < 1:
        return "0s"
    if seconds < 10:
        return f"{seconds:.1f}s"
    return f"{seconds:.0f}s"


def row_status(rows: list[dict[str, str]]) -> str:
    status = rows[-1].get("gurobi_status", "")
    exit_code = rows[-1].get("exit_code", "0")
    found = rows[-1].get("distinguisher_found", "")
    if status == "9":
        return "T/O"
    if exit_code not in ("", "0"):
        return "N/A"
    if found == "1":
        return r"\checkmark"
    if found == "0" and status in ("2", "3"):
        return r"\times"
    return "N/A"


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


def table_round_sweep(rows: list[dict[str, str]]) -> str:
    cells: dict[tuple[str, str, str], list[dict[str, str]]] = defaultdict(list)
    for r in rows:
        key = (r["cipher"], r["rounds"], r["activebits"])
        cells[key].append(r)

    cipher_order = sorted({key[0] for key in cells})
    max_round = max((to_int(key[1]) for key in cells), default=0)
    round_headers = [str(i) for i in range(1, max_round + 1)]
    col_spec = "ll" + "r" * len(round_headers)
    lines = [
        rf"\begin{{tabular}}{{{col_spec}}}",
        r"\toprule",
        "Cipher & Metric & " + " & ".join(round_headers) + r" \\",
        r"\midrule",
    ]

    for cipher in cipher_order:
        activebits = next(key[2] for key in cells if key[0] == cipher)
        found_cells: list[str] = []
        time_cells: list[str] = []
        for rno in round_headers:
            cell = cells.get((cipher, rno, activebits), [])
            if not cell:
                found_cells.append("N/A")
                time_cells.append("N/A")
                continue
            found_cells.append(row_status(cell))
            med = stat_ms(cell, "total_ms")[0]
            if found_cells[-1] == "T/O":
                time_cells.append("T/O")
            else:
                time_cells.append(fmt_seconds(med))
        lines.append(f"{cipher} & Found & " + " & ".join(found_cells) + r" \\")
        lines.append(r" & Time & " + " & ".join(time_cells) + r" \\")
        lines.append(r"\midrule")
    if lines[-1] == r"\midrule":
        lines.pop()
    lines += [r"\bottomrule", r"\end{tabular}"]
    return "\n".join(lines)


def table_performance_comparison(
    rows: list[dict[str, str]],
    external: dict[tuple[str, str, str], dict[str, str]] | None = None,
) -> str:
    external = external or {}
    groups = aggregate_cells(rows)
    lines = [
        r"\begin{tabular}{lllrrrrl}",
        r"\toprule",
        r"Cipher & R & Active & Thr & EasyBC (s) & External (s) & Speedup & Notes \\",
        r"\midrule",
    ]
    for key in sorted(groups):
        cipher, rounds, activebits, _red, thr = key
        cell = groups[key]
        easy_ms = stat_ms(cell, "total_ms")[0]
        easy_s = easy_ms / 1000.0 if easy_ms == easy_ms else float("nan")
        ext_info = external.get((cipher, rounds, activebits), {})
        ext_raw = ext_info.get("external_time", "")
        notes = ext_info.get("notes", "")
        ext_s = to_float(ext_raw)
        if easy_s == easy_s and ext_s == ext_s and easy_s > 0:
            speedup = f"{ext_s / easy_s:.2f}"
        else:
            speedup = "manual"
        lines.append(
            f"{cipher} & {rounds} & {activebits} & {thr} & "
            f"{easy_s:.2f} & {ext_raw or 'manual'} & {speedup} & {notes} \\\\"
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
        r"\begin{tabular}{llrlrrrrrrl}",
        r"\toprule",
        r"Cipher & Active & R & Method & N\_before & N\_after & Ratio & T\_reduce & N\_cons & T\_solve & Correct \\",
        r"\midrule",
    ]
    for key in sorted(cells):
        cipher, rounds, activebits = key
        baseline_rows = cells[key].get("1") or []
        baseline = set(balanced_bits_for_row(baseline_rows[-1])) if baseline_rows else set()
        first = True
        for method in sorted(cells[key], key=lambda x: int(x or "0")):
            cell = cells[key][method]
            n_before = stat_ms(cell, "reduce_n_before_total")[0]
            n_ineq = stat_ms(cell, "reduce_n_after_total")[0]
            t_red = stat_ms(cell, "reduce_ms_total")
            t_solve = stat_ms(cell, "solve_ms")
            n_cons = stat_ms(cell, "n_cons")[0]
            ratio = (
                f"{(1.0 - n_ineq / n_before) * 100:.1f}\\%"
                if n_before == n_before and n_before > 0 and n_ineq == n_ineq else "—"
            )
            ours = set(balanced_bits_for_row(cell[-1]))
            correct = (
                r"\textcolor{teal}{\checkmark}"
                if not baseline or ours == baseline else r"\textcolor{red}{\times}"
            )
            method_label = METHOD_NAMES.get(method, method)
            lines.append(
                f"{cipher if first else ''} & "
                f"{activebits if first else ''} & "
                f"{rounds if first else ''} & "
                f"{method_label} & "
                f"{int(n_before) if n_before == n_before else '—'} & "
                f"{int(n_ineq) if n_ineq == n_ineq else '—'} & "
                f"{ratio} & "
                f"{fmt_ms(*t_red)} & "
                f"{int(n_cons) if n_cons == n_cons else '—'} & "
                f"{fmt_ms(*t_solve)} & "
                f"{correct} \\\\"
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
    ap.add_argument("--table", choices=[
        "1", "2", "3", "4",
        "correctness", "rounds", "perf", "reduction",
    ],
                    help="Emit Table N as LaTeX to stdout")
    ap.add_argument("--plot", choices=["scaling"],
                    help="Emit a matplotlib figure under tables/")
    args = ap.parse_args()

    rows = load_rows(args.csv)
    if not rows:
        print("ERROR: no rows loaded", file=sys.stderr)
        return 2

    if args.table in ("1", "correctness"):
        print(table1_correctness(rows))
    elif args.table in ("2", "rounds"):
        print(table_round_sweep(rows))
    elif args.table in ("3", "perf"):
        print(table_performance_comparison(rows))
    elif args.table in ("4", "reduction"):
        print(table3_reduction(rows))
    elif args.plot == "scaling":
        plot_scaling(rows, TABLES_DIR / "scaling.png")
    else:
        ap.error("specify --table correctness|rounds|perf|reduction or --plot scaling")
    return 0


if __name__ == "__main__":
    sys.exit(main())
