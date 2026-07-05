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

    def test_div3_uses_exact_copy_on_read_like_div2(self) -> None:
        header = (ROOT / "include" / "division" / "Div3SetMILP.h").read_text(
            encoding="utf-8"
        )
        source = (ROOT / "lib" / "division" / "Div3SetMILP.cpp").read_text(
            encoding="utf-8"
        )

        self.assertNotIn("lazyCopyEnabled", header)
        self.assertNotIn("lazyCopyEnabled", source)
        self.assertIn("DivMILPcons::divCopyC", source)
        self.assertNotIn("if (!this->lazyCopyEnabled) return live;", source)

    def test_div3_keyxor_cross_uses_current_live_tail(self) -> None:
        source = (ROOT / "lib" / "division" / "Div3SetMILP.cpp").read_text(
            encoding="utf-8"
        )
        selected_cross = source[
            source.index("if (this->selectedCrossLayer >= 0 &&") :
            source.index("} else {", source.index("if (this->selectedCrossLayer >= 0 &&"))
        ]

        self.assertIn(
            "while (this->liveChain.count(lIdx)) lIdx = this->liveChain[lIdx];",
            selected_cross,
        )

    def test_div3_keyxor_cross_l_tails_are_not_pinned_dead(self) -> None:
        header = (ROOT / "include" / "division" / "Div3SetMILP.h").read_text(
            encoding="utf-8"
        )
        source = (ROOT / "lib" / "division" / "Div3SetMILP.cpp").read_text(
            encoding="utf-8"
        )

        self.assertIn("protectedTailIndices", header)
        self.assertIn("protectedTailIndices.clear()", source)
        self.assertIn("protectedTailIndices.insert(idx)", source)

        tail_pinning = source[
            source.index("for (int tail : chainValues)") :
            source.index("modelApp.close()", source.index("for (int tail : chainValues)"))
        ]
        self.assertIn("this->protectedTailIndices.count(tail)", tail_pinning)


if __name__ == "__main__":
    unittest.main()
