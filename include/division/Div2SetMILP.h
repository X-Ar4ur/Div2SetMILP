//
// Division Property MILP Manager Class
// Algorithm 3: 2-subset bit-based division property integral distinguisher search
//

#ifndef EASYBC_DIV2SETMILP_H
#define EASYBC_DIV2SETMILP_H

#include "ProcedureH.h"
#include <utility>
#include <cmath>
#include <set>
#include <sstream>
#include <ctime>
#include <algorithm>
#include "Transformer.h"
#include "Interpreter.h"
#include "DivMILPcons.h"

#include "gurobi_c++.h"

class Div2SetMILP {

private:
    std::string cipherName;
    std::vector<ProcedureHPtr> procedureHs;

    int rounds;
    std::string activebitsSpec;
    int blockSize = 0;

    int gurobiTimer = 3600 * 24;
    int gurobiThreads = 8;

    std::string pathPrefix;
    std::string modelPath;
    std::string resultsPath;

    std::map<std::string, std::vector<int>> Box;
    std::map<std::string, std::vector<std::vector<int>>> sboxDivIneqs;
    std::map<std::string, int> sboxInputSize;
    std::map<std::string, int> sboxOutputSize;

    int xCounter = 1;
    int dCounter = 1;

    std::map<std::string, int> tanNameMxIndex;
    std::map<std::string, int> rtnMxIndex;
    std::vector<int> rtnIdxSave;

    std::map<std::string, int> sboxNameMxIndex;
    std::vector<int> sboxRtnIdxSave;

    std::vector<int> outputBitIndices;

    std::vector<std::string> constantTan;
    int rndParamR;
    std::map<std::string, int> consTanNameMxVal;

public:
    Div2SetMILP(std::vector<ProcedureHPtr> procedureHs, int rounds,
                const std::string& activebitsSpec, const std::string& cipherName);

    // Parse activebitsSpec ("60", "R31", "L1R32") into a list of MILP variable
    // indices (x1..x{blockSize}) that should be set to 1. All other input
    // variables are set to 0. Returns {} and prints an error if spec is invalid.
    std::vector<int> resolveActiveBitVars() const;

    void setGurobiTimer(int timer) { this->gurobiTimer = timer; }
    void setGurobiThreads(int threads) { this->gurobiThreads = threads; }

    void MGR();

    void preprocess();
    void buildModel();
    void programGenModel();
    void roundFunctionGenModel(const ProcedureHPtr& procedureH);
    void sboxFunctionGenModel(const ProcedureHPtr& procedureH,
                              std::vector<ThreeAddressNodePtr> input,
                              std::vector<ThreeAddressNodePtr> output);

    void XORGenModel(const ThreeAddressNodePtr& left, const ThreeAddressNodePtr& right,
                     const ThreeAddressNodePtr& result, bool ifSboxFuncCall);
    void ANDGenModel(const ThreeAddressNodePtr& left, const ThreeAddressNodePtr& right,
                     const ThreeAddressNodePtr& result, bool ifSboxFuncCall);
    void SboxGenModel(const ThreeAddressNodePtr& sbox, const ThreeAddressNodePtr& input,
                      const ThreeAddressNodePtr& output);
    void PboxGenModel(const ThreeAddressNodePtr& pbox, const ThreeAddressNodePtr& input,
                      const ThreeAddressNodePtr& output);

    void iterativeSolver();

    // Helper methods (reused from DiffSBMILP pattern)
    bool isConstant(ThreeAddressNodePtr threeAddressNodePtr) {
        ThreeAddressNodePtr left = threeAddressNodePtr->getLhs();
        ThreeAddressNodePtr right = threeAddressNodePtr->getRhs();

        if (left and std::find(this->constantTan.begin(), this->constantTan.end(), left->getNodeName()) != this->constantTan.end()) {
            constantTan.push_back(threeAddressNodePtr->getNodeName());
            return true;
        }
        if (right and std::find(this->constantTan.begin(), this->constantTan.end(), right->getNodeName()) != this->constantTan.end()) {
            constantTan.push_back(threeAddressNodePtr->getNodeName());
            return true;
        }

        if (threeAddressNodePtr->getNodeName().find("_symbol_", 0) != std::string::npos) {
            this->constantTan.push_back(threeAddressNodePtr->getNodeName());
            return true;
        }

        int counter = 0;
        for (auto c : threeAddressNodePtr->getNodeName()) {
            if (48 <= c and c <= 57) {
                counter++;
            }
        }
        if (counter == (int)threeAddressNodePtr->getNodeName().size()) {
            this->constantTan.push_back(threeAddressNodePtr->getNodeName());
            return true;
        }

        if (left) {
            if ((threeAddressNodePtr->getOp() == ASTNode::SYMBOLINDEX and left->getNodeType() == NodeType::PARAMETER)) {
                this->constantTan.push_back(threeAddressNodePtr->getNodeName());
                return true;
            }
        }

        return false;
    }

