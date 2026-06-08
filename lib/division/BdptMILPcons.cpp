#include "division/BdptMILPcons.h"
#include <cassert>

void BdptMILPcons::bdptCrossDominanceC(std::string path, int lIdx, int kIdx) {
    std::ofstream scons(path, std::ios::app);
    if (!scons) {
        std::cout << "Wrong file path in bdptCrossDominanceC!" << std::endl;
    } else {
        // k_i >= ell_i  <=>  x{kIdx} - x{lIdx} >= 0
        scons << "x" << kIdx << " - x" << lIdx << " >= 0\n";
    }
    scons.close();
}

void BdptMILPcons::bdptCrossNotAllOneC(std::string path, const std::vector<int>& lIndices) {
    if (lIndices.empty()) return;
    std::ofstream scons(path, std::ios::app);
    if (!scons) {
        std::cout << "Wrong file path in bdptCrossNotAllOneC!" << std::endl;
    } else {
        // ell_0 + ell_1 + ... + ell_{s-1} <= s - 1
        for (int i = 0; i < (int)lIndices.size(); ++i) {
            if (i > 0) scons << " + ";
            scons << "x" << lIndices[i];
        }
        scons << " <= " << (int)lIndices.size() - 1 << "\n";
    }
    scons.close();
}

void BdptMILPcons::bdptCrossWeightIncrementC(std::string path,
                                             const std::vector<int>& kIndices,
                                             const std::vector<int>& lIndices) {
    if (kIndices.empty() || lIndices.empty()) return;
    std::ofstream scons(path, std::ios::app);
    if (!scons) {
        std::cout << "Wrong file path in bdptCrossWeightIncrementC!" << std::endl;
    } else {
        // Sum(k_i^t*) - Sum(ell_i^t) = 1  : K_t* = ell ∨ e_j (exactly one more bit)
        for (int i = 0; i < (int)kIndices.size(); ++i) {
            if (i > 0) scons << " + ";
            scons << "x" << kIndices[i];
        }
        for (int i = 0; i < (int)lIndices.size(); ++i) {
            scons << " - x" << lIndices[i];
        }
        scons << " = 1\n";
    }
    scons.close();
}

void BdptMILPcons::bdptCrossExactOneFlipC(std::string path,
                                          const std::vector<int>& kIndices,
                                          const std::vector<int>& lIndices,
                                          int& dCounter) {
    if (kIndices.empty() || lIndices.empty()) return;
    assert(kIndices.size() == lIndices.size());

    std::ofstream scons(path, std::ios::app);
    if (!scons) {
        std::cout << "Wrong file path in bdptCrossExactOneFlipC!" << std::endl;
    } else {
        std::vector<int> selectors;
        selectors.reserve(lIndices.size());
        for (size_t i = 0; i < lIndices.size(); ++i) {
            selectors.push_back(dCounter++);
        }

        // Exactly one zero position of L_t is selected.
        for (size_t i = 0; i < selectors.size(); ++i) {
            if (i > 0) scons << " + ";
            scons << "d" << selectors[i];
        }
        scons << " = 1\n";

        // A selected position must be zero in L_t.
        for (size_t i = 0; i < selectors.size(); ++i) {
            scons << "d" << selectors[i] << " + x" << lIndices[i] << " <= 1\n";
        }

        // K_t* is L_t with exactly the selected bit flipped to one.
        for (size_t i = 0; i < selectors.size(); ++i) {
            scons << "x" << kIndices[i] << " - x" << lIndices[i]
                  << " - d" << selectors[i] << " = 0\n";
        }
    }
    scons.close();
}
