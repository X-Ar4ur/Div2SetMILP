"""3 子集 BDPT 后端不得包含偏离论文的 oracle/CEGAR 路径。"""

from __future__ import annotations

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class BdptPaperModelTests(unittest.TestCase):
    def test_oracle_cegar_code_is_not_part_of_div3_backend(self) -> None:
        header = (ROOT / "include" / "division" / "Div3SetMILP.h").read_text(
            encoding="utf-8"
        )
        source = (ROOT / "lib" / "division" / "Div3SetMILP.cpp").read_text(
            encoding="utf-8"
        )
        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

        forbidden = [
            "BdptTrailOracle",
            "BdptLocalTransition",
            "localTransitions",
            "loadBdptTrailOracles",
            "validateOracleTransitions",
            "addBdptOracleCut",
            "abstractMay",
            "abstractCandidateCoords",
            "oracle_cuts",
            "possibleOutputs",
            "abstract_candidates",
        ]

        for token in forbidden:
            self.assertNotIn(token, header)
            self.assertNotIn(token, source)
            self.assertNotIn(token, cmake)


if __name__ == "__main__":
    unittest.main()
