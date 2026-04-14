//
// Created by Septi on 6/28/2023.
//

#ifndef EASYBC_LINEARMILPCONS_H
#define EASYBC_LINEARMILPCONS_H


#include <fstream>
#include "istream"
#include "iostream"
#include "vector"
#include "string"

namespace LinearMILPcons {

    // word-wise MILP constraints

    // three-fork constraints
    void wXorC1(std::string path, int inputIdx1, int inputIdx2, int outputIdx, int &dCounter);

    void wXorC2(std::string path, int inputIdx1, int inputIdx2, int outputIdx);

    void wAndOrC(std::string path, int inputIdx1, int inputIdx2, int outputIdx);

    void wLinearC1(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx, int &dCounter, int branchMin, int branchMax);

    void wLinearC2(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx, int &dCounter, int branchMin, int branchMax);



    // bit-wise MILP constraints
    void bThreeForkC(std::string path, int inputIdx, int outputIdx1, int outputIdx2, int &dCounter);

    void bXorC(std::string path, int inputIdx1, int inputIdx2, int outputIdx);

    void bMatrixEntryC12(std::string path, std::vector<std::vector<int>> inputIdx, std::vector<int> outputIdx,
                         std::vector<int> overflows, std::vector<int> module,
                         int &xCounter, int &yCounter, int &fCounter, int &dCounter, int chooser);

    void bNXorC3(std::string path, std::vector<int> inputIdx, int outputIdx, int &yCounter);

    void bMatrixEntryC3(std::string path, std::vector<std::vector<int>> inputIdx, std::vector<int> outputIdx,
                        std::vector<int> overflows, std::vector<int> module,
                        int &xCounter, int &yCounter, int &fCounter);

    // full sbox constraints
    void bSboxC1(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                 std::vector<std::vector<int>> ineqs, int branchMin, int &dCounter, int &ACounter, bool ifInjective);

    void bSboxC2(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                 std::vector<std::vector<int>> ineqs, int &ACounter, bool ifInjective);

    void bSboxC3(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                 std::vector<std::vector<int>> ineqs, int &ACounter);

    void bAddC(std::string path, std::vector<int> inputIdx1, std::vector<int> inputIdx2, std::vector<int> outputIdx,
               std::vector<std::vector<int>> ineqs, int &dCounter, int &PCounter);

    void bAndOrC(std::string path, int inputIdx1, int inputIdx2, int outputIdx);




    // bit-wise MILP constraints for computing probability of differential characteristics


    void dSboxC1(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                 std::vector<std::vector<int>> ineqs, int branchMin, int &dCounter,
                 int &PCounter, bool ifInjective, int extLen);

    void dSboxC2(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                 std::vector<std::vector<int>> ineqs, int &PCounter, bool ifInjective, int extLen);

    void dSboxC3(std::string path, std::vector<int> inputIdx, std::vector<int> outputIdx,
                 std::vector<std::vector<int>> ineqs, int &PCounter, int extLen);

    void dAddC(std::string path, std::vector<int> inputIdx1, std::vector<int> inputIdx2, std::vector<int> outputIdx,
               std::vector<std::vector<int>> ineqs, int &dCounter, int &PCounter);

    void dAndOrC(std::string path, int inputIdx1, int inputIdx2, int outputIdx, int &PCounter);

}

#endif //EASYBC_LINEARMILPCONS_H
