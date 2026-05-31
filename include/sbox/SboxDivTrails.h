#ifndef EASYBC_SBOXDIVTRAILS_H
#define EASYBC_SBOXDIVTRAILS_H

#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <cassert>
#include <iostream>

class SboxDivTrails {

private:
    std::string name_;
    std::vector<int> sbox_;
    int sboxBitSize_;

    std::vector<std::vector<int>> divTrails_;   // K-trails (CBDP, Algorithm 1 for K)
    std::vector<std::vector<int>> lDivTrails_;  // L-trails (BDPT, Algorithm 1 for L)

    // 内部方法
    static int bitProduct(int u, int x);
    std::vector<int> getTruthTable(int u);
    void processTable(std::vector<int>& table);
    std::vector<std::vector<int>> createANF();

public:
    SboxDivTrails(std::string name, std::vector<int> sbox);

    void createDivisionTrails();
    void printDivisionTrails(const std::string& filename);

    // 3-subset BDPT: L-set division trails of the S-box (Algorithm 1, L part).
    // Faithful to the reference implementation (algorithm1/sbox.py): for each
    // (l, u) with l, u in [1, 2^n), (l, u) is a valid L-trail iff pi_u(y)
    // contains the monomial pi_l(x) AND its monomial set has empty all-ones
    // (S_cap non-empty). The (0..0) and (1..1) trails are appended explicitly.
    // No up-set / dominance reduction (L is exact-equality semantics).
    void createLDivisionTrails();
    void printLDivisionTrails(const std::string& filename);

    // 供后续 MILP 建模模块调用的接口
    std::vector<std::vector<int>> getDivisionTrails() const { return divTrails_; }
    std::vector<std::vector<int>> getLDivisionTrails() const { return lDivTrails_; }
    int getSboxBitSize() const { return sboxBitSize_; }
    std::string getName() const { return name_; }
};

#endif //EASYBC_SBOXDIVTRAILS_H
