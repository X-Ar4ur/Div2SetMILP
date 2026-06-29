"""Regression tests for semantic metadata on discovered BDPT Key-XOR layers."""

from __future__ import annotations

import shutil
import subprocess
import tempfile
import textwrap
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


class BdptKeyXorSemanticTests(unittest.TestCase):
    @unittest.skipIf(shutil.which("g++") is None, "g++ is required for this Key-XOR test")
    def test_discovered_layers_record_round_core_boundary(self) -> None:
        source = textwrap.dedent(
            r"""
            #include "ProcedureH.h"
            #include "division/BdptKeyXor.h"

            #include <cassert>
            #include <map>
            #include <memory>
            #include <string>
            #include <vector>

            namespace {

            ThreeAddressNodePtr node(const std::string& name,
                                     ASTNode::Operator op = ASTNode::NULLOP,
                                     NodeType type = UINT1,
                                     ThreeAddressNodePtr lhs = nullptr,
                                     ThreeAddressNodePtr rhs = nullptr) {
                return std::make_shared<ThreeAddressNode>(name, lhs, rhs, op, type);
            }

            } // namespace

            int main() {
                auto roundParam = node("r", ASTNode::NULLOP, UINT8);
                auto key0 = node("key0");
                auto key1 = node("key1");
                auto in0 = node("in0");
                auto in1 = node("in1");
                auto mid0 = node("mid0");
                auto mid1 = node("mid1");
                auto tmp0 = node("tmp0");
                auto tmp1 = node("tmp1");

                std::vector<std::vector<ThreeAddressNodePtr>> roundParams = {
                    {roundParam},
                    {key0, key1},
                    {in0, in1},
                };
                std::vector<ThreeAddressNodePtr> roundBlock = {
                    node("kx0", ASTNode::XOR, UINT1, in0, key0),
                    node("kx1", ASTNode::XOR, UINT1, in1, key1),
                    node("mix0", ASTNode::XOR, UINT1, tmp0, tmp1),
                    node("kx2", ASTNode::XOR, UINT1, mid0, key0),
                    node("kx3", ASTNode::XOR, UINT1, mid1, key1),
                };
                auto roundProc = std::make_shared<ProcedureH>(
                    "round_function", roundParams, roundBlock,
                    std::vector<ThreeAddressNodePtr>{},
                    std::map<std::string, ProcedureHPtr>{});

                auto roundNumber = node("1", ASTNode::NULLOP, UINT);
                auto roundMarker = node("round_marker", ASTNode::NULLOP, UINT1, roundNumber);
                auto callTarget = node("round_function@0", ASTNode::NULLOP, FUNCTION);
                auto call = node("call_round", ASTNode::CALL, FUNCTION, callTarget);
                auto mainProc = std::make_shared<ProcedureH>(
                    "main",
                    std::vector<std::vector<ThreeAddressNodePtr>>{},
                    std::vector<ThreeAddressNodePtr>{roundMarker, call},
                    std::vector<ThreeAddressNodePtr>{},
                    std::map<std::string, ProcedureHPtr>{});

                std::vector<ProcedureHPtr> procedures = {mainProc, roundProc};
                const std::vector<BdptKeyXorLayer> layers =
                    discoverBdptKeyXorLayers(procedures, 1);

                assert(layers.size() == 2);
                assert(layers[0].beforeRoundCore);
                assert(!layers[1].beforeRoundCore);
                return 0;
            }
            """
        )

        with tempfile.TemporaryDirectory() as td:
            td_path = Path(td)
            stub_dir = td_path / "include" / "json"
            stub_dir.mkdir(parents=True)
            (stub_dir / "json.h").write_text(
                """#ifndef EASYBC_TEST_JSON_STUB_H
#define EASYBC_TEST_JSON_STUB_H

#include <string>

namespace Json {
class Value {
public:
    Value() {}
    Value(const char*) {}
    Value(const std::string&) {}

    template <typename T>
    Value(T) {}

    Value& operator[](const char*) { return *this; }
    void append(const Value&) {}
};
} // namespace Json

#endif
""",
                encoding="utf-8",
            )

            test_cpp = td_path / "bdpt_keyxor_semantics_test.cpp"
            exe = td_path / "bdpt_keyxor_semantics_test.exe"
            test_cpp.write_text(source, encoding="utf-8")

            compile_cmd = [
                "g++",
                "-std=c++14",
                "-include",
                "map",
                "-I",
                str(ROOT / "include"),
                "-I",
                str(ROOT / "include" / "preprocessing"),
                "-I",
                str(td_path / "include"),
                str(test_cpp),
                str(ROOT / "lib" / "division" / "BdptKeyXor.cpp"),
                str(ROOT / "lib" / "preprocessing" / "ProcedureH.cpp"),
                "-o",
                str(exe),
            ]
            compiled = subprocess.run(
                compile_cmd,
                cwd=ROOT,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            self.assertEqual(compiled.returncode, 0, msg=compiled.stdout + compiled.stderr)

            completed = subprocess.run(
                [str(exe)],
                cwd=ROOT,
                text=True,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                check=False,
            )
            self.assertEqual(completed.returncode, 0, msg=completed.stdout + completed.stderr)


if __name__ == "__main__":
    unittest.main()
