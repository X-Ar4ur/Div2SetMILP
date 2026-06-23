#include "division/BdptMILPcons.h"

#include <cassert>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static std::string readAll(const std::string& path) {
    std::ifstream input(path);
    std::ostringstream text;
    text << input.rdbuf();
    return text.str();
}

int main() {
    const std::string path = "bdpt_cross_paper_test.lp";
    std::remove(path.c_str());

    BdptMILPcons::bdptCrossPaperC(
        path,
        std::vector<int>{3, 4},
        std::vector<int>{1, 2});

    const std::string emitted = readAll(path);
    std::remove(path.c_str());

    assert(emitted.find("x1 + x2 <= 1\n") != std::string::npos);
    assert(emitted.find("x3 - x1 >= 0\n") != std::string::npos);
    assert(emitted.find("x4 - x2 >= 0\n") != std::string::npos);
    assert(emitted.find("d1") == std::string::npos);
    assert(emitted.find(" = 1\n") == std::string::npos);

    int paperFeasible = 0;
    int exactFeasible = 0;
    for (int l = 0; l < 4; ++l) {
        for (int k = 0; k < 4; ++k) {
            const bool paper = l != 3 && ((k | l) == k);
            const bool exact = paper && __builtin_popcount(k ^ l) == 1;
            if (paper) paperFeasible++;
            if (exact) exactFeasible++;
            assert(!exact || paper);
        }
    }
    assert(paperFeasible == 8);
    assert(exactFeasible == 4);

    return 0;
}
