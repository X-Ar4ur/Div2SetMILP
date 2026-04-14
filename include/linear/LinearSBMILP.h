//
// Created by Septi on 6/28/2023.
//

#ifndef EASYBC_LINEARSBMILP_H
#define EASYBC_LINEARSBMILP_H

#include "ProcedureH.h"
#include <utility>
#include <cmath>
#include <util/utilities.h>
#include "Transformer.h"
#include "Interpreter.h"
#include "Reduction.h"
#include "SyntaxGuided.h"
#include "BranchNum.h"
#include "LinearMILPcons.h"
#include "SboxModel.h"

/*
 * 暂时还没有解决的问题：
 *      1. linear下各种operation的多种建模方式，怎么建模，怎么选择？
 *      2. matrix-vector product如何建模？
 *
 *
 * */

class LinearSBMILP {

private:
    std::string cipherName;
    vector<ProcedureHPtr> procedureHs;


    // **************** setup ****************
    /*
     * "cryptanalysis" -> calculate minimal number of active S-boxes, or maximal differential characteristic probability
     * "evaluation" -> evaluate the security of block ciphers by number of active S-boxes, or differential characteristic probability
     * */
    std::string target;
    /*
     * "AS" -> "cryptanalysis" or "evaluation" by the number of active S-boxes,
     * "DC" -> "cryptanalysis" or "evaluation" by the differential characteristic probability
     * */
    std::string mode;
    int redMode; // the selection of reduction methods for modeling S-boxes
    int gurobiTimer = 3600 * 12; // the timer of gurobi solver
    int gurobiThreads = 8; // the number of threads of gurobi solver
    int startRound = 1; // the start round of "cryptanalysis" or "evaluation"
    int endRound = 5; // the end round of "cryptanalysis" or "evaluation"
    int currentRound; // the current round of "cryptanalysis" or "evaluation"
    /*
     * the selections of constraints for XOR, S-boxes and matrix-vector product
     * */
    int xorConSel = 2; // 1-3
    int sboxConSel = 1; // 1-3
    int matrixConSel = 2; // 1-3
    /*
     * speed up techniques
     * speedup1 -> the sum of input difference is larger than 1
     * speedup2 -> all sum of difference that fixed the searched results is no less than the current one which will be searched
     * */
    bool speedup1 = false;
    bool speedup2 = false;

    bool ILPFlag = false; // ILP or MILP ?


    // **************** results ****************
    /*
     * "cryptanalysis" -> minimal number of active S-boxes, or maximal differential characteristic probability of each round
     * "evaluation" -> number of active S-boxes, or differential characteristic probability of each round for evaluating the security
     * */
    vector<int> finalResults;
    // "cryptanalysis" or "evaluation" time of each round
    vector<double> finalClockTime;
    vector<double> finalTimeTime;


    // **************** analysis structure ****************
    std::string pathPrefix = std::string(DPATH) + "differential/singleKey/BitWiseMILP/";
    std::string pathSuffix;
    std::string modelPath;
    std::string resultsPath;
    std::string diffCharacteristicPath; // save differential characteristics
    std::string finalResultsPath;

    std::map<std::string, std::vector<int>> Box;  // sbox, pbox
    std::map<std::string, std::vector<std::vector<int>>> sboxIneqs;
    std::vector<std::vector<int>> ARXineqs;
    // Here we consider the extension bits to be the same in the case of multiple S-boxes
    std::vector<int> sboxExtWeighted; // the weight of each extended bit of "DC" mode used to decide the objective function
    std::map<std::string, bool> sboxIfInjective;
    std::map<std::string, int> sboxInputSize; // used to save the input and output size of S-box
    std::map<std::string, int> sboxOutputSize;
    std::map<std::string, int> branchNumMin; // the branch numbers
    std::map<std::string, int> branchNumMax;

    // Ascon k = 64, p = 2, r = 12
    // Elephant k = 160, p = 2, r = 80
    // present k = 80, p = 2, r = 31
    // prince k = 128, p = 2, r = 11
    // Gift  k = 128, p = 1.414, r = 28
    // Gift-cofb k = 128, p = 1.414, r = 40
    // rectangle k = 80, p = 2, r = 25
    // skinny k = 128, p = 2, r = 36
    // Sparkle k = 64, r = 13
    // simon32 k = 64, r = 32
    // simon48 k = 72, r = 36
    int keySize = 160;
    int totalRoundNum = 80;
    float sboxMDP;
    int targetNdiffOrPr; // active sbox 数目的目标值或者概率和的目标值,

    int blockSize;

    bool securityFlag = false;
    int securityRoundNumBound;

