"""Source-level guards for BDPT oracle-backed certificate refinement."""

from __future__ import annotations

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class BdptOracleIntegrationTests(unittest.TestCase):
    def test_div3_records_and_validates_local_sbox_transitions(self) -> None:
        header = (ROOT / "include" / "division" / "Div3SetMILP.h").read_text(
            encoding="utf-8"
        )
        source = (ROOT / "lib" / "division" / "Div3SetMILP.cpp").read_text(
            encoding="utf-8"
        )

        self.assertIn("BdptTrailOracle.h", header)
        self.assertIn("BdptLocalTransition", header)
        self.assertIn("localTransitions", header)
        self.assertIn("loadBdptTrailOracles", header)
        self.assertIn("validateOracleTransitions", header)
        self.assertIn("abstractMay", header)
        self.assertIn("abstractCandidateCoords", header)

        self.assertIn("loadBdptTrailOracles();", source)
        self.assertIn("localTransitions.push_back", source)
        self.assertIn("validateOracleTransitions(model", source)
        self.assertIn("addBdptOracleCut", source)
        self.assertIn("oracle_cuts", source)
        self.assertIn("possibleOutputs", source)
        self.assertIn("abstract_candidates", source)


if __name__ == "__main__":
    unittest.main()
