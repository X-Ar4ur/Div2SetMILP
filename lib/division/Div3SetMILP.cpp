#include "division/Div3SetMILP.h"
#include "division/BdptActiveBits.h"
#include "util/setup.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>

extern std::map<std::string, std::vector<int>> allBox;
extern std::string cipherName;

// =============================================================================
// 三子集 BDPT MILP walker。
//
// TAC 遍历逻辑沿用二子集后端的语义动作；差异在于 S 盒根据 chainMode
// 选择 O_k/O_l，不同模型通过 Key-XOR 层切换链模式。
// =============================================================================

Div3SetMILP::Div3SetMILP(std::vector<ProcedureHPtr> procedureHs, int rounds,
                         const std::string& activebitsSpec, const std::string& cipherName)
        : procedureHs(std::move(procedureHs)), rounds(rounds), activebitsSpec(activebitsSpec) {
    this->Box = allBox;
    this->cipherName = cipherName;
    this->pathPrefix = std::string(DPATH) + "division/" + this->cipherName + "/";
}


std::vector<int> Div3SetMILP::resolveActiveBitVars() const {
    return resolveBdptActiveBitVars(this->cipherName, this->blockSize,
                                    this->activebitsSpec);
}


// 消费当前 live 变量的一份 COPY。
int Div3SetMILP::consumeCopy(int rawIdx) {
    if (rawIdx <= 0) return rawIdx;
    int live = rawIdx;
    while (this->liveChain.count(live)) live = this->liveChain[live];
    // SPN (no fan-out): skip the identity split entirely (see header note).
    if (!this->lazyCopyEnabled) return live;
    int a = this->xCounter++;
    int b = this->xCounter++;
    DivMILPcons::divCopyC(this->modelPath, live, std::vector<int>{a, b});
    this->liveChain[live] = b;
    return a;
}


void Div3SetMILP::resetState() {
    this->xCounter = 1;
    this->blockSize = 0;
    this->tanNameMxIndex.clear();
    this->rtnMxIndex.clear();
    this->rtnIdxSave.clear();
    this->sboxNameMxIndex.clear();
    this->sboxRtnIdxSave.clear();
    this->outputBitIndices.clear();
    this->constantTan.clear();
    this->consTanNameMxVal.clear();
    this->liveChain.clear();
    this->selectedCrossLayer = -1;
    this->currentKeyXorLayer = -1;
    this->currentRound = 0;
    this->crossLBits.clear();
    this->crossKBits.clear();
}


void Div3SetMILP::preprocess() {
    auto _bench_t0 = std::chrono::steady_clock::now();
    int _bench_total_ineqs = 0;
    int _bench_total_l_ineqs = 0;
    auto iterator = this->Box.begin();
    while (iterator != this->Box.end()) {
        if (iterator->first.substr(0, 4) == "sbox") {
            std::string sboxName = iterator->first;
            sboxSizeGet(sboxName, iterator->second);
            int expectedSize = sboxInputSize[sboxName] + sboxOutputSize[sboxName] + 1;

            // Lambda to load one inequality file into a target map. The K and L
            // files share the same [in‖out]+b dimension over the n-bit S-box.
            auto loadIneqFile = [&](const std::string& ineqFile,
                                    std::map<std::string, std::vector<std::vector<int>>>& target,
                                    const char* tag) -> int {
                std::ifstream file(ineqFile);
                if (!file) {
                    std::cout << "ERROR: Cannot open " << tag << " reduced inequalities file: " << ineqFile << std::endl;
                    std::cout << "Reduced inequalities are generated automatically by "
                              << "./EasyBC -div3 " << this->cipherName << " [rounds] [activebits] "
                              << "before BDPT MILP Modeling starts." << std::endl;
                    assert(false);
                }
                std::vector<std::vector<int>> ineqs;
                std::string line;
                int lineNo = 0;
                while (std::getline(file, line)) {
                    lineNo++;
                    if (line.empty()) continue;
                    std::vector<int> ineq;
                    std::istringstream iss(line);
                    int val;
                    while (iss >> val) ineq.push_back(val);
                    if (!ineq.empty()) {
                        if ((int)ineq.size() != expectedSize) {
                            std::cout << "ERROR: Invalid " << tag << " reduced inequality in file: " << ineqFile << std::endl;
                            std::cout << "line: " << lineNo << ", expected " << expectedSize
                                      << " coefficients, but got " << ineq.size() << "." << std::endl;
                            assert(false);
                        }
                        ineqs.push_back(ineq);
                    }
                }
                file.close();
                target[sboxName] = ineqs;
                return (int)ineqs.size();
            };

            // O_k inequalities (same file Div2SetMILP reads — never modified).
            std::string kFile = this->pathPrefix + sboxName + "_Reduce_Inequalities.txt";
            int nK = loadIneqFile(kFile, this->sboxDivIneqs, "K");
            _bench_total_ineqs += nK;

            // O_l inequalities (3-subset BDPT, written with the _L_ tag).
            std::string lFile = this->pathPrefix + sboxName + "_L_Reduce_Inequalities.txt";
            int nL = loadIneqFile(lFile, this->sboxLDivIneqs, "L");
            _bench_total_l_ineqs += nL;

            std::cout << "Loaded " << nK << " K + " << nL << " L reduced inequalities for " << sboxName
                      << " (input=" << sboxInputSize[sboxName] << ", output=" << sboxOutputSize[sboxName] << ")" << std::endl;
        }
        iterator++;
    }
    auto _bench_t1 = std::chrono::steady_clock::now();
    long long _bench_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t1 - _bench_t0).count();
    std::cerr << "[BENCH] phase=preprocess cipher=" << this->cipherName
              << " subset=3"
              << " elapsed_ms=" << _bench_ms
              << " n_ineq_loaded=" << _bench_total_ineqs
              << " n_l_ineq_loaded=" << _bench_total_l_ineqs << std::endl;
}


void Div3SetMILP::MGR() {
    std::cout << "\n===== Step 3: 3-subset BDPT MILP Modeling =====" << std::endl;
    std::cout << "Cipher: " << this->cipherName
              << ", Rounds: " << this->rounds
              << ", Active bits: " << this->activebitsSpec
              << ", Strategy: paper-alg3-alg4-min-pin"
              << ", Output: sum0/sum1/unknown" << std::endl;

    preprocess(); // 加载 O_k 与 O_l 两套 S 盒可分迹不等式。

    this->runDir = this->pathPrefix + "milp/";
    (void)system(("mkdir -p " + this->runDir).c_str());
    this->resultsPath = this->runDir + "result_" + std::to_string(this->rounds)
                        + "_" + this->activebitsSpec + "_subset3.txt";

    // 运行论文 Algorithm 4：模型集 P 判 unknown，M_L 判 sum=0/1。
    searchDistinguisher();

    std::cout << "\n===== 3-subset BDPT distinguisher search complete =====" << std::endl;
}


