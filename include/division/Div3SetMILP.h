//
// 3-subset bit-based division property (BDPT) MILP Manager — Algorithm 3 + 4.
//
// Coexists with the 2-subset Div2SetMILP (CBDP). Selected via the `-div3`
// subcommand. The full pipeline (S-box K/L division-trail inequalities,
// Key-XOR cross propagation, per-output-bit decision) is built incrementally;
// see doc/three_subset_bdpt_plan.md.
//
// Phase 2 (current): the TAC walker is copied from Div2SetMILP and parameterized
// by a per-round chain mode (CHAIN_K uses the O_k inequalities, CHAIN_L uses the
// O_l inequalities loaded from <sbox>_L_Reduce_Inequalities.txt). MGR() emits two
// standalone models — a pure K-chain (a faithful clone of the Div2 model, used as
// a zero-regression diff anchor) and a pure L-chain (M_L, the full r-round L
// propagation). Key-XOR cross propagation (Phase 3) and the per-bit decision /
// counting solver (Phase 4) are added later.
//
// IMPORTANT (decision 2, doc §3): the walker below is a deliberate COPY of the
// Div2SetMILP walker, NOT a shared base class. Keep both in sync; any drift in
// the K-chain logic must be mirrored here (and is guarded by the Phase 2 diff
// against the Div2 model). Div2SetMILP is never modified.
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
#include "BdptConfig.h"
#include "BdptSolveResult.h"
#include "BdptKeyXor.h"

#include "gurobi_c++.h"

class Div3SetMILP {

public:
    // Which S-box division-trail inequality set the walker emits for the
    // current round. CHAIN_K -> O_k (sboxDivIneqs), CHAIN_L -> O_l
    // (sboxLDivIneqs). In Phase 2 it is fixed per whole model build; in Phase 3
    // programGenModel() flips it at the Key-XOR split round t.
    enum ChainMode { CHAIN_K = 0, CHAIN_L = 1 };

private:
    std::string cipherName;
    std::vector<ProcedureHPtr> procedureHs;

    int rounds;
    std::string activebitsSpec;
    int blockSize = 0;

    int gurobiTimer = 3600 * 24;
    int gurobiThreads = 8;

    // EasyBC's production -div3 path reports NBB by default. The M_L parity
    // labeler is retained as an internal refinement hook, but it is not part of
    // the normal automatic distinguisher search because Table-1-style results
    // only require balanced-bit coordinates.
    bool signLabeling = false;
    bool reproduction = false;
    BdptCrossMode crossMode = BdptCrossMode::Exact;
    BdptUnitSearchMode unitSearchMode = BdptUnitSearchMode::Hybrid;

    // Lazy COPY-on-read is only needed for fan-out > 1 (a state bit read by
    // several operations, e.g. SIMON/Simeck where l_input feeds p1/p2/p3). The
    // SPN ciphers handled here (PRESENT/Rectangle/GIFT) are bit-permutations with
    // NO fan-out: every bit is read exactly once per round, so each consumeCopy()
    // split is an identity that only bloats the model (~3008 -> ~1300 vars for
    // PRESENT-9r) and slows Gurobi. Default off; a future SIMON/Feistel path
    // (Phase 6) sets it true. Correctness is unchanged for fan-out-1 ciphers.
    bool lazyCopyEnabled = false;

    std::string pathPrefix;
    std::string runDir;
    std::string modelPath;
    std::string resultsPath;

    std::map<std::string, std::vector<int>> Box;
    // O_k inequalities (CBDP K-trail), read from <sbox>_Reduce_Inequalities.txt.
    std::map<std::string, std::vector<std::vector<int>>> sboxDivIneqs;
    // O_l inequalities (BDPT L-trail), read from <sbox>_L_Reduce_Inequalities.txt.
    std::map<std::string, std::vector<std::vector<int>>> sboxLDivIneqs;
    std::map<std::string, int> sboxInputSize;
    std::map<std::string, int> sboxOutputSize;

    // Active S-box inequality selector for the current round (see ChainMode).
    ChainMode chainMode = CHAIN_K;

    // Key-XOR cross propagation state (Algorithm 3).
    //   keyXorLayers        = all Key-XOR layers discovered from the IR/TAC.
    //   selectedCrossLayer  = layer id selected for the current M_t, or -1 for
    //                         pure K/L builds.
    //   currentKeyXorLayer  = layer id currently being emitted by the TAC walker.
    //   pureMode/currentRound keep pure-chain and log bookkeeping.
    //   crossLBits/crossKBits collect the selected layer variables for
    //                         K_t* = L_t OR e_j constraints.
    std::vector<BdptKeyXorLayer> keyXorLayers;
    int selectedCrossLayer = -1;
    int currentKeyXorLayer = -1;
    ChainMode pureMode = CHAIN_K;
    int currentRound = 0;
    std::vector<int> crossLBits;
    std::vector<int> crossKBits;

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

