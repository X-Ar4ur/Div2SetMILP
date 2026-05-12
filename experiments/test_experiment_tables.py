import csv
import importlib.util
import tempfile
import unittest
from pathlib import Path


HERE = Path(__file__).resolve().parent


def load_module(name: str, path: Path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


bench = load_module("bench", HERE / "bench.py")
make_tables = load_module("make_tables", HERE / "make_tables.py")


class ExperimentTableTests(unittest.TestCase):
    def test_balanced_bits_are_output_bits_minus_set_zero(self):
        with tempfile.TemporaryDirectory() as td:
            result_path = Path(td) / "result.txt"
            result_path.write_text(
                "\n".join(
                    [
                        "Output bits: x1,x2,x3,x4",
                        "Those are the coordinates set to zero:",
                        "x1",
                        "x3",
                        "Balanced bits: x2,x4",
                    ]
                )
            )

            parsed = make_tables.parse_result_file(result_path)

            self.assertEqual(parsed["output_bits"], ["x1", "x2", "x3", "x4"])
            self.assertEqual(parsed["set_zero"], ["x1", "x3"])
            self.assertEqual(parsed["balanced_bits"], ["x2", "x4"])

    def test_expand_runs_supports_round_range(self):
        cfg = {
            "defaults": {"reduction": 1, "threads": 8, "repeat": 1},
            "runs": [
                {
                    "cipher": "PRESENT",
                    "activebits": 60,
                    "round_start": 1,
                    "round_end": 3,
                }
            ],
        }

        runs = bench.expand_runs(cfg)

        self.assertEqual([r["rounds"] for r in runs], [1, 2, 3])

    def test_round_sweep_table_has_found_and_time_rows(self):
        rows = [
            {
                "cipher": "PRESENT",
                "rounds": "1",
                "activebits": "60",
                "reduction": "1",
                "threads": "8",
                "trial": "1",
                "gurobi_status": "2",
                "distinguisher_found": "1",
                "solve_ms": "1000",
                "total_ms": "1100",
                "exit_code": "0",
            },
            {
                "cipher": "PRESENT",
                "rounds": "2",
                "activebits": "60",
                "reduction": "1",
                "threads": "8",
                "trial": "1",
                "gurobi_status": "9",
                "distinguisher_found": "0",
                "solve_ms": "86400000",
                "total_ms": "86400000",
                "exit_code": "0",
            },
        ]

        table = make_tables.table_round_sweep(rows)

        self.assertIn("PRESENT", table)
        self.assertIn("Found", table)
        self.assertIn("Time", table)
        self.assertIn(r"\checkmark", table)
        self.assertIn("T/O", table)

    def test_performance_table_can_include_manual_external_time(self):
        rows = [
            {
                "cipher": "PRESENT",
                "rounds": "9",
                "activebits": "60",
                "reduction": "1",
                "threads": "8",
                "total_ms": "7400",
                "solve_ms": "6800",
                "gurobi_status": "2",
                "exit_code": "0",
            }
        ]
        external = {
            ("PRESENT", "9", "60"): {
                "external_time": "12.0",
                "notes": "manual baseline",
            }
        }

        table = make_tables.table_performance_comparison(rows, external)

        self.assertIn("PRESENT", table)
        self.assertIn("7.40", table)
        self.assertIn("12.0", table)
        self.assertIn("1.62", table)


if __name__ == "__main__":
    unittest.main()