BdptSolveResult Div3SetMILP::solveMtReachableCoords(
        const std::string& lpFile,
        const std::vector<int>& outIdx,
        const std::set<int>& skip) {
    return solveMtReachableCoordsMinPin(lpFile, outIdx, skip);
}


BdptSolveResult Div3SetMILP::solveMtReachableCoordsMinPin(
        const std::string& lpFile,
        const std::vector<int>& outIdx,
        const std::set<int>& skip) {
    BdptSolveResult result;
    result.strategy = "min-pin";

    GRBEnv env = GRBEnv(true);
    env.set(GRB_IntParam_Threads, this->gurobiThreads);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.start();
    GRBModel model = GRBModel(env, lpFile);
    model.set(GRB_DoubleParam_TimeLimit, this->gurobiTimer);
    model.set(GRB_DoubleParam_NodefileStart, 0.5);
    // 论文 Algorithm 4 需要判断每个 K_q 是否可达。这里按论文目标函数
    // Minimize sum k_i^{r*} 做 min-pin 枚举：每找到一个权重为 1 的输出单位
    // 向量就把该坐标固定为 0，再继续找下一个单位向量。
    model.set(GRB_DoubleParam_BestObjStop, 1.0);

    std::vector<GRBVar> outVars(outIdx.size());
    for (int j = 0; j < (int)outIdx.size(); ++j) {
        outVars[j] = model.getVarByName("x" + std::to_string(outIdx[j]));
    }

    GRBLinExpr sumRemainingExpr = 0;
    int remaining = 0;
    for (int q : skip) {
        if (q >= 0 && q < (int)outVars.size()) outVars[q].set(GRB_DoubleAttr_UB, 0.0);
    }
    for (int q = 0; q < (int)outVars.size(); ++q) {
        if (skip.count(q)) continue;
        sumRemainingExpr += outVars[q];
        remaining++;
    }
    if (remaining == 0) {
        return result;
    }
    model.addConstr(sumRemainingExpr >= 1, "bdpt_min_pin_nonzero_output");
    model.update();

    auto started = std::chrono::steady_clock::now();
    while (true) {
        double elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        double remainingSeconds = (double)this->gurobiTimer - elapsed;
        if (remainingSeconds <= 0.0) {
            result.markIncomplete(GRB_TIME_LIMIT, "min-pin-time-budget-exhausted");
            break;
        }
        model.set(GRB_DoubleParam_TimeLimit, remainingSeconds);

        auto solveStart = std::chrono::steady_clock::now();
        model.optimize();
        auto solveEnd = std::chrono::steady_clock::now();
        result.solveCount++;
        result.solverSeconds += std::chrono::duration<double>(
            solveEnd - solveStart).count();
        int status = model.get(GRB_IntAttr_Status);
        result.lastStatus = status;
        result.iterationStatuses.push_back(status);
        int solCount = model.get(GRB_IntAttr_SolCount);
        // objVal >= 0, so round-to-nearest is (int)(v + 0.5); -1 marks "no incumbent".
        int objR = (solCount > 0) ? (int)(model.get(GRB_DoubleAttr_ObjVal) + 0.5) : -1;

        // 由于存在 sumRemainingExpr >= 1，权重 1 incumbent 已经是单位输出。
        if (solCount > 0 && objR == 1) {
            int setIdx = -1;
            for (int j = 0; j < (int)outVars.size(); ++j) {
                if (outVars[j].get(GRB_DoubleAttr_X) > 0.5) { setIdx = j; break; }
            }
            if (setIdx < 0) {
                result.markIncomplete(status, "weight-one-solution-without-set-output");
                break;
            }
            result.reachable.insert(setIdx);
            result.coordinateStatus[setIdx] = status;
            outVars[setIdx].set(GRB_DoubleAttr_UB, 0.0);
            model.update();
            continue;
        }

        if (status == GRB_OPTIMAL || status == GRB_INFEASIBLE) {
            break;
        }

        std::cout << "WARNING: Gurobi status " << status << " (no proof) for "
                  << lpFile << "; model result is incomplete." << std::endl;
        result.markIncomplete(status, "unsettled-min-pin-terminal-proof");
        break;
    }
    return result;
}


int Div3SetMILP::classifyMLParity(const std::string& lpFile,
                                  const std::vector<int>& outIdx,
                                  int coord,
                                  int& solutionCount) {
    solutionCount = 0;
    int parity = 0;

    GRBEnv env = GRBEnv(true);
    env.set(GRB_IntParam_Threads, this->gurobiThreads);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.start();
    GRBModel model = GRBModel(env, lpFile);
    model.set(GRB_DoubleParam_NodefileStart, 0.5);

    for (int j = 0; j < (int)outIdx.size(); ++j) {
        GRBVar v = model.getVarByName("x" + std::to_string(outIdx[j]));
        const double fixed = (j == coord) ? 1.0 : 0.0;
        v.set(GRB_DoubleAttr_LB, fixed);
        v.set(GRB_DoubleAttr_UB, fixed);
    }

    GRBVar* rawVars = model.getVars();
    const int numVars = model.get(GRB_IntAttr_NumVars);
    std::vector<GRBVar> vars(rawVars, rawVars + numVars);
    delete[] rawVars;
    model.update();

    auto started = std::chrono::steady_clock::now();
    while (true) {
        double elapsed = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();
        double remainingSeconds = (double)this->gurobiTimer - elapsed;
        if (remainingSeconds <= 0.0) {
            return -1;
        }
        model.set(GRB_DoubleParam_TimeLimit, remainingSeconds);
        model.optimize();
        const int status = model.get(GRB_IntAttr_Status);
        const int solCount = model.get(GRB_IntAttr_SolCount);

        if (status == GRB_INFEASIBLE) {
            return parity;
        }
        if (status != GRB_OPTIMAL || solCount <= 0) {
            return -1;
        }

        solutionCount++;
        parity ^= 1;

        GRBLinExpr noGood = 0;
        for (GRBVar& v : vars) {
            if (v.get(GRB_DoubleAttr_X) > 0.5) noGood += 1 - v;
            else noGood += v;
        }
        model.addConstr(noGood >= 1,
                        "bdpt_ml_nogood_" + std::to_string(solutionCount));
        model.update();
    }
}


