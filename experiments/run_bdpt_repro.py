"""Run the strict BDPT baseline or the four-combination experiment matrix."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import platform
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

try:
    from experiments.bdpt_repro import generate_report
except ImportError:
    from bdpt_repro import generate_report


HERE = Path(__file__).resolve().parent
ROOT = HERE.parent
DEFAULT_GOLDEN = HERE / "golden" / "bdpt" / "PRESENT_R9_63.json"

MATRIX = [
    ("paper", "per-bit"),
    ("paper", "min-pin"),
    ("exact", "per-bit"),
    ("exact", "min-pin"),
]


def extract_gurobi_version(log_text: str) -> str:
    match = re.search(r"Gurobi Optimizer version ([^\r\n]+)", log_text)
    return match.group(1).strip() if match else "not reported"


def fingerprint_lp(path: Path) -> dict:
    lines = [
        line.strip()
        for line in path.read_text(encoding="utf-8", errors="replace").splitlines()
        if line.strip() and not line.lstrip().startswith("\\")
    ]
    normalized = "\n".join(lines) + "\n"
    section = ""
    constraint_lines = []
    binary_variables = []
    for line in lines:
        lowered = line.lower()
        if lowered == "subject to":
            section = "constraints"
            continue
        if lowered == "binary":
            section = "binary"
            continue
        if lowered == "end":
            section = ""
            continue
        if section == "constraints":
            constraint_lines.append(line)
        elif section == "binary":
            binary_variables.append(line)

    nonzero_terms = sum(
        len(re.findall(r"\b(?:x|d)\d+\b", line))
        for line in constraint_lines
    )
    return {
        "path": path.name,
        "sha256": hashlib.sha256(normalized.encode("utf-8")).hexdigest(),
        "constraints": len(constraint_lines),
        "binary_variables": len(binary_variables),
        "nonzero_terms": nonzero_terms,
    }


def build_command(
    *,
    binary: Path,
    cipher: str,
    reduction: int,
    rounds: int,
    activebits: str,
    cross: str,
    solver: str,
    timer: int,
    threads: int,
) -> list[str]:
    return [
        str(binary),
        "-div3",
        cipher,
        str(reduction),
        str(rounds),
        activebits,
        "cross",
        cross,
        "solver",
        solver,
        "sign",
        "1",
        "repro",
        "1",
        "timer",
        str(timer),
        "threads",
        str(threads),
    ]


def result_path(
    *,
    binary: Path,
    cipher: str,
    rounds: int,
    activebits: str,
    cross: str,
    solver: str,
) -> Path:
    repo_root = binary.resolve().parent.parent
    return (
        repo_root
        / "data"
        / "division"
        / cipher
        / "repro"
        / "subset3"
        / f"{rounds}_{activebits}_{cross}_{solver}"
        / "result.txt"
    )


def run_one(
    *,
    binary: Path,
    cipher: str,
    reduction: int,
    rounds: int,
    activebits: str,
    cross: str,
    solver: str,
    timer: int,
    threads: int,
    golden: Path,
    dry_run: bool,
) -> dict:
    command = build_command(
        binary=binary,
        cipher=cipher,
        reduction=reduction,
        rounds=rounds,
        activebits=activebits,
        cross=cross,
        solver=solver,
        timer=timer,
        threads=threads,
    )
    result = result_path(
        binary=binary,
        cipher=cipher,
        rounds=rounds,
        activebits=activebits,
        cross=cross,
        solver=solver,
    )
    run_dir = result.parent

    if dry_run:
        return {
            "cross": cross,
            "solver": solver,
            "command": command,
            "result_path": str(result),
            "status": "DRY_RUN",
        }

    run_dir.mkdir(parents=True, exist_ok=True)
    if not binary.exists():
        error_report = {
            "status": "ERROR",
            "cross": cross,
            "solver": solver,
            "command": command,
            "result_path": str(result),
            "reason": f"EasyBC binary does not exist: {binary}",
        }
        (run_dir / "report.json").write_text(
            json.dumps(error_report, indent=2) + "\n",
            encoding="utf-8",
        )
        return error_report

    with (run_dir / "run.log").open("w", encoding="utf-8") as log:
        completed = subprocess.run(
            command,
            cwd=binary.resolve().parent,
            stdout=log,
            stderr=subprocess.STDOUT,
            text=True,
            check=False,
        )
    run_log_text = (run_dir / "run.log").read_text(
        encoding="utf-8", errors="replace"
    )

    if not result.exists():
        error_report = {
            "status": "ERROR",
            "cross": cross,
            "solver": solver,
            "exit_code": completed.returncode,
            "command": command,
            "result_path": str(result),
            "reason": "EasyBC did not produce result.txt",
        }
        (run_dir / "report.json").write_text(
            json.dumps(error_report, indent=2) + "\n",
            encoding="utf-8",
        )
        return error_report

    fingerprints = {
        lp_path.name: fingerprint_lp(lp_path)
        for lp_path in sorted(run_dir.glob("*.lp"))
    }
    (run_dir / "model_fingerprints.json").write_text(
        json.dumps(fingerprints, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )

    report = generate_report(
        result_path=result,
        golden_path=golden,
        output_dir=run_dir,
        metadata={
            "cross": cross,
            "solver": solver,
            "exit_code": completed.returncode,
            "command": command,
            "result_path": str(result),
            "environment": {
                "captured_utc": datetime.now(timezone.utc).isoformat(),
                "platform": platform.platform(),
                "python": sys.version,
                "binary": str(binary),
                "timer_seconds": timer,
                "threads": threads,
                "gurobi_home": os.environ.get("GUROBI_HOME", ""),
                "gurobi_version": extract_gurobi_version(run_log_text),
            },
            "model_fingerprints": fingerprints,
        },
    )
    (run_dir / "report.json").write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return report


def compute_matrix_diffs(reports: list[dict]) -> dict:
    if not reports:
        return {}
    baseline = reports[0].get("mt_reachable", {})
    diffs = {}
    all_models = sorted(
        {
            model
            for report in reports
            for model in report.get("mt_reachable", {})
        }
    )
    for report in reports:
        name = f"{report.get('cross', '')}/{report.get('solver', '')}"
        model_diffs = {}
        for model in all_models:
            baseline_set = set(baseline.get(model, []))
            actual_set = set(report.get("mt_reachable", {}).get(model, []))
            model_diffs[model] = {
                "missing_vs_baseline": sorted(baseline_set - actual_set),
                "extra_vs_baseline": sorted(actual_set - baseline_set),
            }
        diffs[name] = model_diffs
    return diffs


def exit_code_for_reports(mode: str, reports: list[dict]) -> int:
    if mode == "matrix":
        accepted = {"PASS", "MISMATCH", "DRY_RUN"}
    else:
        accepted = {"PASS", "DRY_RUN"}
    return 0 if all(report.get("status") in accepted for report in reports) else 1


def write_matrix_summary(reports: list[dict], output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    diffs = compute_matrix_diffs(reports)
    (output_dir / "matrix.json").write_text(
        json.dumps(
            {"reports": reports, "mt_diffs_vs_paper_per_bit": diffs},
            indent=2,
            sort_keys=True,
        )
        + "\n",
        encoding="utf-8",
    )
    fields = [
        "cross",
        "solver",
        "status",
        "complete",
        "n_balanced",
        "exit_code",
        "result_path",
    ]
    with (output_dir / "matrix.csv").open(
        "w", newline="", encoding="utf-8"
    ) as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=fields)
        writer.writeheader()
        for report in reports:
            writer.writerow({field: report.get(field, "") for field in fields})

    rows = [
        "# BDPT 四组合复现实验",
        "",
        "| cross | solver | status | complete | NBB |",
        "|---|---|---:|---:|---:|",
    ]
    for report in reports:
        rows.append(
            "| {cross} | {solver} | {status} | {complete} | {n_balanced} |".format(
                cross=report.get("cross", ""),
                solver=report.get("solver", ""),
                status=report.get("status", ""),
                complete=report.get("complete", ""),
                n_balanced=report.get("n_balanced", ""),
            )
        )
    rows.extend(["", "## 各 M_t 相对 paper/per-bit 的集合差异", ""])
    for combination, model_diffs in diffs.items():
        rows.append(f"### {combination}")
        rows.append("")
        for model, diff in model_diffs.items():
            rows.append(
                f"- {model}: missing={diff['missing_vs_baseline']}, "
                f"extra={diff['extra_vs_baseline']}"
            )
        rows.append("")
    rows.append("")
    (output_dir / "matrix.md").write_text(
        "\n".join(rows), encoding="utf-8"
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", type=Path, default=ROOT / "build" / "EasyBC")
    parser.add_argument("--mode", choices=("baseline", "matrix"), default="baseline")
    parser.add_argument("--cipher", default="PRESENT")
    parser.add_argument("--reduction", type=int, default=2)
    parser.add_argument("--rounds", type=int, default=9)
    parser.add_argument("--activebits", default="63")
    parser.add_argument("--timer", type=int, default=86400)
    parser.add_argument("--threads", type=int, default=8)
    parser.add_argument("--golden", type=Path, default=DEFAULT_GOLDEN)
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    binary = args.binary.resolve()
    combinations = MATRIX if args.mode == "matrix" else [MATRIX[0]]
    reports = [
        run_one(
            binary=binary,
            cipher=args.cipher,
            reduction=args.reduction,
            rounds=args.rounds,
            activebits=args.activebits,
            cross=cross,
            solver=solver,
            timer=args.timer,
            threads=args.threads,
            golden=args.golden,
            dry_run=args.dry_run,
        )
        for cross, solver in combinations
    ]

    summary_dir = (
        binary.parent.parent
        / "data"
        / "division"
        / args.cipher
        / "repro"
        / "subset3"
    )
    write_matrix_summary(reports, summary_dir)
    print(json.dumps(reports, indent=2, sort_keys=True))
    return exit_code_for_reports(args.mode, reports)


if __name__ == "__main__":
    raise SystemExit(main())
