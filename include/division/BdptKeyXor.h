#ifndef EASYBC_BDPTKEYXOR_H
#define EASYBC_BDPTKEYXOR_H

#include <map>
#include <memory>
#include <string>
#include <vector>

class ProcedureH;
class ThreeAddressNode;
typedef std::shared_ptr<ProcedureH> ProcedureHPtr;
typedef std::shared_ptr<ThreeAddressNode> ThreeAddressNodePtr;

struct BdptKeyXorLayer {
    int id = -1;
    int round = -1;
    std::string roundFunction;
    std::vector<std::string> xorNodeNames;
    int firstNodeIndex = -1;
    bool beforeRoundCore = false;
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
