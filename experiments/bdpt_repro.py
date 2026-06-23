"""Paper-coordinate mapping and strict BDPT reproduction verdicts."""

from __future__ import annotations

import argparse
import csv
import json
import re
from pathlib import Path
from typing import Iterable


PRESENT_PBOX = (
    0, 16, 32, 48, 1, 17, 33, 49,
    2, 18, 34, 50, 3, 19, 35, 51,
    4, 20, 36, 52, 5, 21, 37, 53,
    6, 22, 38, 54, 7, 23, 39, 55,
    8, 24, 40, 56, 9, 25, 41, 57,
    10, 26, 42, 58, 11, 27, 43, 59,
    12, 28, 44, 60, 13, 29, 45, 61,
    14, 30, 46, 62, 15, 31, 47, 63,
)


def zero_positions(pattern: str) -> set[int]:
    return {position for position, symbol in enumerate(pattern) if symbol == "0"}


def paper_positions(cipher: str, internal_coordinates: Iterable[int]) -> set[int]:
    if cipher != "PRESENT":
        raise ValueError(f"unsupported paper-coordinate mapping for {cipher}")

    inverse_pbox = [0] * len(PRESENT_PBOX)
    for source, destination in enumerate(PRESENT_PBOX):
        inverse_pbox[destination] = source

    return {
        63 - inverse_pbox[coordinate]
        for coordinate in internal_coordinates
    }


def evaluate_reproduction(
    *,
    cipher: str,
    complete: bool,
    internal_balanced: Iterable[int],
    sum0: Iterable[int],
    sum1: Iterable[int],
    unresolved: Iterable[int],
    expected_pattern: str,
) -> dict:
    internal_balanced_set = set(internal_balanced)
    sum0_set = set(sum0)
    sum1_set = set(sum1)
    unresolved_set = set(unresolved)
    actual_positions = paper_positions(cipher, internal_balanced_set)
    expected_positions = zero_positions(expected_pattern)

    missing = sorted(expected_positions - actual_positions)
    extra = sorted(actual_positions - expected_positions)
    signs_complete = (
        sum0_set == internal_balanced_set
        and not sum1_set
        and not unresolved_set
    )

    if not complete:
        status = "INCOMPLETE"
    elif missing or extra or not signs_complete:
        status = "MISMATCH"
    else:
        status = "PASS"

    return {
        "status": status,
        "cipher": cipher,
        "complete": complete,
        "n_balanced": len(internal_balanced_set),
        "internal_balanced": sorted(internal_balanced_set),
        "paper_positions": sorted(actual_positions),
        "expected_paper_positions": sorted(expected_positions),
        "missing_paper_positions": missing,
        "extra_paper_positions": extra,
        "sum0": sorted(sum0_set),
        "sum1": sorted(sum1_set),
        "unresolved": sorted(unresolved_set),
    }


def _parse_coordinate_set(line: str) -> set[int]:
    match = re.search(r"\{([^}]*)\}", line)
    if not match or not match.group(1).strip():
        return set()
    coordinates = set()
    for item in match.group(1).split(","):
        coordinate = re.match(r"\s*(\d+)", item)
        if coordinate:
            coordinates.add(int(coordinate.group(1)))
    return coordinates


def parse_easybc_result(text: str) -> dict:
    parsed = {
        "complete": False,
        "internal_balanced": set(),
        "sum0": set(),
        "sum1": set(),
        "unresolved": set(),
        "mt_reachable": {},
        "mt_details": {},
    }
    for line in text.splitlines():
        stripped = line.strip()
        if stripped == "Reproduction completeness: COMPLETE":
            parsed["complete"] = True
        elif stripped.startswith("Balanced bits (NBB)"):
            parsed["internal_balanced"] = _parse_coordinate_set(stripped)
        elif stripped.startswith("- sum=0 coords"):
            parsed["sum0"] = _parse_coordinate_set(stripped)
        elif stripped.startswith("- sum=1 coords"):
            parsed["sum1"] = _parse_coordinate_set(stripped)
        elif stripped.startswith("- 'b' (sign unresolved) coords"):
            parsed["unresolved"] = _parse_coordinate_set(stripped)
        elif stripped.startswith("M_") and "reachable unit coords" in stripped:
            model_name = stripped.split(" ", 1)[0]
            parsed["mt_reachable"][model_name] = _parse_coordinate_set(stripped)
            summary = re.search(
                r"complete=(\d+)\s+solves=(\d+)\s+"
                r"solver_seconds=([0-9.eE+-]+)\s+status=(-?\d+)",
                stripped,
            )
            detail = {
                "complete": bool(int(summary.group(1))) if summary else False,
                "solve_count": int(summary.group(2)) if summary else 0,
                "solver_seconds": float(summary.group(3)) if summary else 0.0,
                "status": int(summary.group(4)) if summary else 0,
                "coordinate_statuses": {},
                "iteration_statuses": [],
            }
            reason = re.search(r"\sreason=([^\s]+)", stripped)
            if reason:
                detail["reason"] = reason.group(1)
            coordinate_statuses = re.search(
                r"coord_statuses=\{([^}]*)\}", stripped
            )
            if coordinate_statuses and coordinate_statuses.group(1):
                detail["coordinate_statuses"] = {
                    int(coordinate): int(status)
                    for coordinate, status in (
                        item.split(":", 1)
                        for item in coordinate_statuses.group(1).split(",")
                    )
                }
            iteration_statuses = re.search(
                r"iteration_statuses=\{([^}]*)\}", stripped
            )
            if iteration_statuses and iteration_statuses.group(1):
                detail["iteration_statuses"] = [
                    int(status)
                    for status in iteration_statuses.group(1).split(",")
                ]
            parsed["mt_details"][model_name] = detail
    return parsed