    int xCounter = 1;
    int dCounter = 1;
    int ACounter = 1;
    int PCounter = 1;
    int fCounter = 1; // 溢出位结果标识
    int yCounter = 1; // new XOR model2 label

    // speedup constraints auxiliary variables
    int lastRoundACounter = 1, lastRoundPCounter = 1;
    std::vector<std::vector<int>> allRoundACounters, allRoundPCounters;

    map<std::string, int> tanNameMxIndex; // the map of ThreeAddressNodes to x indexes
    map<std::string, int> rtnMxIndex; // the map of returned ThreeAddressNodes to x indexes
    std::vector<int> rtnIdxSave; // the x indexes of returned ThreeAddressNodes

    // 当在sboxFunction内调用各种操作的建模函数时，用于存储对应于sboxFunction的各个对应的xCounter
    map<std::string, int> sboxNameMxIndex;
    // 当在sboxFunction内调用各种操作的建模函数时，用于存储对应于sboxFunction的return值的xCounter
    std::vector<int> sboxRtnIdxSave;

    std::vector<std::vector<int>> differentialCharacteristic;

    // 用于存放所有是constant的threeAddressNode
    std::vector<std::string> constantTan;

    // 下面的参数目前只用在round function中
    /*
     * liner的分析中，比较特殊的是如何判断three-fork的存在，
     * 现在设想的做法是，在round function的对应模型构造之前，我们先遍历该函数，
     * 通过判断每个threeAddressNode参与的operation数量来判断其是否是three fork，
     * 理论上来说，若一个threeAddressNode实例作为输入值之一参与了两次不同的operation，
     * 则该threeAddressNode实例有一个three-fork，
     * 且该three-fork的输入是该threeAddressNode实例，输出是两个operation分别对应于该三地址的输入，
     * 通过以上分析，我考虑使用一个map<string, int>来维护所有threeAddressNode是否有three-fork，
     * 若一个map条目中对应的int值为2，则该条目中string对应的threeAddressNode存在three-fork
     * */
    // 每个function维护的threeAddressNode对应是否有three-fork
    std::map<std::string, int> tanThreeForkMap;
    /*
     * 每个确定是three-fork输入的threeAddressNode实例，都有对应两个输出，而在对应的约束构建以后，
     * 后续在该threeAddressNode实例后续两次构建约束时，要分别使用这两个输出对应的xCounter，
     * 因此需要用下面这个map来维护，用以判断对应的输出xCounter是否已经被使用。
     * 其次，我们用std::map<std::string, std::vector<string>>，具体的，第一个string是对应于
     * three-fork输入的threeAddressNode的nodeName，后续的vector存储该threeAddressNode对应
     * 的两个输出的xCounter，这里用string存储是因为后面需要判断对应的哪个xCounter已经被使用过了，
     * 初始std::vector<string>中的两个元素是两个输出的xCounter的to_string，若某个输出的xCounter
     * 已经被使用，则对应的to_string结果之前加上 "used"，用以后续判断
     *
     *
     * revised：
     * 实现的时候考虑了一下，只需要维护一个std::map<std::string, int>，因为在three-fork的约束
     * 构建时，直接把对应three-fork的输入的threeAddressNode对应的xCounter映射为其中一个输入xCounter，
     * 另一个存放在维护的map中，后续再对该threeAddressNode进行约束构建时，直接取存放在map中的xCounter
     * */
    //std::map<std::string, std::vector<std::string>> tanThreeForkNameMxIndex;
    std::map<std::string, int> tanThreeForkNameMxIndex;
    // 所有function维护的threeAddressNode对应是否有three-fork
    std::map<std::string, std::map<std::string, int>> funcTanThreeForkMap;


public:
    LinearSBMILP(std::vector<ProcedureHPtr> procedureHs, std::string target, std::string mode, int redMode);

    // parameters  setup
    void setGurobiTimer(int timer) {this->gurobiTimer = timer;}
    void setGurobiThreads(int threads) {this->gurobiThreads = threads;}
    void setStartRound(int round) {this->startRound = round;}
    void setEndRound(int round) {this->endRound = round;}
    void setXorConSel(int select) {this->xorConSel = select;}
    void setSboxConSel(int select) {this->sboxConSel = select;}
    void setMatrixConSel(int select) {this->matrixConSel = select;}
    void setSpeedUp1() {this->speedup1 = true;}
    void setSpeedUp2() {this->speedup2 = true;}
    void setILP() {this->ILPFlag = true;}
    void setKeySize(int size) {this->keySize = size;}
    void setTotalRoundNum(int num) {this->totalRoundNum = num;}


    void MGR();

    void cryptanalysis();

    void evaluation();

    void solver();

    void programGenModel();

