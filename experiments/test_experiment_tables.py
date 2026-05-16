import csv
import json
import importlib.util
import sys
import tempfile
import types
import unittest
from pathlib import Path


HERE = Path(__file__).resolve().parent


def load_module(name: str, path: Path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


sys.modules.setdefault("yaml", types.SimpleNamespace(safe_load=lambda _f: {}))
bench = load_module("bench", HERE / "bench.py")
make_tables = load_module("make_tables", HERE / "make_tables.py")
active_search = load_module("active_search", HERE / "active_search.py")


class ExperimentTableTests(unittest.TestCase):
    def test_active_mask_uses_big_endian_x_variable_order(self):
        mask = active_search.mask_from_active_vars(8, [8, 7, 4, 1])

        self.assertEqual(mask.hex_spec, "hex:c9")
        self.assertEqual(mask.weight, 4)
        self.assertEqual(mask.inactive_positions, [6, 5, 3, 2])

    def test_default_mask_matches_present_integer_activebits(self):
        mask = active_search.default_mask("PRESENT", 64, 60)

        self.assertEqual(mask.hex_spec, "hex:fffffffffffffff0")
        self.assertEqual(mask.weight, 60)
        self.assertEqual(mask.inactive_positions, [4, 3, 2, 1])

    def test_search_config_expands_published_and_next_round(self):
        cfg = {
            "defaults": {"reduction": 1, "threads": 8, "repeat": 1},
            "runs": [
                {
                    "cipher": "PRESENT",
                    "block_size": 64,
                    "published_round": 9,
                    "weight": 60,
                    "strategies": ["published_variants"],
                }
            ],
        }

        runs = active_search.expand_search_runs(cfg)

        self.assertEqual(sorted({r["rounds"] for r in runs}), [9, 10])
        self.assertTrue(all(r["activebits"].startswith("hex:") for r in runs))
        self.assertTrue(all(r["weight"] == 60 for r in runs))

    def test_perf_config_is_single_cipher_round_sweep(self):
        cfg_text = (HERE / "configs" / "perf.yaml").read_text()

        self.assertIn("round_start", cfg_text)
        self.assertIn("round_end", cfg_text)
        self.assertIn("cipher: PRESENT", cfg_text)
        self.assertNotIn("cipher: SIMON32", cfg_text)
        self.assertNotIn("cipher: TWINE", cfg_text)

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

    def test_hex_activebits_uses_safe_result_filename(self):
        self.assertEqual(bench.safe_activebits_id("hex:c9"), "hex_c9")
        path = bench.result_path_for("PRESENT", 9, "hex:c9")

        self.assertEqual(path.name, "result_9_hex_c9.txt")

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

    def test_perf_table_reports_single_cipher_round_metrics(self):
        rows = [
            {
                "cipher": "PRESENT",
                "rounds": "1",
                "activebits": "60",
                "reduction": "1",
                "threads": "8",
                "build_ms": "123",
                "model_mem_kib": "",
                "n_vars": "200",
                "n_cons": "300",
                "total_ms": "7400",
                "solve_mem_kib": "",
                "n_iter": "2",
            }
        ]

        table = make_tables.table_performance_metrics(rows, cipher="PRESENT")

        self.assertIn(r"R & T\_m(ms) & M\_m(kiB) & N\_v & N\_c & T\_s(ms) & M\_s(kiB) & Iter.", table)
        self.assertIn("1 & 123 & -- & 200 & 300 & 7400 & -- & 2", table)

    def test_perf_table_requires_cipher_when_multiple_ciphers_present(self):
        rows = [
            {"cipher": "PRESENT", "rounds": "1", "activebits": "60"},
            {"cipher": "TWINE", "rounds": "1", "activebits": "60"},
        ]

        with self.assertRaisesRegex(ValueError, "PRESENT, TWINE"):
            make_tables.table_performance_metrics(rows)

    def test_correctness_table_reports_times_and_ref_last(self):
        rows = [
            {
                "cipher": "PRESENT",
                "rounds": "9",
                "activebits": "60",
                "paper_ref": "Xiang2016 Table 1",
                "balanced_bits": "x1",
                "total_ms": "7400",
            }
        ]

        with tempfile.TemporaryDirectory() as td:
            old_golden_dir = make_tables.GOLDEN_DIR
            make_tables.GOLDEN_DIR = Path(td)
            try:
                (Path(td) / "PRESENT_R9_60.json").write_text(
                    json.dumps(
                        {
                            "cipher": "PRESENT",
                            "rounds": 9,
                            "activebits": "60",
                            "paper_ref": "Xiang2016 Table 1",
                            "balanced_bits": [],
                            "n_balanced": 1,
                            "xiang_time_s": "12.0",
                        }
                    )
                )

                table = make_tables.table1_correctness(rows)
            finally:
                make_tables.GOLDEN_DIR = old_golden_dir

        self.assertIn(r"|Bal|$_p$", table)
        self.assertIn(r"T\_Xiang(s)", table)
        self.assertIn(r"T\_EasyBC(s)", table)
        self.assertIn("PRESENT & 9 & 60 & 1 & 1 & 12.0 & 7.40 & Xiang2016 Table 1", table)
        header = next(line for line in table.splitlines() if line.startswith("Cipher &"))
        self.assertTrue(header.endswith(r"Ref. \\"))
        self.assertNotIn("Status", table)
        self.assertNotIn("Match", table)
        self.assertNotIn("Missing", table)
        self.assertNotIn("Extra", table)


if __name__ == "__main__":
    unittest.main()
