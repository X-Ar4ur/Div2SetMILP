"""Regression tests for 3-subset BDPT solver strategy choices."""

from __future__ import annotations

import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class BdptSolverStrategyTests(unittest.TestCase):
    def test_hybrid_solver_does_not_fall_back_to_per_bit(self) -> None:
        source = (ROOT / "lib" / "division" / "Div3SetMILP.cpp").read_text(
            encoding="utf-8"
        )
        match = re.search(
            r"BdptSolveResult Div3SetMILP::solveMtReachableCoordsHybrid"
            r"\([\s\S]*?\n\}\n\n\n// Algorithm 4",
            source,
        )
        self.assertIsNotNone(match)
        hybrid_body = match.group(0)

        self.assertNotIn("solveMtReachableCoordsPerBit", hybrid_body)
        self.assertNotIn("hybrid+per-bit", hybrid_body)


if __name__ == "__main__":
    unittest.main()