    std::vector<int> extIdxFromTOUINTorBOXINDEX(const ThreeAddressNodePtr& input) {
        std::vector<int> extIdx;
        if (input->getOp() == ASTNode::TOUINT) {
            ThreeAddressNodePtr left = input->getLhs();
            while (left->getOp() == ASTNode::BOXINDEX) {
                if (tanNameMxIndex.count(left->getLhs()->getNodeName()) != 0) {
                    extIdx.push_back(tanNameMxIndex.find(left->getLhs()->getNodeName())->second);
                    left = left->getRhs();
                }
                else if (!rtnIdxSave.empty()) {
                    tanNameMxIndex[left->getLhs()->getNodeName()] = rtnIdxSave.front();
                    extIdx.push_back(rtnIdxSave.front());
                    rtnIdxSave.erase(rtnIdxSave.cbegin());
                    left = left->getRhs();
                } else {
                    tanNameMxIndex[left->getLhs()->getNodeName()] = xCounter;
                    extIdx.push_back(xCounter);
                    xCounter++;
                    left = left->getRhs();
                }
            }
            if (!rtnIdxSave.empty()) {
                tanNameMxIndex[left->getLhs()->getNodeName()] = rtnIdxSave.front();
                extIdx.push_back(rtnIdxSave.front());
                rtnIdxSave.erase(rtnIdxSave.cbegin());
            } else if (tanNameMxIndex.count(left->getNodeName()) != 0) {
                extIdx.push_back(tanNameMxIndex.find(left->getNodeName())->second);
            } else {
                tanNameMxIndex[left->getLhs()->getNodeName()] = xCounter;
                extIdx.push_back(xCounter);
                xCounter++;
            }
        } else if (input->getOp() == ASTNode::BOXINDEX) {
            ThreeAddressNodePtr left = input;
            while (left->getOp() == ASTNode::BOXINDEX) {
                if (tanNameMxIndex.count(left->getLhs()->getNodeName()) != 0) {
                    extIdx.push_back(tanNameMxIndex.find(left->getLhs()->getNodeName())->second);
                    left = left->getRhs();
                }
                else if (left->getLhs() != nullptr and left->getRhs() != nullptr) {
                    if (left->getLhs()->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getLhs()->getNodeType() != UINT1) {
                        if (tanNameMxIndex.count(left->getLhs()->getLhs()->getNodeName() + "_$B$_" +
                                                 left->getLhs()->getRhs()->getNodeName()) != 0) {
                            extIdx.push_back(tanNameMxIndex.find(left->getLhs()->getLhs()->getNodeName() + "_$B$_" +
                                                                 left->getLhs()->getRhs()->getNodeName())->second);
                            tanNameMxIndex[left->getNodeName()] = tanNameMxIndex.find(left->getLhs()->getLhs()->getNodeName() + "_$B$_" +
                                                                                      left->getLhs()->getRhs()->getNodeName())->second;
                            left = left->getRhs();
                        } else
                            assert(false);
                    }
                }
                else if (!rtnIdxSave.empty()) {
                    tanNameMxIndex[left->getLhs()->getNodeName()] = rtnIdxSave.front();
                    extIdx.push_back(rtnIdxSave.front());
                    rtnIdxSave.erase(rtnIdxSave.cbegin());
                    left = left->getRhs();
                } else {
                    tanNameMxIndex[left->getLhs()->getNodeName()] = xCounter;
                    extIdx.push_back(xCounter);
                    xCounter++;
                    left = left->getRhs();
                }
            }
            if (!rtnIdxSave.empty()) {
                tanNameMxIndex[left->getNodeName()] = rtnIdxSave.front();
                extIdx.push_back(rtnIdxSave.front());
                rtnIdxSave.erase(rtnIdxSave.cbegin());
            } else if (tanNameMxIndex.count(left->getNodeName()) != 0) {
                extIdx.push_back(tanNameMxIndex.find(left->getNodeName())->second);
            } else {
                tanNameMxIndex[left->getLhs()->getNodeName()] = xCounter;
                extIdx.push_back(xCounter);
                xCounter++;
            }
        } else assert(false);
        return extIdx;
    }

    static int transNodeTypeSize(NodeType nodeType) {
        if (nodeType == UINT) return 0;
        else if (nodeType == UINT1) return 1;
        else if (nodeType == UINT4) return 4;
        else if (nodeType == UINT6) return 6;
        else if (nodeType == UINT8) return 8;
        else if (nodeType == UINT12) return 12;
        else if (nodeType == UINT16) return 16;
        else if (nodeType == UINT32) return 32;
        else if (nodeType == UINT64) return 64;
        else if (nodeType == UINT128) return 128;
        else if (nodeType == UINT256) return 256;
        return 0;
    }

    void sboxSizeGet(std::string name, std::vector<int> sbox) {
        sboxInputSize[name] = int(log2(sbox.size()));
        std::map<int, int> sboxMap;
        for (int & i : sbox) {
            sboxMap[i]++;
        }
        sboxOutputSize[name] = int(log2(sboxMap.size()));
    }
};

#endif //EASYBC_DIV2SETMILP_H
