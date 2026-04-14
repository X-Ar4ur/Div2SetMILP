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

    std::vector<std::vector<int>> divTrails_;

    // 内部方法
    static int bitProduct(int u, int x);
    std::vector<int> getTruthTable(int u);
    void processTable(std::vector<int>& table);
    std::vector<std::vector<int>> createANF();

public:
    SboxDivTrails(std::string name, std::vector<int> sbox);

    void createDivisionTrails();
    void printDivisionTrails(const std::string& filename);

    // 供后续 MILP 建模模块调用的接口
    std::vector<std::vector<int>> getDivisionTrails() const { return divTrails_; }
    int getSboxBitSize() const { return sboxBitSize_; }
    std::string getName() const { return name_; }
};

#endif //EASYBC_SBOXDIVTRAILS_H