    void roundFunctionThreeForkTraversal(const ProcedureHPtr& procedureH);

    void roundFunctionGenModel(const ProcedureHPtr& procedureH);

    void sboxFunctionGenModel(const ProcedureHPtr& procedureH, std::vector<ThreeAddressNodePtr> input, std::vector<ThreeAddressNodePtr> output);

    void ThreeForkGenModel(const ThreeAddressNodePtr& input, bool ifSboxFuncCall);

    void XORGenModel(const ThreeAddressNodePtr& left, const ThreeAddressNodePtr& right, const ThreeAddressNodePtr& result, bool ifSboxFuncCall);

    void ANDandORGenModel(const ThreeAddressNodePtr& left, const ThreeAddressNodePtr& right, const ThreeAddressNodePtr& result, bool ifSboxFuncCall);

    void SboxGenModel(const ThreeAddressNodePtr& sbox, const ThreeAddressNodePtr& input, const ThreeAddressNodePtr& output);

    void PboxGenModel(const ThreeAddressNodePtr& pbox, const ThreeAddressNodePtr& input, const ThreeAddressNodePtr& output);

    // matrix-vector product
    void MatrixVectorGenModel(const ThreeAddressNodePtr& pboxm, const ThreeAddressNodePtr& input, const std::vector<ThreeAddressNodePtr>& output);
    void multiXORGenModel12(std::vector<ThreeAddressNodePtr> input, ThreeAddressNodePtr result);
    void multiXORGenModel3(std::vector<ThreeAddressNodePtr> input, ThreeAddressNodePtr result);

    // for ARX add
    void ADDandMINUSGenModel(std::vector<ThreeAddressNodePtr>& input1, std::vector<ThreeAddressNodePtr>& input2,
                             const std::vector<ThreeAddressNodePtr>& output, int AddOrMinus, bool ifSboxFuncCall);


    void preprocess(std::map<std::string, std::vector<int>> pboxM, std::map<std::string, std::vector<int>> Ffm) {
        // process various box
        auto iterator = this->Box.begin();
        while (iterator != this->Box.end()) {
            if (iterator->first.substr(0, 4) == "sbox") {
                std::string sboxName = iterator->first;
                SboxM sboxM(sboxName, iterator->second, "linear", this->mode);
                this->sboxExtWeighted = sboxM.get_extWeighted();
                // 获取sboxMDP
                this->sboxMDP = sboxM.getSboxMDP();
                // when redMode = 8, the ineqs is read from extern files
                std::vector<std::vector<int>> ineq_set = Red::reduction(this->redMode, sboxM);
                this->sboxIneqs[sboxName] = ineq_set;
                // check if the sbox is injective or not
                this->sboxIfInjective[sboxName] = sboxInjectiveCheck(iterator->second);
                // get the input and output size of the S-box
                sboxSizeGet(sboxName, iterator->second);
            }
            // Calculate branch number of each box
            this->branchNumMin[iterator->first] = Red::branch_num_of_sbox(iterator->second);
            iterator++;
        }

        SboxM sboxM(this->cipherName, this->mode);
        // for ineqs of arx-structure, we always use greedy algorithm to reduce the generated inequalities
        this->ARXineqs = Red::reduction(1, sboxM);

        // process matrix
        auto iterM = pboxM.begin();
        while (iterM != pboxM.end()) {
            BranchN branchN(iterM->second, Ffm[iterM->first], "b");
            std::vector<int> branches = branchN.getBranchNum();
            this->branchNumMin[iterM->first] = branches[0];
            this->branchNumMax[iterM->first] = branches[1];
            iterM++;
        }

        this->pathSuffix = this->cipherName + "/" + this->mode + "/xor" + std::to_string(this->xorConSel) +
                           "_sbox" + std::to_string(this->sboxConSel) + "_matrix" + std::to_string(this->matrixConSel);
        switch (this->redMode) {
            case 1:
                this->pathSuffix += "_greedy_sun";
                break;
            case 2:
                this->pathSuffix += "_sub_milp";
                break;
            case 3:
                this->pathSuffix += "_convex_hull_tech";
                break;
            case 4:
                this->pathSuffix += "_logic_cond";
                break;
            case 5:
                this->pathSuffix += "_comb233";
                break;
            case 6:
                this->pathSuffix += "_superball";
                break;
            case 7:
                this->pathSuffix += "_cnf";
                break;
            default:
                this->pathSuffix += "_greedy_sun";
                break;
        }

        if (speedup1) this->pathSuffix += "_speedup1";
        if (speedup2) this->pathSuffix += "_speedup2";
        this->pathSuffix += "/";
    }