void Div3SetMILP::searchDistinguisher() {
    auto _bench_t0 = std::chrono::steady_clock::now();

    std::string base = this->runDir + this->cipherName + "_" + std::to_string(this->rounds)
                       + "_" + this->activebitsSpec + "_subset3";

    this->keyXorLayers = discoverBdptKeyXorLayers(this->procedureHs, this->rounds);
    if (this->keyXorLayers.empty()) {
        std::cout << "ERROR: No Key-XOR layer was discovered from the IR; "
                  << "BDPT model set P cannot be built soundly." << std::endl;
        assert(false);
    }
    std::vector<BdptScheduledCrossLayer> scheduledCrossLayers =
        scheduleBdptCrossLayers(this->keyXorLayers, this->rounds);
    if (scheduledCrossLayers.empty()) {
        std::cout << "ERROR: No semantic Key-XOR cross layer was selected; "
                  << "BDPT model set P cannot be built soundly." << std::endl;
        assert(false);
    }

    std::string mlFile = base + "_ML.lp";
    buildMLModel(mlFile);
    std::vector<int> mlOutIdx = this->outputBitIndices;
    const int resultBlockSize = this->blockSize;

    std::set<int> unknownCoords;
    int nModels = (int)scheduledCrossLayers.size();
    int modelsAttempted = 0;
    bool searchComplete = true;
    std::string incompleteReason;

    std::ofstream result(this->resultsPath, std::ios::trunc);
    result << "===== 3-subset BDPT integral distinguisher search =====\n";
    result << "Cipher: " << this->cipherName << ", Rounds: " << this->rounds
           << ", Active bits: " << this->activebitsSpec << "\n";
    result << "Strategy: paper-alg3-alg4-min-pin\n";
    result << "M_L model: " << mlFile << "\n";
    result << "Key-XOR layers discovered from IR: " << this->keyXorLayers.size() << "\n";
    for (const auto& layer : this->keyXorLayers) {
        result << "  keyxor#" << layer.id
               << "@round " << layer.round
               << " function=" << layer.roundFunction
               << " xor_nodes=" << layer.xorNodeNames.size()
               << " first_node=" << layer.firstNodeIndex
               << " before_round_core=" << (layer.beforeRoundCore ? 1 : 0)
               << "\n";
    }
    result << "Semantic model scheduler selected " << nModels << " cross layers:\n";
    for (const auto& item : scheduledCrossLayers) {
        result << "  M_" << item.modelNumber
               << " -> keyxor#" << item.layer.id
               << "@round " << item.layer.round
               << " fe_rounds_before_cross=" << item.feRoundsBeforeCross
               << "\n";
    }
    result << "Model set P = {M_1, ..., M_" << nModels << "}\n\n";
    result.close();

    for (const BdptScheduledCrossLayer& scheduled : scheduledCrossLayers) {
        const int tau = scheduled.modelNumber;
        modelsAttempted++;
        const BdptKeyXorLayer& layer = scheduled.layer;
        std::string lpFile = base + "_Mt" + std::to_string(tau) + ".lp";
        buildMtModel(tau, layer.id, lpFile);
        // outputBitIndices now holds M_t's K_r* coordinates (index j = coord j).
        std::vector<int> outIdx = this->outputBitIndices;
        std::set<int> solverSkip = unknownCoords;
        const int skippedBefore = (int)unknownCoords.size();
        int remainingCandidates = (int)outIdx.size() - skippedBefore;

        // Pass the running unknown set as skip: a coordinate already shown
        // unknown by an earlier M_t need not be re-found here (the union is all
        // that matters); pinning it to 0 also tightens this model's minimize.
        BdptSolveResult solveResult =
            solveMtReachableCoords(lpFile, outIdx, solverSkip);
        std::set<int> reach = solveResult.reachable;
        if (!solveResult.complete) {
            searchComplete = false;
            incompleteReason = "M_" + std::to_string(tau) + ":"
                               + solveResult.reason;
            // Conservative fallback: an unproved coordinate remains unknown and
            // can never be reported as balanced.
            for (int q = 0; q < (int)outIdx.size(); ++q) {
                if (!unknownCoords.count(q) && !reach.count(q)) {
                    reach.insert(q);
                }
            }
        }
        for (int j : reach) unknownCoords.insert(j);

        std::ofstream r(this->resultsPath, std::ios::app);
        r << "M_" << tau << " (keyxor#" << layer.id << "@round "
          << layer.round
          << ", fe_rounds_before_cross=" << scheduled.feRoundsBeforeCross
          << "): reachable unit coords = {";
        bool first = true;
        for (int j : reach) { r << (first ? "" : ",") << j; first = false; }
        r << "}  (" << reach.size() << ")"
          << " complete=" << (solveResult.complete ? 1 : 0)
          << " solves=" << solveResult.solveCount
          << " solver_seconds=" << solveResult.solverSeconds
          << " status=" << solveResult.lastStatus
          << " strategy=" << solveResult.strategy
          << " skipped_unknowns=" << skippedBefore
          << " remaining_candidates=" << remainingCandidates;
        if (!solveResult.reason.empty()) r << " reason=" << solveResult.reason;
        if (!solveResult.coordinateStatus.empty()) {
            r << " coord_statuses={";
            bool firstStatus = true;
            for (const auto& item : solveResult.coordinateStatus) {
                r << (firstStatus ? "" : ",") << item.first << ":" << item.second;
                firstStatus = false;
            }
            r << "}";
        }
        if (!solveResult.iterationStatuses.empty()) {
            r << " iteration_statuses={";
            for (size_t i = 0; i < solveResult.iterationStatuses.size(); ++i) {
                r << (i == 0 ? "" : ",") << solveResult.iterationStatuses[i];
            }
            r << "}";
        }
        r << "\n";
        r.close();

        std::cerr << "[BENCH] phase=solve_mt cipher=" << this->cipherName
                  << " subset=3 model=Mt" << tau
                  << " cross_layer=" << layer.id
                  << " cross_round=" << layer.round
                  << " fe_rounds_before_cross=" << scheduled.feRoundsBeforeCross
                  << " strategy=" << solveResult.strategy
                  << " skipped_unknowns=" << skippedBefore
                  << " remaining_candidates=" << remainingCandidates
                  << " n_reachable=" << reach.size()
                  << " complete=" << (solveResult.complete ? 1 : 0)
                  << " solve_count=" << solveResult.solveCount
                  << " solver_seconds=" << solveResult.solverSeconds
                  << " status=" << solveResult.lastStatus << std::endl;

    }

    std::vector<int> sum0;
    std::vector<int> sum1;
    std::vector<int> unresolved;
    for (int q = 0; q < resultBlockSize; ++q) {
        if (unknownCoords.count(q)) continue;
        int solutionCount = 0;
        int parity = classifyMLParity(mlFile, mlOutIdx, q, solutionCount);
        std::ofstream r(this->resultsPath, std::ios::app);
        r << "M_L coord " << q
          << ": parity=" << parity
          << " solutions=" << solutionCount << "\n";
        r.close();

        if (parity == 0) sum0.push_back(q);
        else if (parity == 1) sum1.push_back(q);
        else {
            unresolved.push_back(q);
            searchComplete = false;
            if (incompleteReason.empty()) {
                incompleteReason = "M_L:unsettled-parity";
            }
        }
    }

    auto writeCoordSet = [](std::ofstream& os, const char* label,
                            const std::vector<int>& v) {
        os << label << " (" << v.size() << "): {";
        bool first = true;
        for (int j : v) { os << (first ? "" : ",") << j; first = false; }
        os << "}\n";
    };

    std::vector<int> balanced;
    balanced.insert(balanced.end(), sum0.begin(), sum0.end());
    balanced.insert(balanced.end(), sum1.begin(), sum1.end());

    std::ofstream r(this->resultsPath, std::ios::app);
    {
        r << "\nUnknown coords (" << unknownCoords.size() << "): {";
        bool f1 = true;
        for (int j : unknownCoords) { r << (f1 ? "" : ",") << j; f1 = false; }
        r << "}\n";
    }
    writeCoordSet(r, "Balanced bits (NBB)", balanced);
    writeCoordSet(r, "  - sum=0 coords", sum0);
    writeCoordSet(r, "  - sum=1 coords", sum1);
    writeCoordSet(r, "  - 'b' (sign unresolved) coords", unresolved);
    r << "\n";
    r << "Reproduction completeness: "
      << (searchComplete ? "COMPLETE" : "INCOMPLETE") << "\n";
    if (!searchComplete) {
        r << "Incomplete reason: " << incompleteReason << "\n";
        r << "Unsettled coordinates were excluded from reproducible NBB\n";
    }
    if (!balanced.empty())
        r << "Integral Distinguisher Found! (" << balanced.size() << " balanced bits)\n";
    else
        r << "Integral Distinguisher does NOT exist\n";
    r.close();

    auto _bench_t1 = std::chrono::steady_clock::now();
    long long _bench_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t1 - _bench_t0).count();

    std::cout << "\nBDPT result: " << balanced.size() << " balanced (NBB)";
    std::cout << ", " << unknownCoords.size() << " unknown (block size "
              << resultBlockSize << ")" << std::endl;
    if (!searchComplete)
        std::cout << "*** BDPT search incomplete; unresolved coordinates excluded from NBB: "
                  << incompleteReason << " ***" << std::endl;
    if (!balanced.empty())
        std::cout << "*** Integral Distinguisher Found! (" << balanced.size()
                  << " balanced bits) ***" << std::endl;
    else
        std::cout << "*** Integral Distinguisher does NOT exist ***" << std::endl;
    std::cout << "Results saved to: " << this->resultsPath << std::endl;

    std::cerr << "[BENCH] phase=solve cipher=" << this->cipherName
              << " subset=3"
              << " rounds=" << this->rounds
              << " activebits=" << this->activebitsSpec
              << " elapsed_ms=" << _bench_ms
              << " n_models=" << nModels
              << " n_models_attempted=" << modelsAttempted
              << " model_set_complete=" << (searchComplete ? 1 : 0)
              << " strategy=paper-alg3-alg4-min-pin"
              << " n_balanced=" << balanced.size()
              << " n_sum0=" << sum0.size()
              << " n_sum1=" << sum1.size()
              << " n_unresolved=" << unresolved.size()
              << " n_unknown=" << unknownCoords.size()
              << " block_size=" << resultBlockSize
              << " distinguisher_found=" << (balanced.empty() ? 0 : 1)
              << " threads=" << this->gurobiThreads << std::endl;
}


