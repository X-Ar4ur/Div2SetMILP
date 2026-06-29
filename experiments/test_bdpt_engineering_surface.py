"""Guards for keeping -div3 as an automatic engineering backend."""

from __future__ import annotations

import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class BdptEngineeringSurfaceTests(unittest.TestCase):
    def test_no_reproduction_or_strategy_cli_surface(self) -> None:
        main_cpp = (ROOT / "main.cpp").read_text(encoding="utf-8")
        header = (ROOT / "include" / "division" / "Div3SetMILP.h").read_text(
            encoding="utf-8"
        )
        source = (ROOT / "lib" / "division" / "Div3SetMILP.cpp").read_text(
            encoding="utf-8"
        )

        forbidden = [
            "BDPT_SOLVER",
            "setSignLabeling",
            "setReproduction",
            "setCrossMode",
            "setUnitSearchMode",
            "crossMode",
            "unitSearchMode",
            "signLabeling",
            "reproduction",
        ]
        for token in forbidden:
            self.assertNotIn(token, main_cpp)
            self.assertNotIn(token, header)
            self.assertNotIn(token, source)


if __name__ == "__main__":
    unittest.main()
