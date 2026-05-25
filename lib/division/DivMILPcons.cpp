#include "division/DivMILPcons.h"

void DivMILPcons::divXorC(std::string path, int inputIdx1, int inputIdx2, int outputIdx) {
    std::ofstream scons(path, std::ios::app);
    if (!scons) {
        std::cout << "Wrong file path in divXorC!" << std::endl;
    } else {
        scons << "x" << outputIdx << " - x" << inputIdx1 << " - x" << inputIdx2 << " = 0\n";
    }
    scons.close();
}

void DivMILPcons::divCopyC(std::string path, int inputIdx, std::vector<int> copyIdx) {
    std::ofstream scons(path, std::ios::app);
    if (!scons) {
        std::cout << "Wrong file path in divCopyC!" << std::endl;
    } else {
        scons << "x" << inputIdx;
        for (auto i : copyIdx) {
            scons << " - x" << i;
        }
        scons << " = 0\n";
    }
    scons.close();
}

void DivMILPcons::divAndC(std::string path, int inputIdx1, int inputIdx2, int outputIdx) {
    std::ofstream scons(path, std::ios::app);
    if (!scons) {
        std::cout << "Wrong file path in divAndC!" << std::endl;
    } else {
        scons << "x" << outputIdx << " - x" << inputIdx1 << " >= 0\n";
        scons << "x" << outputIdx << " - x" << inputIdx2 << " >= 0\n";
        scons << "x" << outputIdx << " - x" << inputIdx1 << " - x" << inputIdx2 << " <= 0\n";
    }
    scons.close();
}

void DivMILPcons::divSboxC(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                            std::vector<std::vector<int>> ineqs) {
    std::ofstream scons(path, std::ios::app);
    if (!scons) {
        std::cout << "Wrong file path in divSboxC!" << std::endl;
    } else {
        for (const auto& ineq : ineqs) {
            int idx = 0;
            // Input variable coefficients
            for (auto i : inputIdx) {
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << i;
                else
                    scons << " - " << abs(ineq[idx]) << " x" << i;
                idx++;
            }
            // Output variable coefficients
            for (auto i : outputIdx) {
                if (ineq[idx] >= 0)
                    scons << " + " << abs(ineq[idx]) << " x" << i;
                else
                    scons << " - " << abs(ineq[idx]) << " x" << i;
                idx++;
            }
            // Constant term: inequality is a0*x0 + ... + b >= 0, i.e., LHS >= -b
            if (ineq[idx] > 0)
                scons << " >= -" << abs(ineq[idx]) << "\n";
            else
                scons << " >= " << abs(ineq[idx]) << "\n";
        }
    }
    scons.close();
}

void DivMILPcons::divXorMultiC(std::string path, const std::vector<int>& inputIdx, int outputIdx) {
    if (inputIdx.empty()) {
        std::cout << "ERROR: divXorMultiC called with empty inputIdx (outputIdx=" << outputIdx << ")" << std::endl;
        return;
    }
    std::ofstream scons(path, std::ios::app);
    if (!scons) {
        std::cout << "Wrong file path in divXorMultiC!" << std::endl;
    } else {
        scons << "x" << outputIdx;
        for (auto i : inputIdx) {
            scons << " - x" << i;
        }
        scons << " = 0\n";
    }
    scons.close();
}