// Shared .lp finalizer. Assumes the round-function constraints have already
// been written to modelPath by programGenModel(); rewrites the file with the
// objective + initial constraints prepended and the Binary section appended.
void Div3SetMILP::writeLpFile(const std::string& modeTag) {
    std::ifstream file(this->modelPath);
    std::string constraints, line;
    while (std::getline(file, line)) { constraints += line + "\n"; }
    file.close();

    std::ofstream model(this->modelPath, std::ios::trunc);
    if (!model) {
        std::cout << "ERROR: Cannot write model file: " << this->modelPath << std::endl;
        assert(false);
    }

    // Objective: Minimize sum of final-round output bits (K_r* for M_t).
    model << "Minimize\n";
    for (int i = 0; i < (int)this->outputBitIndices.size(); ++i) {
        model << "x" << this->outputBitIndices[i];
        if (i < (int)this->outputBitIndices.size() - 1)
            model << " + ";
    }
    model << "\n";

    model << "Subject To\n";

    // Initial BDPT: the L-chain starts from ell^0 = activebits pattern (the
    // all-ones k chain is ignored by Stopping Rule 1, so every model here is
    // seeded with the same activebit initial as the L input).
    std::vector<int> activeVars = this->resolveActiveBitVars();
    std::set<int> activeSet(activeVars.begin(), activeVars.end());
    for (int xi : activeVars) {
        model << "x" << xi << " = 1\n";
    }
    for (int xi = 1; xi <= this->blockSize; ++xi) {
        if (activeSet.find(xi) == activeSet.end()) {
            model << "x" << xi << " = 0\n";
        }
    }

    model << constraints;
    model.close();

    std::ofstream binary(this->modelPath, std::ios::app);
    if (!binary) {
        std::cout << "ERROR: Cannot append to model file!" << std::endl;
        assert(false);
    }
    binary << "Binary\n";
    for (int i = 1; i < this->xCounter; ++i) {
        binary << "x" << i << "\n";
    }
    binary << "End";
    binary.close();

    std::cout << modeTag << " MILP model written to: " << this->modelPath
              << "  (x1.." << (this->xCounter - 1) << ")" << std::endl;
}


void Div3SetMILP::buildMLModel(const std::string& modelFile) {
    auto _bench_t0 = std::chrono::steady_clock::now();

    resetState();
    this->selectedCrossLayer = -1;
    this->chainMode = CHAIN_L;
    this->modelPath = modelFile;

    std::ofstream clearFile(this->modelPath, std::ios::trunc);
    clearFile.close();

    programGenModel();
    writeLpFile("ML");

    auto _bench_t1 = std::chrono::steady_clock::now();
    long long _bench_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t1 - _bench_t0).count();
    std::cerr << "[BENCH] phase=build cipher=" << this->cipherName
              << " subset=3 model=ML"
              << " rounds=" << this->rounds
              << " activebits=" << this->activebitsSpec
              << " elapsed_ms=" << _bench_ms
              << " n_xvars=" << (this->xCounter - 1)
              << " block_size=" << this->blockSize << std::endl;
}


