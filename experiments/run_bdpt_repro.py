"""Run EasyBC's production 3-subset BDPT command and summarize the result."""

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
) -> Path:
    repo_root = binary.resolve().parent.parent
    return (
        repo_root
        / "data"
        / "division"
        / cipher
        / "milp"
        / f"result_{rounds}_{activebits}_subset3.txt"
    )


def run_one(
    *,
    binary: Path,
    cipher: str,
    reduction: int,
    rounds: int,
    activebits: str,
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
        timer=timer,
        threads=threads,
    )
    result = result_path(
        binary=binary,
        cipher=cipher,
        rounds=rounds,
        activebits=activebits,
    )
    run_dir = result.parent

    if dry_run:
        return {
            "command": command,
            "result_path": str(result),
            "status": "DRY_RUN",
        }

    run_dir.mkdir(parents=True, exist_ok=True)
    if not binary.exists():
        error_report = {
            "status": "ERROR",
            "command": command,
            "result_path": str(result),
            "reason": f"EasyBC binary does not exist: {binary}",
        }
        (run_dir / "production_run.json").write_text(
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
            "exit_code": completed.returncode,
            "command": command,
            "result_path": str(result),
            "reason": "EasyBC did not produce the production -div3 result file",
        }
        (run_dir / "production_run.json").write_text(
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
    (run_dir / "production_run.json").write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return report


def exit_code_for_reports(reports: list[dict]) -> int:
    accepted = {"PASS", "DRY_RUN"}
    return 0 if all(report.get("status") in accepted for report in reports) else 1


def write_summary(reports: list[dict], output_dir: Path) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    fields = ["status", "complete", "n_balanced", "exit_code", "result_path"]
    with (output_dir / "production_run.csv").open(
        "w", newline="", encoding="utf-8"
    ) as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=fields)
        writer.writeheader()
        for report in reports:
            writer.writerow({field: report.get(field, "") for field in fields})

    rows = [
        "# BDPT production run",
        "",
        "| status | complete | NBB | result |",
        "|---:|---:|---:|---|",
    ]
    for report in reports:
        rows.append(
            "| {status} | {complete} | {n_balanced} | {result_path} |".format(
                status=report.get("status", ""),
                complete=report.get("complete", ""),
                n_balanced=report.get("n_balanced", ""),
                result_path=report.get("result_path", ""),
            )
        )
    rows.append("")
    (output_dir / "production_run.md").write_text(
        "\n".join(rows), encoding="utf-8"
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--binary", type=Path, default=ROOT / "build" / "EasyBC")
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
    reports = [
        run_one(
            binary=binary,
            cipher=args.cipher,
            reduction=args.reduction,
            rounds=args.rounds,
            activebits=args.activebits,
            timer=args.timer,
            threads=args.threads,
            golden=args.golden,
            dry_run=args.dry_run,
        )
    ]

    summary_dir = (
        binary.parent.parent
        / "data"
        / "division"
        / args.cipher
        / "milp"
    )
    write_summary(reports, summary_dir)
    print(json.dumps(reports, indent=2, sort_keys=True))
    return exit_code_for_reports(reports)


if __name__ == "__main__":
    raise SystemExit(main())
