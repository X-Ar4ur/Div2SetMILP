#include "division/BdptMILPcons.h"

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