void Div3SetMILP::buildMtModel(int modelNumber, int keyXorLayerId, const std::string& modelFile) {
    auto _bench_t0 = std::chrono::steady_clock::now();

    resetState();
    this->selectedCrossLayer = keyXorLayerId;
    this->chainMode = CHAIN_L;      // switch to O_k when the selected IR layer is crossed
    this->modelPath = modelFile;

    std::ofstream clearFile(this->modelPath, std::ios::trunc);
    clearFile.close();

    programGenModel();

    writeLpFile("Mt" + std::to_string(modelNumber));

    int crossLayerRound = -1;
    for (const auto& layer : this->keyXorLayers) {
        if (layer.id == keyXorLayerId) {
            crossLayerRound = layer.round;
            break;
        }
    }

    auto _bench_t1 = std::chrono::steady_clock::now();
    long long _bench_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t1 - _bench_t0).count();
    std::cerr << "[BENCH] phase=build cipher=" << this->cipherName
              << " subset=3 model=Mt" << modelNumber
              << " cross_layer=" << keyXorLayerId
              << " cross_round=" << crossLayerRound
              << " rounds=" << this->rounds
              << " activebits=" << this->activebitsSpec
              << " elapsed_ms=" << _bench_ms
              << " n_xvars=" << (this->xCounter - 1)
              << " block_size=" << this->blockSize << std::endl;
}


void Div3SetMILP::programGenModel() {
    int roundCounter = this->rounds;
    int processed = 0;
    for (const auto& proc : this->procedureHs) {
        if (proc->getName() == "main") {
            std::string roundFuncId;
            bool newRoundFlag = false;
            int tempSizeCounter = 0;
            for (const auto& ele : proc->getBlock()) {
                if (ele->getLhs()->getNodeType() == UINT) {
                    newRoundFlag = true;
                    this->rndParamR = stoi(ele->getLhs()->getNodeName());
                    continue;
                }
                if (newRoundFlag and roundCounter > 0) {
                    if (ele->getNodeName() == "plaintext_push")
                        tempSizeCounter++;
                    if (ele->getOp() == ASTNode::CALL) {
                        this->blockSize = tempSizeCounter;
                        tempSizeCounter = 0;

                        // M_t builds start in L-chain mode and switch to K-chain
                        // exactly when roundFunctionGenModel emits the selected
                        // IR Key-XOR layer.
                        this->currentRound = ++processed;

                        roundFuncId = ele->getLhs()->getNodeName().substr(0, ele->getLhs()->getNodeName().find("@"));
                        for (const auto& tproc : this->procedureHs) {
                            if (tproc->getName() == roundFuncId) {
                                roundFunctionGenModel(tproc);
                                break;
                            }
                        }
                        newRoundFlag = false;
                        roundCounter--;
                    }
                }
            }
            break;
        }
    }

    // Capture output bit indices from the last round's return values.
    this->outputBitIndices.clear();
    for (int & i : this->rtnIdxSave) {
        this->outputBitIndices.push_back(i);
    }
    // 将 COPY 链中不再被读取且不是输出的尾变量固定为 0。
    std::set<int> chainKeys;
    std::set<int> chainValues;
    for (const auto& kv : this->liveChain) {
        chainKeys.insert(kv.first);
        chainValues.insert(kv.second);
    }
    std::set<int> outSet(this->outputBitIndices.begin(), this->outputBitIndices.end());
    std::ofstream modelApp(this->modelPath, std::ios::app);
    for (int tail : chainValues) {
        if (chainKeys.count(tail)) continue;
        if (outSet.count(tail)) continue;
        modelApp << "x" << tail << " = 0\n";
    }
    modelApp.close();
}


