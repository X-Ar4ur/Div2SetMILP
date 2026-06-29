#include "division/BdptKeyXor.h"

#include "ProcedureH.h"

#include <cctype>
#include <map>
#include <set>

namespace {

std::string baseProcedureName(const std::string& name) {
    const size_t at = name.find('@');
    return at == std::string::npos ? name : name.substr(0, at);
}

std::string stripTrailingDigits(const std::string& name) {
    size_t end = name.size();
    while (end > 0 && std::isdigit(static_cast<unsigned char>(name[end - 1]))) {
        --end;
    }
    return name.substr(0, end);
}

std::set<std::string> keyParameterNamesOf(const ProcedureHPtr& procedure) {
    std::set<std::string> names;
    const auto& params = procedure->getParameters();
    if (params.size() < 2) return names;
    for (const auto& keyNode : params[1]) {
        names.insert(keyNode->getNodeName());
    }
    return names;
}

std::set<std::string> keyArrayBaseNamesOf(const std::set<std::string>& keyNames) {
    std::set<std::string> bases;
    for (const std::string& name : keyNames) {
        const std::string base = stripTrailingDigits(name);
        if (!base.empty() && base != name) {
            bases.insert(base);
        }
    }
    return bases;
}

bool hasGeneratedKeyBitName(const std::string& name,
                            const std::set<std::string>& keyBases) {
    for (const std::string& base : keyBases) {
        if (name.rfind(base, 0) != 0) continue;
        const std::string suffix = name.substr(base.size());
        if (suffix.empty()) continue;
        bool allDigits = true;
        for (char c : suffix) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                allDigits = false;
                break;
            }
        }
        if (allDigits) return true;
        if (suffix.rfind("_$B$_", 0) == 0) return true;
    }
    return false;
}

bool isKeyOperand(const ThreeAddressNodePtr& node,
                  const std::set<std::string>& keyNames,
                  const std::set<std::string>& keyBases) {
    if (!node) return false;
    const std::string& name = node->getNodeName();
    if (keyNames.count(name)) return true;
    if (hasGeneratedKeyBitName(name, keyBases)) return true;

    if ((node->getOp() == ASTNode::INDEX || node->getOp() == ASTNode::SYMBOLINDEX) &&
        node->getLhs()) {
        const std::string& arrayName = node->getLhs()->getNodeName();
        return keyBases.count(arrayName) != 0 || keyNames.count(arrayName) != 0;
    }
    return false;
}

bool isRoundCoreNode(const ThreeAddressNodePtr& node,
                     const ProcedureHPtr& procedure) {
    if (!node) return false;
    if (matchBdptKeyXorNode(node, procedure).isKeyXor) return false;

    const auto op = node->getOp();
    if (op == ASTNode::XOR || op == ASTNode::AND || op == ASTNode::OR ||
        op == ASTNode::BOXOP || op == ASTNode::PUSH ||
        op == ASTNode::ADD || op == ASTNode::MINUS || op == ASTNode::NOT) {
        return true;
    }
    if (op == ASTNode::SYMBOLINDEX && node->getLhs() &&
        node->getLhs()->getOp() == ASTNode::FFTIMES) {
        return true;
    }
    return false;
}

std::vector<BdptKeyXorLayer> discoverRoundLayers(
        const ProcedureHPtr& procedure,
        int round,
        int& nextLayerId) {
    std::vector<BdptKeyXorLayer> layers;
    bool inKeyXorLayer = false;
    bool seenRoundCore = false;

    for (int i = 0; i < (int)procedure->getBlock().size(); ++i) {
        const auto& node = procedure->getBlock().at(i);
        const BdptKeyXorMatch match = matchBdptKeyXorNode(node, procedure);
        if (match.isKeyXor) {
            if (!inKeyXorLayer) {
                BdptKeyXorLayer layer;
                layer.id = nextLayerId++;
                layer.round = round;
                layer.roundFunction = procedure->getName();
                layer.firstNodeIndex = i;
                layer.beforeRoundCore = !seenRoundCore;
                layers.push_back(layer);
                inKeyXorLayer = true;
            }
            layers.back().xorNodeNames.push_back(node->getNodeName());
        } else {
            inKeyXorLayer = false;
            if (isRoundCoreNode(node, procedure)) seenRoundCore = true;
        }
    }

    return layers;
}

} // namespace

BdptKeyXorMatch matchBdptKeyXorNode(const ThreeAddressNodePtr& node,
                                    const ProcedureHPtr& roundProcedure) {
    BdptKeyXorMatch match;
    if (!node || node->getOp() != ASTNode::XOR ||
        !node->getLhs() || !node->getRhs() || !roundProcedure) {
        return match;
    }

    const std::set<std::string> keyNames = keyParameterNamesOf(roundProcedure);
    const std::set<std::string> keyBases = keyArrayBaseNamesOf(keyNames);
    match.leftIsKey = isKeyOperand(node->getLhs(), keyNames, keyBases);
    match.rightIsKey = isKeyOperand(node->getRhs(), keyNames, keyBases);
    match.isKeyXor = match.leftIsKey != match.rightIsKey;
    return match;
}

std::vector<BdptKeyXorLayer>
discoverBdptKeyXorLayers(const std::vector<ProcedureHPtr>& procedures,
                         int rounds) {
    std::map<std::string, ProcedureHPtr> procByName;
    ProcedureHPtr mainProc;
    for (const auto& proc : procedures) {
        procByName[proc->getName()] = proc;
        if (proc->getName() == "main") mainProc = proc;
    }

    std::vector<BdptKeyXorLayer> layers;
    if (!mainProc || rounds <= 0) return layers;

    bool roundReady = false;
    int processedRounds = 0;
    int nextLayerId = 0;
    for (const auto& node : mainProc->getBlock()) {
        if (node && node->getLhs() && node->getLhs()->getNodeType() == UINT) {
            roundReady = true;
            continue;
        }
        if (!roundReady || processedRounds >= rounds ||
            !node || node->getOp() != ASTNode::CALL || !node->getLhs()) {
            continue;
        }

        const std::string roundFunctionName =
            baseProcedureName(node->getLhs()->getNodeName());
        auto procIt = procByName.find(roundFunctionName);
        if (procIt != procByName.end()) {
            const int round = processedRounds + 1;
            std::vector<BdptKeyXorLayer> roundLayers =
                discoverRoundLayers(procIt->second, round, nextLayerId);
            layers.insert(layers.end(), roundLayers.begin(), roundLayers.end());
        }

        processedRounds++;
        roundReady = false;
    }

    return layers;
}
