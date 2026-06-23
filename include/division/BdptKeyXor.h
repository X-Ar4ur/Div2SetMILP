#ifndef EASYBC_BDPTKEYXOR_H
#define EASYBC_BDPTKEYXOR_H

#include <map>
#include <string>
#include <vector>

#include "ProcedureH.h"

struct BdptKeyXorLayer {
    int id = -1;
    int round = -1;
    std::string roundFunction;
    std::vector<std::string> xorNodeNames;
};

struct BdptKeyXorMatch {
    bool isKeyXor = false;
    bool leftIsKey = false;
    bool rightIsKey = false;
};

BdptKeyXorMatch matchBdptKeyXorNode(const ThreeAddressNodePtr& node,
                                    const ProcedureHPtr& roundProcedure);

std::vector<BdptKeyXorLayer>
discoverBdptKeyXorLayers(const std::vector<ProcedureHPtr>& procedures,
                         int rounds);

#endif // EASYBC_BDPTKEYXOR_H