void Div3SetMILP::roundFunctionGenModel(const ProcedureHPtr &procedureH) {
    this->constantTan.clear();
    this->consTanNameMxVal.clear();

    this->consTanNameMxVal[procedureH->getParameters().at(0).at(0)->getNodeName()] = this->rndParamR;
    this->constantTan.push_back(procedureH->getParameters().at(0).at(0)->getNodeName());

    if (!this->rtnIdxSave.empty()) {
        for (int i = 0; i < (int)this->rtnIdxSave.size(); ++i)
            this->tanNameMxIndex[procedureH->getParameters().at(2).at(i)->getNodeName()] = this->rtnIdxSave[i];
        this->rtnIdxSave.clear();
        this->rtnMxIndex.clear();
    } else {
        for (const auto & i : procedureH->getParameters().at(2)) {
            const int idx = this->xCounter;
            this->tanNameMxIndex[i->getNodeName()] = idx;
            this->xCounter++;
        }
    }

    bool functionCallFlag;
    bool previousWasKeyXor = false;
    for (int i = 0; i < (int)procedureH->getBlock().size(); ++i) {
        functionCallFlag = false;
        bool currentNodeIsKeyXor = false;
        ThreeAddressNodePtr ele = procedureH->getBlock().at(i);

        if (ele->getOp() == ASTNode::XOR) {
            const BdptKeyXorMatch keyXor = matchBdptKeyXorNode(ele, procedureH);
            if (keyXor.isKeyXor) {
                currentNodeIsKeyXor = true;
                if (!previousWasKeyXor) this->currentKeyXorLayer++;

                ThreeAddressNodePtr dataNode = keyXor.leftIsKey ? ele->getRhs() : ele->getLhs();
                auto it = this->tanNameMxIndex.find(dataNode->getNodeName());
                if (it == this->tanNameMxIndex.end()) assert(false);

                if (this->selectedCrossLayer >= 0 &&
                    this->currentKeyXorLayer == this->selectedCrossLayer) {
                    int lIdx = it->second;
                    int kIdx = this->xCounter++;
                    this->crossLBits.push_back(lIdx);
                    this->crossKBits.push_back(kIdx);
                    this->tanNameMxIndex[ele->getNodeName()] = kIdx;
                    this->chainMode = CHAIN_K;
                } else {
                    this->tanNameMxIndex[ele->getNodeName()] = it->second;
                }
                functionCallFlag = true;
            }
            else if (this->isConstant(ele->getLhs()) or this->isConstant(ele->getRhs())) {}
            else {
                functionCallFlag = true;
                XORGenModel(ele->getLhs(), ele->getRhs(), ele, false);
            }
        } else if (ele->getOp() == ASTNode::AND or ele->getOp() == ASTNode::OR) {
            functionCallFlag = true;
            ANDGenModel(ele->getLhs(), ele->getRhs(), ele, false);
        } else if (ele->getOp() == ASTNode::BOXOP) {
            functionCallFlag = true;
            if (ele->getLhs()->getNodeName().substr(0, 4) == "sbox") {
                SboxGenModel(ele->getLhs(), ele->getRhs(), ele);
            } else if (ele->getLhs()->getNodeName().substr(0, 4) == "pbox") {
                PboxGenModel(ele->getLhs(), ele->getRhs(), ele);
            }
        } else if (ele->getOp() == ASTNode::ADD or ele->getOp() == ASTNode::MINUS) {
            if (ele->getLhs()->getNodeType() == NodeType::UINT or ele->getRhs()->getNodeType() == NodeType::UINT) {
                previousWasKeyXor = false;
                continue;
            }
            if (isConstant(ele->getLhs()) or isConstant(ele->getRhs())) {
                previousWasKeyXor = false;
                continue;
            }
            std::cout << "WARNING: Modular ADD/MINUS operation encountered in BDPT division property analysis.\n"
                      << "  This is not supported (SPN ciphers only in the current phase)." << std::endl;
            assert(false);
        } else if (ele->getOp() == ASTNode::PUSH) {
            functionCallFlag = true;
            std::vector<ThreeAddressNodePtr> input, output;
            while (ele->getNodeName() == "sbox_push") {
                input.push_back(ele->getLhs());
                i++;
                ele = procedureH->getBlock().at(i);
            }
            std::string sboxFuncId = ele->getLhs()->getNodeName().substr(0, ele->getLhs()->getNodeName().find("@"));
            while (ele->getLhs()->getNodeName().substr(0, ele->getLhs()->getNodeName().find("@")) == sboxFuncId) {
                output.push_back(ele);
                i++;
                ele = procedureH->getBlock().at(i);
            }
            i--;
            ele = procedureH->getBlock().at(i);
            for (const auto& tproc : this->procedureHs) {
                if (tproc->getName() == sboxFuncId) {
                    sboxFunctionGenModel(tproc, input, output);
                    break;
                }
            }
        } else if (ele->getOp() == ASTNode::SYMBOLINDEX) {
            ThreeAddressNodePtr left = ele->getLhs();
            if (left->getOp() == ASTNode::FFTIMES) {
                std::cout << "WARNING: Matrix-vector multiplication (pboxm) in BDPT division property analysis.\n"
                          << "  Currently not supported. Use ciphers with simple pbox permutation." << std::endl;
                assert(false);
            }
        } else if (ele->getOp() == ASTNode::TOUINT) {
            if (ele->getLhs()->getOp() == ASTNode::BOXINDEX) {
                ThreeAddressNodePtr left = ele->getLhs();
                std::vector<int> leftIdx = extIdxFromTOUINTorBOXINDEX(left);
                int size = transNodeTypeSize(ele->getNodeType());
                for (int j = 0; j < size; ++j) {
                    this->tanNameMxIndex[ele->getNodeName() + "_$B$_" + std::to_string(j)] = leftIdx[j];
                }
            }
        } else if (ele->getOp() == ASTNode::MOD) {
            int leftVal, rightVal;
            if (this->consTanNameMxVal.count(ele->getLhs()->getNodeName()) != 0)
                leftVal = this->consTanNameMxVal[ele->getLhs()->getNodeName()];
            else if (ele->getLhs()->getOp() == ASTNode::NULLOP)
                leftVal = stoi(ele->getLhs()->getNodeName());
            else
                assert(false);

            if (this->consTanNameMxVal.count(ele->getRhs()->getNodeName()) != 0)
                rightVal = this->consTanNameMxVal[ele->getRhs()->getNodeName()];
            else if (ele->getRhs()->getOp() == ASTNode::NULLOP)
                rightVal = stoi(ele->getRhs()->getNodeName());
            else
                assert(false);

            this->consTanNameMxVal[ele->getNodeName()] = leftVal % rightVal;
            this->constantTan.push_back(ele->getNodeName());
        } else if (ele->getOp() == ASTNode::NOT) {
            if (this->tanNameMxIndex.count(ele->getLhs()->getNodeName()) != 0)
                this->tanNameMxIndex[ele->getNodeName()] = this->tanNameMxIndex[ele->getLhs()->getNodeName()];
            else
                assert(false);
        } else
            assert(false);

        if (!functionCallFlag) {
            if (ele->getLhs()->getNodeType() == NodeType::ARRAY and ele->getOp() == ASTNode::SYMBOLINDEX) {
                std::string finderName = ele->getLhs()->getNodeName() + "_$B$_" + ele->getRhs()->getNodeName();
                for (const auto &pair: this->tanNameMxIndex) {
                    if (finderName == pair.first)
                        this->tanNameMxIndex[ele->getNodeName()] = pair.second;
                }
            } else {
                for (const auto &pair: this->tanNameMxIndex) {
                    if (ele->getLhs() != nullptr)
                        if (ele->getLhs()->getNodeName() == pair.first)
                            this->tanNameMxIndex[ele->getNodeName()] = pair.second;
                    if (ele->getRhs() != nullptr)
                        if (ele->getRhs()->getNodeName() == pair.first)
                            this->tanNameMxIndex[ele->getNodeName()] = pair.second;
                }
            }
        } else
            functionCallFlag = false;
        previousWasKeyXor = currentNodeIsKeyXor;
    }

    // 论文 Algorithm 3 第 9-10 行：L_t 在 key 覆盖位置不全为 1，
    // 且 K_t* 按位支配 L_t。
    if (this->selectedCrossLayer >= 0 && !this->crossLBits.empty()) {
        BdptMILPcons::bdptCrossPaperC(
            this->modelPath, this->crossKBits, this->crossLBits);
        this->crossLBits.clear();
        this->crossKBits.clear();
    }

    this->rtnIdxSave.clear();
    this->rtnMxIndex.clear();
    for (const auto& rtn : procedureH->getReturns()) {
        for (const auto &pair: this->tanNameMxIndex) {
            if (rtn->getNodeName() == pair.first) {
                int idx = pair.second;
                while (this->liveChain.count(idx)) idx = this->liveChain[idx];
                this->rtnMxIndex[pair.first] = idx;
                this->rtnIdxSave.push_back(idx);
            }
        }
    }
}