    // return true -> injective     false -> noninjective
    static bool sboxInjectiveCheck(std::vector<int> sbox) {
        set<int> arr;
        pair<set<int>::iterator,bool> pr;
        for(auto it=sbox.begin();it<sbox.end();it++){
            pr = arr.insert(*it);
            if(!pr.second){
                return false;
            }
        }
        return true;
    }

    void sboxSizeGet(std::string name, std::vector<int> sbox) {
        sboxInputSize[name] = int(log2(sbox.size()));
        map<int, int> sboxMap;
        for (int & i : sbox) {
            sboxMap[i]++;
        }
        sboxOutputSize[name] = int(log2(sboxMap.size()));
    }


    //////////////////////////////////////////////
    // 2023.5.31 添加注释：
    // 对于有 symbol 的value，我们处理方式是：
    // 如果有symbol，我们将其命名位name+SYMBOLINDEX，从而不会覆盖env中原有的name标记的value
    // 而如果我们对这种symbol的value再进一步取某一位，那么这个bit一定是常量的某一个位。
    // 此时对包含 symbol 的某一位的取值的三地址对象的名字一定包含字符串 "_symbol_",
    // 因此，我们可以判定, 若一个对象的名字中包含字符串 "_symbol_", 它一定是个常数
    bool isConstant(ThreeAddressNodePtr threeAddressNodePtr) {
        ThreeAddressNodePtr left = threeAddressNodePtr->getLhs();
        ThreeAddressNodePtr right = threeAddressNodePtr->getRhs();

        // 若左右两个孩子节点有一个已经判断为常数，那么本节点也是常数
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
        // 本节点是否为常数
        if (counter == threeAddressNodePtr->getNodeName().size()) {
            this->constantTan.push_back(threeAddressNodePtr->getNodeName());
            return true;
        }

        // 本节点是round function参数r的某一个bit
        if (left) {
            if ((threeAddressNodePtr->getOp() == ASTNode::SYMBOLINDEX and left->getNodeType() == NodeType::PARAMETER)) {
                this->constantTan.push_back(threeAddressNodePtr->getNodeName());
                return true;
            }
        }

        return false;
    }

    // extract index form the ThreeAddressNode instance whose op is touint or boxindex
    std::vector<int> extIdxFromTOUINTorBOXINDEX(const ThreeAddressNodePtr& input) {
        std::vector<int> extIdx;
        if (input->getOp() == ASTNode::TOUINT) {
            ThreeAddressNodePtr left = input->getLhs();
            while (left->getOp() == ASTNode::BOXINDEX) {
                // Before the specific treatment, we need to add a judgment to  whether the input three-address instances
                // have been stored in the map of tanNameMxIndex.
                // If there is, there is no need to repeat the processing and directly end the operation of the function
                if (tanNameMxIndex.count(left->getLhs()->getNodeName()) != 0) {
                    extIdx.push_back(tanNameMxIndex.find(left->getLhs()->getNodeName())->second);
                    left = left->getRhs();
                }
                    // If it is not the first round, i.e. rtnRecorder is not empty,
                    // the first element of rtnRecorder is taken each time as the ordinal number of x
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
                    // tanNameMxIndex.count(left->getLhs()->getNodeName()) == 0说明该元素还没有对应的xCounter的map
                    // 但是其可能是某个uints的某一位，而这个uints已经被map，所以需要寻找该uints的对应某一位的map
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
                tanNameMxIndex[left->getNodeName()] = xCounter;
                extIdx.push_back(xCounter);
                xCounter++;
            }
        } else assert(false);
        return extIdx;
    }

    static std::vector<int> shiftVec(std::vector<int> input, int shiftNum) {
        std::vector<int> rtn;
        for (int i = shiftNum; i < input.size(); ++i) {
            rtn.push_back(input[i]);
        }
        for (int i = rtn.size(); i < input.size(); ++i) {
            rtn.push_back(0);
        }
        return rtn;
    }

    static int transNodeTypeSize(NodeType nodeType) {
        if (nodeType == UINT) {
            return 0;
        }
        else if (nodeType == UINT1) {
            return 1;
        }
        else if (nodeType == UINT4) {
            return 4;
        }
        else if (nodeType == UINT6) {
            return 6;
        }
        else if (nodeType == UINT8) {
            return 8;
        }
        else if (nodeType == UINT12) {
            return 12;
        }
        else if (nodeType == UINT16) {
            return 16;
        }
        else if (nodeType == UINT32) {
            return 32;
        }
        else if (nodeType == UINT64) {
            return 64;
        }
        else if (nodeType == UINT128) {
            return 128;
        }
        else if (nodeType == UINT256) {
            return 256;
        }
        return 0;
    }

};

#endif //EASYBC_LINEARSBMILP_H
