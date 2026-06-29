"""Regression tests for 3-subset BDPT solver strategy choices."""

from __future__ import annotations

import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class BdptSolverStrategyTests(unittest.TestCase):
    def test_dispatcher_uses_single_min_pin_production_path(self) -> None:
        source = (ROOT / "lib" / "division" / "Div3SetMILP.cpp").read_text(
            encoding="utf-8"
        )
        match = re.search(
            r"BdptSolveResult Div3SetMILP::solveMtReachableCoords"
            r"\([\s\S]*?\n\}\n\n\nBdptSolveResult Div3SetMILP::solveMtReachableCoordsMinPin",
            source,
        )
        self.assertIsNotNone(match)
        dispatch_body = match.group(0)

        self.assertIn("return solveMtReachableCoordsMinPin", dispatch_body)
        self.assertNotIn("getenv", dispatch_body)
        self.assertNotIn("solveMtReachableCoordsPerBit", source)
        self.assertNotIn("solveMtReachableCoordsHybrid", source)


if __name__ == "__main__":
    unittest.main()