void Div3SetMILP::sboxFunctionGenModel(const ProcedureHPtr &procedureH,
                                        std::vector<ThreeAddressNodePtr> input,
                                        std::vector<ThreeAddressNodePtr> output) {
    this->sboxNameMxIndex.clear();

    int idx = 0;
    for (const auto & i : procedureH->getParameters().at(0)) {
        this->sboxNameMxIndex[i->getNodeName()] = this->tanNameMxIndex[input[idx]->getNodeName()];
        idx++;
    }

    bool functionCallFlag;
    for (int i = 0; i < (int)procedureH->getBlock().size(); ++i) {
        functionCallFlag = false;
        ThreeAddressNodePtr ele = procedureH->getBlock().at(i);
        if (ele->getOp() == ASTNode::XOR) {
            if (this->isConstant(ele->getLhs()) or this->isConstant(ele->getRhs())) {}
            else {
                functionCallFlag = true;
                XORGenModel(ele->getLhs(), ele->getRhs(), ele, true);
            }
        } else if (ele->getOp() == ASTNode::AND or ele->getOp() == ASTNode::OR) {
            functionCallFlag = true;
            ANDGenModel(ele->getLhs(), ele->getRhs(), ele, true);
        } else if (ele->getOp() == ASTNode::TOUINT) {
            if (ele->getLhs()->getOp() == ASTNode::BOXINDEX) {
                ThreeAddressNodePtr left = ele->getLhs();
                std::vector<int> leftIdx = extIdxFromTOUINTorBOXINDEX(left);
                int size = transNodeTypeSize(ele->getNodeType());
                for (int j = 0; j < size; ++j) {
                    this->sboxNameMxIndex[ele->getNodeName() + "_$B$_" + std::to_string(j)] = leftIdx[j];
                }
            }
        } else if (ele->getOp() == ASTNode::ADD) {
            std::cout << "WARNING: Modular ADD in sbox function - not supported for BDPT division property." << std::endl;
            assert(false);
        } else if (ele->getOp() == ASTNode::MINUS) {
            if (ele->getLhs()->getNodeType() == NodeType::UINT or ele->getRhs()->getNodeType() == NodeType::UINT) {
                continue;
            }
            if (isConstant(ele->getLhs()) or isConstant(ele->getRhs())) {
                continue;
            }
            std::cout << "WARNING: Modular MINUS in sbox function - not supported for BDPT division property." << std::endl;
            assert(false);
        } else
            assert(false);

        if (!functionCallFlag) {
            if (ele->getLhs()->getNodeType() == NodeType::ARRAY and ele->getOp() == ASTNode::SYMBOLINDEX) {
                std::string finderName = ele->getLhs()->getNodeName() + "_$B$_" + ele->getRhs()->getNodeName();
                for (const auto &pair: this->sboxNameMxIndex) {
                    if (finderName == pair.first)
                        this->sboxNameMxIndex[ele->getNodeName()] = pair.second;
                }
            } else {
                for (const auto &pair: this->sboxNameMxIndex) {
                    if (ele->getLhs() != nullptr)
                        if (ele->getLhs()->getNodeName() == pair.first)
                            this->sboxNameMxIndex[ele->getNodeName()] = pair.second;
                    if (ele->getRhs() != nullptr)
                        if (ele->getRhs()->getNodeName() == pair.first)
                            this->sboxNameMxIndex[ele->getNodeName()] = pair.second;
                }
            }
        } else
            functionCallFlag = false;
    }

    this->sboxRtnIdxSave.clear();
    for (const auto& rtn : procedureH->getReturns()) {
        for (const auto &pair: this->sboxNameMxIndex) {
            if (rtn->getNodeName() == pair.first) {
                this->sboxRtnIdxSave.push_back(pair.second);
            }
        }
    }

    for (int i = 0; i < (int)output.size(); ++i) {
        this->tanNameMxIndex[output[i]->getNodeName()] = this->sboxRtnIdxSave[i];
    }
}


void Div3SetMILP::XORGenModel(const ThreeAddressNodePtr &left, const ThreeAddressNodePtr &right,
                               const ThreeAddressNodePtr &result, bool ifSboxFuncCall) {
    int inputIdx1 = 0, inputIdx2 = 0;
    if (ifSboxFuncCall) {
        for (const auto& pair : this->sboxNameMxIndex) {
            if (pair.first == left->getNodeName()) inputIdx1 = pair.second;
            if (pair.first == right->getNodeName()) inputIdx2 = pair.second;
        }
    } else {
        for (const auto& pair : this->tanNameMxIndex) {
            if (pair.first == left->getNodeName()) inputIdx1 = pair.second;
            if (pair.first == right->getNodeName()) inputIdx2 = pair.second;
        }
    }

    if (ifSboxFuncCall) {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->sboxNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        }
        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->sboxNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->sboxNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        }
    } else {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->tanNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        }
        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->tanNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->tanNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        }
    }

    if (ifSboxFuncCall) {
        if (inputIdx1 == 0) {
            if (!this->sboxRtnIdxSave.empty()) {
                inputIdx1 = this->sboxRtnIdxSave.front();
                this->sboxRtnIdxSave.erase(this->sboxRtnIdxSave.cbegin());
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
            } else {
                inputIdx1 = this->xCounter;
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
                this->xCounter++;
            }
        }
        if (inputIdx2 == 0) {
            if (!this->sboxRtnIdxSave.empty()) {
                inputIdx2 = this->sboxRtnIdxSave.front();
                this->sboxRtnIdxSave.erase(this->sboxRtnIdxSave.cbegin());
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx2;
            } else {
                inputIdx2 = this->xCounter;
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx2;
                this->xCounter++;
            }
        }
    } else {
        if (inputIdx1 == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx1 = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
            } else {
                inputIdx1 = this->xCounter;
                this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
                this->xCounter++;
            }
        }
        if (inputIdx2 == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx2 = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = inputIdx2;
            } else {
                inputIdx2 = this->xCounter;
                this->tanNameMxIndex[left->getNodeName()] = inputIdx2;
                this->xCounter++;
            }
        }
    }

    int outputIdx = this->xCounter;
    if (ifSboxFuncCall) {
        this->sboxNameMxIndex[result->getNodeName()] = outputIdx;
    } else {
        this->tanNameMxIndex[result->getNodeName()] = outputIdx;
    }
    this->xCounter++;

    inputIdx1 = consumeCopy(inputIdx1);
    inputIdx2 = consumeCopy(inputIdx2);

    DivMILPcons::divXorC(this->modelPath, inputIdx1, inputIdx2, outputIdx);
}