def generate_report(
    *,
    result_path: Path,
    golden_path: Path,
    output_dir: Path,
    metadata: dict | None = None,
) -> dict:
    parsed = parse_easybc_result(result_path.read_text(encoding="utf-8"))
    golden = json.loads(golden_path.read_text(encoding="utf-8"))
    report = evaluate_reproduction(
        cipher=golden["cipher"],
        complete=parsed["complete"],
        internal_balanced=parsed["internal_balanced"],
        sum0=parsed["sum0"],
        sum1=parsed["sum1"],
        unresolved=parsed["unresolved"],
        expected_pattern=golden["pattern"],
    )
    report["expected_n_balanced"] = golden["n_balanced"]
    report["paper_ref"] = golden.get("paper_ref", "")
    report["mt_reachable"] = {
        model: sorted(coordinates)
        for model, coordinates in parsed["mt_reachable"].items()
    }
    report["mt_details"] = parsed["mt_details"]
    if metadata:
        report.update(metadata)
    if report["status"] == "PASS" and (
        report["n_balanced"] != golden["n_balanced"]
    ):
        report["status"] = "MISMATCH"
    if report.get("exit_code", 0) != 0:
        report["status"] = "ERROR"
        report["reason"] = "EasyBC returned a non-zero exit code"

    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "report.json").write_text(
        json.dumps(report, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )

    with (output_dir / "report.csv").open(
        "w", newline="", encoding="utf-8"
    ) as csv_file:
        writer = csv.DictWriter(
            csv_file,
            fieldnames=[
                "status",
                "cipher",
                "cross",
                "solver",
                "complete",
                "n_balanced",
                "expected_n_balanced",
                "missing_paper_positions",
                "extra_paper_positions",
                "sum1",
                "unresolved",
            ],
        )
        writer.writeheader()
        writer.writerow(
            {
                key: (
                    ",".join(map(str, report.get(key, [])))
                    if isinstance(report.get(key), list)
                    else report.get(key, "")
                )
                for key in writer.fieldnames
            }
        )

    markdown = "\n".join(
        [
            "# BDPT 论文结果复现报告",
            "",
            f"- 状态：**{report['status']}**",
            f"- 密码：{report['cipher']}",
            (
                f"- 配置：{report.get('cross', 'unknown')}/"
                f"{report.get('solver', 'unknown')}"
            ),
            f"- 求解完整：{'是' if report['complete'] else '否'}",
            (
                f"- Balanced bits：{report['n_balanced']} "
                f"（论文：{report['expected_n_balanced']}）"
            ),
            f"- 缺失论文位置：{report['missing_paper_positions']}",
            f"- 额外论文位置：{report['extra_paper_positions']}",
            f"- sum=1 内部坐标：{report['sum1']}",
            f"- 符号未决内部坐标：{report['unresolved']}",
            f"- 论文出处：{report['paper_ref'] or '未填写'}",
            (
                "- 环境："
                f"{report.get('environment', {}).get('platform', '未记录')}"
            ),
            (
                "- 二进制："
                f"{report.get('environment', {}).get('binary', '未记录')}"
            ),
            (
                "- Gurobi："
                f"{report.get('environment', {}).get('gurobi_version', '未记录')}"
            ),
            f"- 模型指纹数：{len(report.get('model_fingerprints', {}))}",
            "",
        ]
    )
    (output_dir / "report.md").write_text(markdown, encoding="utf-8")
    return report


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Generate a strict BDPT paper-reproduction report."
    )
    parser.add_argument("--result", required=True, type=Path)
    parser.add_argument("--golden", required=True, type=Path)
    parser.add_argument("--out-dir", required=True, type=Path)
    args = parser.parse_args()

    report = generate_report(
        result_path=args.result,
        golden_path=args.golden,
        output_dir=args.out_dir,
    )
    print(json.dumps(report, indent=2, sort_keys=True))
    return 0 if report["status"] == "PASS" else 1


if __name__ == "__main__":
    raise SystemExit(main())
