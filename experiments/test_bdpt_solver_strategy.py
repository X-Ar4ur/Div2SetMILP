"""3 子集 BDPT 求解策略的论文范式回归测试。"""

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

    def test_min_pin_query_excludes_zero_output_vector(self) -> None:
        source = (ROOT / "lib" / "division" / "Div3SetMILP.cpp").read_text(
            encoding="utf-8"
        )

        self.assertIn("sumRemainingExpr", source)
        self.assertIn(">= 1", source)

    def test_algorithm4_ml_parity_is_part_of_production_path(self) -> None:
        header = (ROOT / "include" / "division" / "Div3SetMILP.h").read_text(
            encoding="utf-8"
        )
        source = (ROOT / "lib" / "division" / "Div3SetMILP.cpp").read_text(
            encoding="utf-8"
        )

        self.assertIn("buildMLModel", header)
        self.assertIn("classifyMLParity", header)
        self.assertIn("void Div3SetMILP::buildMLModel", source)
        self.assertIn("int Div3SetMILP::classifyMLParity", source)
        search_body = source[source.index("void Div3SetMILP::searchDistinguisher") :]
        self.assertIn("buildMLModel", search_body)
        self.assertIn("classifyMLParity", search_body)

    def test_cross_constraints_use_paper_algorithm3_only(self) -> None:
        source = (ROOT / "lib" / "division" / "Div3SetMILP.cpp").read_text(
            encoding="utf-8"
        )
        header = (ROOT / "include" / "division" / "BdptMILPcons.h").read_text(
            encoding="utf-8"
        )

        self.assertIn("bdptCrossPaperC", source)
        self.assertNotIn("bdptCrossExactOneFlipC", source)
        self.assertNotIn("bdptCrossWeightIncrementC", source)
        self.assertNotIn("bdptCrossExactOneFlipC", header)
        self.assertNotIn("bdptCrossWeightIncrementC", header)


if __name__ == "__main__":
    unittest.main()
