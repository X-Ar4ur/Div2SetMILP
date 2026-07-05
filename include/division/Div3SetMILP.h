//
// 三子集比特级可分性（BDPT）MILP 后端，对应参考论文 Algorithm 3/4。
// 该后端由 `-div3` 入口调用，与二子集 Div2SetMILP 共存且不修改二子集路径。
//
// 建模结构：
//   * M_L：全 r 轮 L 链，使用 O_l 不等式；
//   * M_t：cross 前使用 O_l，选中 Key-XOR 后切换到 O_k；
//   * Key-XOR cross：只加入论文第 9-10 行的不全 1 与支配约束；
//   * 求解：模型集 P 用 min-pin 枚举单位 K_q，M_L 枚举 L_q 解数奇偶。
//

#ifndef EASYBC_DIV3SETMILP_H
#define EASYBC_DIV3SETMILP_H

#include <utility>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sstream>
#include <ctime>
#include <algorithm>
#include "ProcedureH.h"
#include "Transformer.h"
#include "Interpreter.h"
#include "DivMILPcons.h"
#include "BdptMILPcons.h"
#include "BdptSolveResult.h"
#include "BdptKeyXor.h"
#include "BdptSemanticScheduler.h"

#include "gurobi_c++.h"

class Div3SetMILP {

public:
    // 当前 S 盒使用的可分迹不等式集合。
    enum ChainMode { CHAIN_K = 0, CHAIN_L = 1 };

private:
    std::string cipherName;
    std::vector<ProcedureHPtr> procedureHs;

    int rounds;
    std::string activebitsSpec;
    int blockSize = 0;

    int gurobiTimer = 3600 * 24;
    int gurobiThreads = 8;

    std::string pathPrefix;
    std::string runDir;
    std::string modelPath;
    std::string resultsPath;

    std::map<std::string, std::vector<int>> Box;
    // O_k 不等式：K 链 S 盒可分迹。
    std::map<std::string, std::vector<std::vector<int>>> sboxDivIneqs;
    // O_l 不等式：L 链 S 盒可分迹。
    std::map<std::string, std::vector<std::vector<int>>> sboxLDivIneqs;
    std::map<std::string, int> sboxInputSize;
    std::map<std::string, int> sboxOutputSize;

    // 当前链模式决定 S 盒处使用 O_k 还是 O_l。
    ChainMode chainMode = CHAIN_K;

    // Key-XOR cross 状态，对应论文 Algorithm 3 的 t-th Key-XOR。
    std::vector<BdptKeyXorLayer> keyXorLayers;
    int selectedCrossLayer = -1;
    int currentKeyXorLayer = -1;
    int currentRound = 0;
    std::vector<int> crossLBits;
    std::vector<int> crossKBits;
    // Key-XOR cross 的 L 端变量虽然不是最终输出，但仍被 cross 约束引用，不能作为死 COPY 尾固定为 0。
    std::set<int> protectedTailIndices;

    int xCounter = 1;

    std::map<std::string, int> tanNameMxIndex;
    std::map<std::string, int> rtnMxIndex;
    std::vector<int> rtnIdxSave;

    std::map<std::string, int> sboxNameMxIndex;
    std::vector<int> sboxRtnIdxSave;

    std::vector<int> outputBitIndices;

    std::vector<std::string> constantTan;
    int rndParamR;
    std::map<std::string, int> consTanNameMxVal;

    // 惰性 COPY-on-read：读取 live 变量时拆出消费副本和新的 live 代表。
    std::map<int, int> liveChain;
    int consumeCopy(int rawIdx);

    // 清理单次建模状态；已加载的不等式和 S 盒尺寸保留。
    void resetState();

    // 统一 LP 收尾：添加目标函数、初始 L 约束和 Binary 段。
    void writeLpFile(const std::string& modeTag);

    // 构造 M_L 与 M_t。M_t 在选中的 Key-XOR 后从 L 链切换到 K 链。
    void buildMLModel(const std::string& modelFile);
    void buildMtModel(int modelNumber, int keyXorLayerId, const std::string& modelFile);

    // Algorithm 4 的 K_q 判定：用 min-pin 枚举单位输出，不做 per-bit 求解。
    BdptSolveResult solveMtReachableCoords(const std::string& lpFile,
                                           const std::vector<int>& outIdx,
                                           const std::set<int>& skip);
    BdptSolveResult solveMtReachableCoordsMinPin(const std::string& lpFile,
                                                 const std::vector<int>& outIdx,
                                                 const std::set<int>& skip);

    // Algorithm 4: 固定 M_L 的 L_q=e_q，枚举解数奇偶。
    int classifyMLParity(const std::string& lpFile,
                         const std::vector<int>& outIdx,
                         int coord,
                         int& solutionCount);

    // 按论文 Algorithm 4 组织模型集 P 与 M_L 的整体判定。
    void searchDistinguisher();

public:
    Div3SetMILP(std::vector<ProcedureHPtr> procedureHs, int rounds,
                const std::string& activebitsSpec, const std::string& cipherName);

    std::vector<int> resolveActiveBitVars() const;

    void setGurobiTimer(int timer) { this->gurobiTimer = timer; }
    void setGurobiThreads(int threads) { this->gurobiThreads = threads; }

    void MGR();

    void preprocess();
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

    // 下面是从 Div2SetMILP 复用的 TAC 辅助方法。
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

#endif //EASYBC_DIV3SETMILP_H
