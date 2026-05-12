#!/usr/bin/env python3
"""
Parse a single EasyBC stderr/stdout log and emit a JSON summary.

Used by humans to inspect a run, and by make_tables.py for richer fields
that aren't already in bench.py's CSV (e.g. per-S-box trail counts).

Usage
-----
    python parse_log.py results/logs/<run_id>.log
    python parse_log.py results/logs/<run_id>.log --json > summary.json
    python parse_log.py results/logs/<run_id>.log --with-balanced  # show balanced bits

Reads result_*.txt from data/division/<cipher>/milp/ when given the
--with-balanced flag, by recovering (cipher, rounds, activebits) from
the [BENCH] config line. Balanced bits are parsed as all output bits minus
the coordinates set to zero by the iterative solver.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any

HERE = Path(__file__).resolve().parent
REPO_ROOT = HERE.parent

BENCH_LINE_RE = re.compile(r"^\[BENCH\]\s+(.*)$")
KV_RE = re.compile(r"(\w+)=([^\s]+)")
VAR_RE = re.compile(r"x\d+")


def parse_kv(s: str) -> dict[str, str]:
    return dict(KV_RE.findall(s))


def natural_var_key(name: str) -> tuple[str, int]:
    m = re.match(r"([a-zA-Z]+)(\d+)$", name)
    if not m:
        return name, 0
    return m.group(1), int(m.group(2))


def sorted_vars(values: list[str]) -> list[str]:
    return sorted(set(values), key=natural_var_key)


def vars_from_line(line: str) -> list[str]:
    return VAR_RE.findall(line)


def parse(text: str) -> dict[str, Any]:
    """Parse all [BENCH] lines and return a structured dict.

    Multiple S-boxes ⇒ phase=trail, phase=ineq_gen, phase=reduce can each
    appear several times. We collect them as lists keyed by sbox.
    """
    summary: dict[str, Any] = {
        "config": {},
        "trail":     {},   # sbox -> {elapsed_ms, n_trails}
        "ineq_gen":  {},   # sbox -> {elapsed_ms, n_ineq}
        "reduce":    {},   # sbox -> {method, elapsed_ms, n_ineq_before, n_ineq_after}
        "preprocess": None,
        "build":      None,
        "model_load": None,
        "solve":      None,
    }

    for line in text.splitlines():
        m = BENCH_LINE_RE.match(line)
        if not m:
            continue
        body = m.group(1)
        kvs = parse_kv(body)

        # config header has no `phase=` key.
        if "phase" not in kvs and body.lstrip().startswith("config"):
            summary["config"] = parse_kv(body[len("config"):])
            continue

        phase = kvs.pop("phase", "")
        if phase == "trail":
            summary["trail"][kvs.get("sbox", "?")] = kvs
        elif phase == "ineq_gen":
            summary["ineq_gen"][kvs.get("sbox", "?")] = kvs
        elif phase == "reduce":
            summary["reduce"][kvs.get("sbox", "?")] = kvs
        elif phase in ("preprocess", "build", "model_load", "solve"):
            summary[phase] = kvs

    return summary


def read_balanced_bits(cipher: str, rounds: str, activebits: str) -> list[str]:
    p = (REPO_ROOT / "data" / "division" / cipher / "milp"
         / f"result_{rounds}_{activebits}.txt")
    if not p.exists():
        return []
    output_bits: list[str] = []
    set_zero: list[str] = []
    listed_balanced: list[str] = []
    in_set_zero_block = False
    for raw in p.read_text().splitlines():
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
        return sorted_vars(list(set(output_bits) - set(set_zero)))
    return sorted_vars(listed_balanced)


def fmt_human(s: dict[str, Any]) -> str:
    cfg = s["config"] or {}
    out = []
    out.append(f"cipher={cfg.get('cipher', '?')}  "
               f"reduction={cfg.get('reduction', '?')}  "
               f"rounds={cfg.get('rounds', '?')}  "
               f"activebits={cfg.get('activebits', '?')}")
    if s["trail"]:
        out.append("\n[trail extraction]")
        total_ms = 0
        total_n = 0
        for sbox, kv in sorted(s["trail"].items()):
            ms = int(kv.get("elapsed_ms", 0))
            n = int(kv.get("n_trails", 0))
            total_ms += ms
            total_n += n
            out.append(f"  {sbox:16s}  {ms:8d} ms   {n:6d} trails")
        out.append(f"  {'TOTAL':16s}  {total_ms:8d} ms   {total_n:6d} trails")
    if s["reduce"]:
        out.append("\n[inequality reduction]")
        for sbox, kv in sorted(s["reduce"].items()):
            out.append(
                f"  {sbox:16s}  method={kv.get('method', '?'):>2}  "
                f"{kv.get('elapsed_ms', '?'):>8} ms   "
                f"{kv.get('n_ineq_before', '?'):>5} -> "
                f"{kv.get('n_ineq_after', '?'):>5}"
            )
    for phase in ("preprocess", "build", "model_load", "solve"):
        if s[phase]:
            kv = s[phase]
            out.append(f"\n[{phase}]  " + "  ".join(
                f"{k}={v}" for k, v in kv.items()
            ))
    return "\n".join(out)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("log", type=Path, help="Path to bench.py log or raw stderr")
    ap.add_argument("--json", action="store_true",
                    help="Emit JSON instead of human-readable summary")
    ap.add_argument("--with-balanced", action="store_true",
                    help="Also read the result_*.txt file and list balanced bits")
    args = ap.parse_args()

    if not args.log.exists():
        print(f"ERROR: {args.log} not found", file=sys.stderr)
        return 2

    text = args.log.read_text()
    summary = parse(text)

    if args.with_balanced:
        cfg = summary["config"]
        if cfg:
            summary["balanced_bits"] = read_balanced_bits(
                cfg.get("cipher", ""), cfg.get("rounds", ""),
                cfg.get("activebits", ""),
            )

    if args.json:
        json.dump(summary, sys.stdout, indent=2, sort_keys=True)
        sys.stdout.write("\n")
    else:
        print(fmt_human(summary))
        if args.with_balanced:
            bal = summary.get("balanced_bits", [])
            print(f"\n[balanced bits, {len(bal)}]")
            print("  " + ",".join(bal) if bal else "  (none — distinguisher not found)")

    return 0


if __name__ == "__main__":
    sys.exit(main())