    // Phase 5 (copied from Div2SetMILP): lazy COPY-on-read. Each read of a live
    // MILP variable allocates (a, b) and emits x_live = x_a + x_b; `a` is the
    // consumer's copy, `b` becomes the new live representative. Dead tails are
    // pinned to 0 in programGenModel(), telescoping the chain into an exact
    // N-way COPY.
    std::map<int, int> liveChain;
    int consumeCopy(int rawIdx);

    // Clear all per-build mutable walker state so MGR() can build several models
    // (K-chain, L-chain, and later the per-t model set) from one instance. The
    // loaded inequalities / S-box sizes / Box persist across resets.
    void resetState();

    // Shared .lp finalizer: reads the round-function constraints already
    // written to modelPath, then rewrites the file as objective (Minimize sum
    // of outputBitIndices) + initial L/activebit constraints + constraints +
    // Binary section. Uses the current walker state (xCounter / blockSize /
    // outputBitIndices). modeTag only labels the log line.
    void writeLpFile(const std::string& modeTag);

    // Build one pure single-mode model (no Key-XOR cross) into modelFile.
    // CHAIN_K -> Div2-equivalent K-chain diff anchor; CHAIN_L -> M_L (the full
    // r-round L propagation used by Algorithm 4's parity test). Resets state.
    void buildChainModel(ChainMode mode, const std::string& modelFile);

    // Build model M_t (Algorithm 3): use O_l before the selected Key-XOR layer,
    // cross L_t -> K_t* at that exact IR layer, then use O_k to K_r*.
    // Objective Minimize sum k_i^r*. Resets state.
    void buildMtModel(int modelNumber, int keyXorLayerId, const std::string& modelFile);

    // Algorithm 4 unknown test. The dispatcher selects per-coordinate
    // feasibility, explicit minimize-and-pin, or the production hybrid wrapper
    // over the paper/reference minimize-and-pin loop. All return structured
    // completeness, status and timing data; `skip` contains coordinates already
    // known unknown.
    BdptSolveResult solveMtReachableCoords(const std::string& lpFile,
                                           const std::vector<int>& outIdx,
                                           const std::set<int>& skip);
    BdptSolveResult solveMtReachableCoordsPerBit(const std::string& lpFile,
                                                 const std::vector<int>& outIdx,
                                                 const std::set<int>& skip);
    BdptSolveResult solveMtReachableCoordsMinPin(const std::string& lpFile,
                                                 const std::vector<int>& outIdx,
                                                 const std::set<int>& skip);
    BdptSolveResult solveMtReachableCoordsHybrid(const std::string& lpFile,
                                                 const std::vector<int>& outIdx,
                                                 const std::set<int>& skip);

    // Algorithm 4 (parity test): count the r-round pure-L trails of M_L that
    // reach ell^r = e_coord (fix outIdx[coord]=1, all other outputs=0) and return
    // the parity of that count. mlLpFile is the M_L model written by
    // buildChainModel(CHAIN_L, ...); mlOutIdx[j] is the MILP var of L_r
    // coordinate j. Returns:
    //    0  even number of solutions  => q-th output bit sum is 0 (balanced),
    //    1  odd  number of solutions  => q-th output bit sum is 1 (constant one),
    //   -1  the solve hit the time budget (gurobiTimer) before enumerating all
    //       solutions (parity unknown; the bit stays balanced, labelled 'b'),
    //   -2  the count hit the solution-pool cap (parity unknown, 'b'),
    //   -3  M_L failed the free-binary-variable sanity check (a variable in no
    //       constraint doubles every count, so no parity can be trusted; 'b').
    // solCount returns the enumerated (possibly capped) count for logging.
    int classifyMLParity(const std::string& mlLpFile,
                         const std::vector<int>& mlOutIdx,
                         int coord, long long& solCount);

    // Orchestrate the BDPT search: build + solve each M_t, union the reachable
    // (unknown) coordinates, and report the balanced coordinates (complement).
    void searchDistinguisher();

public:
    Div3SetMILP(std::vector<ProcedureHPtr> procedureHs, int rounds,
                const std::string& activebitsSpec, const std::string& cipherName);

    std::vector<int> resolveActiveBitVars() const;

    void setGurobiTimer(int timer) { this->gurobiTimer = timer; }
    void setGurobiThreads(int threads) { this->gurobiThreads = threads; }
    void setSignLabeling(bool s) { this->signLabeling = s; }
    void setReproduction(bool enabled) { this->reproduction = enabled; }
    void setCrossMode(BdptCrossMode mode) { this->crossMode = mode; }
    void setUnitSearchMode(BdptUnitSearchMode mode) { this->unitSearchMode = mode; }

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

    // Helper methods (copied verbatim from Div2SetMILP).
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
