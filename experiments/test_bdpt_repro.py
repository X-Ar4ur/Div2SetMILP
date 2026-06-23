import importlib.util
import json
import sys
import tempfile
import unittest
from pathlib import Path


HERE = Path(__file__).resolve().parent
MODULE_PATH = HERE / "bdpt_repro.py"

spec = importlib.util.spec_from_file_location("bdpt_repro", MODULE_PATH)
bdpt_repro = importlib.util.module_from_spec(spec)
assert spec.loader is not None
sys.modules[spec.name] = bdpt_repro
spec.loader.exec_module(bdpt_repro)


PAPER_F9_PATTERN = "???0???0???00000???0???0???00000" * 2
INTERNAL_BALANCED = {
    0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 13, 14, 15,
    16, 20, 24, 28, 32, 36, 40, 44,
    48, 52, 56, 60,
}


class BdptReproductionTests(unittest.TestCase):
    def test_present_internal_coordinates_map_exactly_to_appendix_f9(self):
        expected = bdpt_repro.zero_positions(PAPER_F9_PATTERN)
        actual = bdpt_repro.paper_positions("PRESENT", INTERNAL_BALANCED)
        self.assertEqual(actual, expected)
        self.assertEqual(len(actual), 28)

    def test_complete_exact_match_is_pass(self):
        result = bdpt_repro.evaluate_reproduction(
            cipher="PRESENT",
            complete=True,
            internal_balanced=INTERNAL_BALANCED,
            sum0=INTERNAL_BALANCED,
            sum1=set(),
            unresolved=set(),
            expected_pattern=PAPER_F9_PATTERN,
        )
        self.assertEqual(result["status"], "PASS")
        self.assertEqual(result["n_balanced"], 28)
        self.assertEqual(result["missing_paper_positions"], [])
        self.assertEqual(result["extra_paper_positions"], [])

    def test_incomplete_solver_state_never_passes(self):
        result = bdpt_repro.evaluate_reproduction(
            cipher="PRESENT",
            complete=False,
            internal_balanced=INTERNAL_BALANCED,
            sum0=INTERNAL_BALANCED,
            sum1=set(),
            unresolved=set(),
            expected_pattern=PAPER_F9_PATTERN,
        )
        self.assertEqual(result["status"], "INCOMPLETE")

    def test_completed_coordinate_difference_is_mismatch(self):
        result = bdpt_repro.evaluate_reproduction(
            cipher="PRESENT",
            complete=True,
            internal_balanced=INTERNAL_BALANCED - {0},
            sum0=INTERNAL_BALANCED - {0},
            sum1=set(),
            unresolved=set(),
            expected_pattern=PAPER_F9_PATTERN,
        )
        self.assertEqual(result["status"], "MISMATCH")

    def test_result_parser_reads_completeness_and_coordinate_sets(self):
        parsed = bdpt_repro.parse_easybc_result(
            "\n".join(
                [
                    (
                        "M_1 (cross@round 2): reachable unit coords = {1,3}  (2) "
                        "complete=1 solves=2 solver_seconds=0.25 status=2 "
                        "coord_statuses={0:2,1:3}"
                    ),
                    (
                        "M_2 (cross@round 3): reachable unit coords = {}  (0) "
                        "complete=0 solves=2 solver_seconds=1.5 status=9 "
                        "reason=time-limit coord_statuses={0:3,1:9}"
                    ),
                    "Reproduction completeness: COMPLETE",
                    "Balanced bits (NBB) (2): {0,4}",
                    "  - sum=0 coords (2): {0,4}",
                    "  - sum=1 coords (0): {}",
                    "  - 'b' (sign unresolved) coords (0): {}",
                ]
            )
        )
        self.assertTrue(parsed["complete"])
        self.assertEqual(parsed["internal_balanced"], {0, 4})
        self.assertEqual(parsed["sum0"], {0, 4})
        self.assertEqual(parsed["sum1"], set())
        self.assertEqual(parsed["unresolved"], set())
        self.assertEqual(
            parsed["mt_reachable"],
            {"M_1": {1, 3}, "M_2": set()},
        )
        self.assertEqual(parsed["mt_details"]["M_1"]["status"], 2)
        self.assertEqual(parsed["mt_details"]["M_1"]["solve_count"], 2)
        self.assertTrue(parsed["mt_details"]["M_1"]["complete"])
        self.assertEqual(
            parsed["mt_details"]["M_2"]["coordinate_statuses"],
            {0: 3, 1: 9},
        )

    def test_generate_report_writes_json_csv_and_markdown(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            result_path = root / "result.txt"
            golden_path = root / "golden.json"
            output_dir = root / "report"
            result_path.write_text(
                "\n".join(
                    [
                        "Reproduction completeness: COMPLETE",
                        "Balanced bits (NBB) (28): {"
                        + ",".join(map(str, sorted(INTERNAL_BALANCED)))
                        + "}",
                        "  - sum=0 coords (28): {"
                        + ",".join(map(str, sorted(INTERNAL_BALANCED)))
                        + "}",
                        "  - sum=1 coords (0): {}",
                        "  - 'b' (sign unresolved) coords (0): {}",
                    ]
                ),
                encoding="utf-8",
            )
            golden_path.write_text(
                json.dumps(
                    {
                        "cipher": "PRESENT",
                        "pattern": PAPER_F9_PATTERN,
                        "n_balanced": 28,
                    }
                ),
                encoding="utf-8",
            )

            report = bdpt_repro.generate_report(
                result_path=result_path,
                golden_path=golden_path,
                output_dir=output_dir,
                metadata={"cross": "paper", "solver": "per-bit"},
            )

            self.assertEqual(report["status"], "PASS")
            self.assertEqual(report["cross"], "paper")
            self.assertEqual(report["solver"], "per-bit")
            self.assertTrue((output_dir / "report.json").exists())
            self.assertTrue((output_dir / "report.csv").exists())
            self.assertTrue((output_dir / "report.md").exists())
            self.assertIn(
                "PASS",
                (output_dir / "report.md").read_text(encoding="utf-8"),
            )
            self.assertIn(
                "paper/per-bit",
                (output_dir / "report.md").read_text(encoding="utf-8"),
            )


if __name__ == "__main__":
    unittest.main()