void Div3SetMILP::ANDGenModel(const ThreeAddressNodePtr &left, const ThreeAddressNodePtr &right,
                               const ThreeAddressNodePtr &result, bool ifSboxFuncCall) {
    int inputIdx1 = 0, inputIdx2 = 0;
    if (ifSboxFuncCall) {
        for (const auto& pair : this->sboxNameMxIndex) {
            if (pair.first == left->getNodeName()) inputIdx1 = pair.second;
            if (pair.first == right->getNodeName()) inputIdx2 = pair.second;
        }
    } else {
        for (const auto& pair : this->tanNameMxIndex) {
            if (pair.first == left->getNodeName()) inputIdx1 = pair.second;
            if (pair.first == right->getNodeName()) inputIdx2 = pair.second;
        }
    }

    if (ifSboxFuncCall) {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->sboxNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        } else if (left->getOp() == ASTNode::NOT) {
            if (this->sboxNameMxIndex.count(left->getLhs()->getNodeName()) != 0) {
                this->sboxNameMxIndex[left->getNodeName()] = this->sboxNameMxIndex[left->getLhs()->getNodeName()];
                inputIdx1 = this->sboxNameMxIndex[left->getNodeName()];
            } else
                assert(false);
        }
        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->sboxNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->sboxNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        } else if (right->getOp() == ASTNode::NOT) {
            if (this->sboxNameMxIndex.count(right->getLhs()->getNodeName()) != 0) {
                this->sboxNameMxIndex[right->getNodeName()] = this->sboxNameMxIndex[right->getLhs()->getNodeName()];
                inputIdx2 = this->sboxNameMxIndex[right->getNodeName()];
            } else
                assert(false);
        }
    } else {
        if (left->getOp() == ASTNode::SYMBOLINDEX and left->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->tanNameMxIndex) {
                if (pair.first == left->getLhs()->getNodeName() + "_$B$_" + left->getRhs()->getNodeName()) {
                    inputIdx1 = pair.second;
                    this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
                }
            }
        } else if (left->getOp() == ASTNode::NOT) {
            if (this->tanNameMxIndex.count(left->getLhs()->getNodeName()) != 0) {
                this->tanNameMxIndex[left->getNodeName()] = this->tanNameMxIndex[left->getLhs()->getNodeName()];
                inputIdx1 = this->tanNameMxIndex[left->getNodeName()];
            } else
                assert(false);
        }
        if (right->getOp() == ASTNode::SYMBOLINDEX and right->getLhs()->getNodeType() != NodeType::UINT1) {
            for (const auto& pair : this->tanNameMxIndex) {
                if (pair.first == right->getLhs()->getNodeName() + "_$B$_" + right->getRhs()->getNodeName()) {
                    inputIdx2 = pair.second;
                    this->tanNameMxIndex[right->getNodeName()] = inputIdx2;
                }
            }
        } else if (right->getOp() == ASTNode::NOT) {
            if (this->tanNameMxIndex.count(right->getLhs()->getNodeName()) != 0) {
                this->tanNameMxIndex[right->getNodeName()] = this->tanNameMxIndex[right->getLhs()->getNodeName()];
                inputIdx2 = this->tanNameMxIndex[right->getNodeName()];
            } else
                assert(false);
        }
    }

    if (ifSboxFuncCall) {
        if (inputIdx1 == 0) {
            if (!this->sboxRtnIdxSave.empty()) {
                inputIdx1 = this->sboxRtnIdxSave.front();
                this->sboxRtnIdxSave.erase(this->sboxRtnIdxSave.cbegin());
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
            } else {
                inputIdx1 = this->xCounter;
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx1;
                this->xCounter++;
            }
        }
        if (inputIdx2 == 0) {
            if (!this->sboxRtnIdxSave.empty()) {
                inputIdx2 = this->sboxRtnIdxSave.front();
                this->sboxRtnIdxSave.erase(this->sboxRtnIdxSave.cbegin());
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx2;
            } else {
                inputIdx2 = this->xCounter;
                this->sboxNameMxIndex[left->getNodeName()] = inputIdx2;
                this->xCounter++;
            }
        }
    } else {
        if (inputIdx1 == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx1 = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
            } else {
                inputIdx1 = this->xCounter;
                this->tanNameMxIndex[left->getNodeName()] = inputIdx1;
                this->xCounter++;
            }
        }
        if (inputIdx2 == 0) {
            if (!this->rtnIdxSave.empty()) {
                inputIdx2 = this->rtnIdxSave.front();
                this->rtnIdxSave.erase(this->rtnIdxSave.cbegin());
                this->tanNameMxIndex[left->getNodeName()] = inputIdx2;
            } else {
                inputIdx2 = this->xCounter;
                this->tanNameMxIndex[left->getNodeName()] = inputIdx2;
                this->xCounter++;
            }
        }
    }

    int outputIdx = this->xCounter;
    if (ifSboxFuncCall) {
        this->sboxNameMxIndex[result->getNodeName()] = outputIdx;
    } else {
        this->tanNameMxIndex[result->getNodeName()] = outputIdx;
    }
    this->xCounter++;

    inputIdx1 = consumeCopy(inputIdx1);
    inputIdx2 = consumeCopy(inputIdx2);

    DivMILPcons::divAndC(this->modelPath, inputIdx1, inputIdx2, outputIdx);
}


void Div3SetMILP::SboxGenModel(const ThreeAddressNodePtr &sbox, const ThreeAddressNodePtr &input,
                                const ThreeAddressNodePtr &output) {
    std::vector<int> inputIdx = extIdxFromTOUINTorBOXINDEX(input);
    int outputSize = sboxOutputSize[sbox->getNodeName()];
    std::vector<int> outputIdx;
    for (int i = 0; i < outputSize; ++i) {
        outputIdx.push_back(this->xCounter);
        this->tanNameMxIndex[output->getNodeName() + "_$B$_" + std::to_string(i)] = this->xCounter;
        this->xCounter++;
    }

    for (int &idx : inputIdx) idx = consumeCopy(idx);

    // The only functional divergence from the Div2 walker: pick the O_k or O_l
    // inequality set for the current chain mode. CHAIN_K -> sboxDivIneqs (same
    // file Div2 reads), CHAIN_L -> sboxLDivIneqs (the BDPT L-trail inequalities).
    const std::vector<std::vector<int>>& ineqSet =
        (this->chainMode == CHAIN_L) ? this->sboxLDivIneqs[sbox->getNodeName()]
                                     : this->sboxDivIneqs[sbox->getNodeName()];

    DivMILPcons::divSboxC(this->modelPath, inputIdx, outputIdx, ineqSet);
}


void Div3SetMILP::PboxGenModel(const ThreeAddressNodePtr &pbox, const ThreeAddressNodePtr &input,
                                const ThreeAddressNodePtr &output) {
    // P 盒对 K/L 都只是变量重排；每个穿过 P 盒的比特视为一次读取。
    std::vector<int> pboxValue = this->Box[pbox->getNodeName()];
    std::vector<int> inputIdx = extIdxFromTOUINTorBOXINDEX(input);
    for (int &idx : inputIdx) idx = consumeCopy(idx);
    std::vector<int> outputIdx;
    for (int i = 0; i < (int)pboxValue.size(); ++i)
        outputIdx.push_back(0);
    for (int i = 0; i < (int)pboxValue.size(); ++i)
        outputIdx[i] = inputIdx[pboxValue[i]];
    for (int i = 0; i < (int)outputIdx.size(); ++i)
        this->tanNameMxIndex[output->getNodeName() + "_$B$_" + std::to_string(i)] = outputIdx[i];
}
