import importlib.util
import sys
import tempfile
import unittest
from pathlib import Path


HERE = Path(__file__).resolve().parent
MODULE_PATH = HERE / "run_bdpt_repro.py"

spec = importlib.util.spec_from_file_location("run_bdpt_repro", MODULE_PATH)
run_bdpt_repro = importlib.util.module_from_spec(spec)
assert spec.loader is not None
sys.modules[spec.name] = run_bdpt_repro
spec.loader.exec_module(run_bdpt_repro)


class BdptRunnerTests(unittest.TestCase):
    def test_baseline_command_is_paper_per_bit_strict(self):
        binary = Path("C:/srv/easybc/build/EasyBC")
        command = run_bdpt_repro.build_command(
            binary=binary,
            cipher="PRESENT",
            reduction=2,
            rounds=9,
            activebits="63",
            cross="paper",
            solver="per-bit",
            timer=86400,
            threads=8,
        )
        self.assertEqual(
            command,
            [
                str(binary),
                "-div3",
                "PRESENT",
                "2",
                "9",
                "63",
                "cross",
                "paper",
                "solver",
                "per-bit",
                "sign",
                "1",
                "repro",
                "1",
                "timer",
                "86400",
                "threads",
                "8",
            ],
        )

    def test_matrix_contains_all_four_orthogonal_combinations(self):
        self.assertEqual(
            run_bdpt_repro.MATRIX,
            [
                ("paper", "per-bit"),
                ("paper", "min-pin"),
                ("exact", "per-bit"),
                ("exact", "min-pin"),
            ],
        )

    def test_result_path_follows_binary_working_directory(self):
        binary = Path("C:/srv/easybc/build/EasyBC")
        path = run_bdpt_repro.result_path(
            binary=binary,
            cipher="PRESENT",
            rounds=9,
            activebits="63",
            cross="paper",
            solver="per-bit",
        )
        self.assertEqual(
            path,
            binary.resolve().parent.parent
            / "data"
            / "division"
            / "PRESENT"
            / "repro"
            / "subset3"
            / "9_63_paper_per-bit"
            / "result.txt",
        )

    def test_matrix_diff_compares_each_mt_against_paper_per_bit(self):
        diffs = run_bdpt_repro.compute_matrix_diffs(
            [
                {
                    "cross": "paper",
                    "solver": "per-bit",
                    "mt_reachable": {"M_1": [1, 2], "M_2": []},
                },
                {
                    "cross": "paper",
                    "solver": "min-pin",
                    "mt_reachable": {"M_1": [1, 3], "M_2": []},
                },
            ]
        )
        self.assertEqual(
            diffs["paper/min-pin"]["M_1"],
            {"missing_vs_baseline": [2], "extra_vs_baseline": [3]},
        )
        self.assertEqual(
            diffs["paper/min-pin"]["M_2"],
            {"missing_vs_baseline": [], "extra_vs_baseline": []},
        )

    def test_lp_fingerprint_is_canonical_and_counts_model_size(self):
        with tempfile.TemporaryDirectory() as td:
            lp = Path(td) / "model.lp"
            lp.write_text(
                "\n".join(
                    [
                        "Minimize",
                        "  x1 + x2",
                        "Subject To",
                        "  x1 + x2 >= 1",
                        "  x2 - d1 = 0",
                        "Binary",
                        "  x1",
                        "  x2",
                        "  d1",
                        "End",
                    ]
                ),
                encoding="utf-8",
            )
            fingerprint = run_bdpt_repro.fingerprint_lp(lp)
            self.assertEqual(fingerprint["constraints"], 2)
            self.assertEqual(fingerprint["binary_variables"], 3)
            self.assertEqual(fingerprint["nonzero_terms"], 4)
            self.assertEqual(len(fingerprint["sha256"]), 64)

    def test_matrix_mismatch_is_a_result_but_incomplete_is_a_failure(self):
        self.assertEqual(
            run_bdpt_repro.exit_code_for_reports(
                "matrix", [{"status": "PASS"}, {"status": "MISMATCH"}]
            ),
            0,
        )
        self.assertEqual(
            run_bdpt_repro.exit_code_for_reports(
                "matrix", [{"status": "PASS"}, {"status": "INCOMPLETE"}]
            ),
            1,
        )
        self.assertEqual(
            run_bdpt_repro.exit_code_for_reports(
                "baseline", [{"status": "MISMATCH"}]
            ),
            1,
        )

    def test_gurobi_version_is_extracted_from_run_log(self):
        self.assertEqual(
            run_bdpt_repro.extract_gurobi_version(
                "Gurobi Optimizer version 13.0.2 build v13.0.2rc0\n"
            ),
            "13.0.2 build v13.0.2rc0",
        )

    def test_missing_binary_is_reported_as_error(self):
        with tempfile.TemporaryDirectory() as td:
            root = Path(td)
            binary = root / "build" / "EasyBC"
            report = run_bdpt_repro.run_one(
                binary=binary,
                cipher="PRESENT",
                reduction=2,
                rounds=9,
                activebits="63",
                cross="paper",
                solver="per-bit",
                timer=1,
                threads=1,
                golden=Path("unused.json"),
                dry_run=False,
            )
            self.assertEqual(report["status"], "ERROR")
            self.assertIn("binary", report["reason"].lower())


if __name__ == "__main__":
    unittest.main()
