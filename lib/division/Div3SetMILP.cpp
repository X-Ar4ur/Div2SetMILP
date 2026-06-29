#include "division/Div3SetMILP.h"
#include "util/setup.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>

extern std::map<std::string, std::vector<int>> allBox;
extern std::string cipherName;

// =============================================================================
// 3-subset BDPT MILP walker.
//
// The TAC walker below (programGenModel / roundFunctionGenModel /
// sboxFunctionGenModel / XORGenModel / ANDGenModel / SboxGenModel /
// PboxGenModel / consumeCopy / resolveActiveBitVars) is a deliberate COPY of the
// Div2SetMILP walker (decision 2, doc/three_subset_bdpt_plan.md §3). The only
// functional change vs Div2SetMILP is that SboxGenModel selects the O_k or O_l
// inequality set according to `chainMode`. Everything else is byte-identical so
// the K-chain model is a faithful clone of the Div2 model (Phase 2 diff anchor).
// =============================================================================

Div3SetMILP::Div3SetMILP(std::vector<ProcedureHPtr> procedureHs, int rounds,
                         const std::string& activebitsSpec, const std::string& cipherName)
        : procedureHs(std::move(procedureHs)), rounds(rounds), activebitsSpec(activebitsSpec) {
    this->Box = allBox;
    this->cipherName = cipherName;
    this->pathPrefix = std::string(DPATH) + "division/" + this->cipherName + "/";
}


// Parse activebitsSpec into the list of x-variable indices to initialize to 1.
// Copied verbatim from Div2SetMILP::resolveActiveBitVars(). See that method's
// header comment for the full per-cipher semantics of "<N>" / "R<k>" /
// "L<m>R<k>" / "L<m>" / "hex:<mask>".
std::vector<int> Div3SetMILP::resolveActiveBitVars() const {
    std::vector<int> active;
    if (this->activebitsSpec.empty() || this->blockSize <= 0) return active;

    const std::string& s = this->activebitsSpec;

    // -------- hex:<HH..> explicit bitmask --------
    if (s.size() > 4 && s.substr(0, 4) == "hex:") {
        std::string hex = s.substr(4);
        if (hex.size() > 2 && (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X'))) hex = hex.substr(2);
        std::string clean;
        for (char c : hex) if (c != '_' && c != ' ') clean.push_back(c);
        int bitsNeeded = this->blockSize;
        int hexLen = (bitsNeeded + 3) / 4;
        if ((int)clean.size() != hexLen) {
            std::cout << "ERROR: hex mask '" << clean << "' has " << clean.size()
                      << " nibbles; expected " << hexLen << " for block size " << bitsNeeded << std::endl;
            assert(false);
        }
        for (int nib = 0; nib < hexLen; ++nib) {
            char c = clean[nib];
            int v;
            if (c >= '0' && c <= '9') v = c - '0';
            else if (c >= 'a' && c <= 'f') v = 10 + (c - 'a');
            else if (c >= 'A' && c <= 'F') v = 10 + (c - 'A');
            else { std::cout << "ERROR: non-hex char '" << c << "' in mask" << std::endl; assert(false); }
            for (int b = 0; b < 4; ++b) {
                int xi = this->blockSize - nib * 4 - b;
                if (xi < 1) break;
                if ((v >> (3 - b)) & 1) active.push_back(xi);
            }
        }
        return active;
    }

    bool allDigits = !s.empty();
    for (char c : s) if (!std::isdigit((unsigned char)c)) { allDigits = false; break; }

    // -------- <N> pure integer: dispatch per cipher to match reference Init() --------
    if (allDigits) {
        int n = std::stoi(s);
        if (n < 0 || n > this->blockSize) {
            std::cout << "ERROR: activebits " << n << " out of range [0, " << this->blockSize << "]" << std::endl;
            assert(false);
        }

        if (this->cipherName == "Rectangle") {
            if (this->blockSize != 64) {
                std::cout << "ERROR: Rectangle preset expects block size 64, got " << this->blockSize << std::endl;
                assert(false);
            }
            for (int i = 0; i < n; ++i) {
                int row = (i + 2) % 4;
                int col = 15 - (i / 4);
                int idx = row * 16 + col;
                active.push_back(idx + 1);
            }
            return active;
        }

        if (this->cipherName == "LBlock") {
            int wordLen = this->blockSize / 2;
            if (wordLen != 32) {
                std::cout << "ERROR: LBlock preset expects block size 64, got " << this->blockSize << std::endl;
                assert(false);
            }
            auto halfIdx = [&](int i) -> int {
                return (7 - (i / 4)) * 4 + (i % 4);
            };
            int yActive = std::min(n, 32);
            int xActive = std::max(0, n - 32);
            for (int i = 0; i < yActive; ++i) active.push_back(wordLen + halfIdx(i) + 1);
            for (int i = 0; i < xActive; ++i) active.push_back(halfIdx(i) + 1);
            return active;
        }

        // Default (PRESENT / GIFT / TWINE): top N x-variables active.
        for (int i = 0; i < n; ++i) active.push_back(this->blockSize - i);
        return active;
    }

    // -------- L<m>R<k> Feistel/SIMON spec --------
    int wordLen = this->blockSize / 2;
    int lCount = 0, rCount = 0;
    size_t p = 0;

    if (p < s.size() && s[p] == 'L') {
        ++p;
        size_t start = p;
        while (p < s.size() && std::isdigit((unsigned char)s[p])) ++p;
        if (start == p) { std::cout << "ERROR: malformed L<m> in '" << s << "'" << std::endl; assert(false); }
        lCount = std::stoi(s.substr(start, p - start));
    }
    if (p < s.size() && s[p] == 'R') {
        ++p;
        size_t start = p;
        while (p < s.size() && std::isdigit((unsigned char)s[p])) ++p;
        if (start == p) { std::cout << "ERROR: malformed R<k> in '" << s << "'" << std::endl; assert(false); }
        rCount = std::stoi(s.substr(start, p - start));
    }
    if (p != s.size() || (lCount == 0 && rCount == 0)) {
        std::cout << "ERROR: unrecognized activebits spec '" << s << "'. "
                  << "Use <N>, R<k>, L<m>, L<m>R<k>, or hex:<mask>." << std::endl;
        assert(false);
    }
    if (lCount < 0 || lCount > wordLen || rCount < 0 || rCount > wordLen) {
        std::cout << "ERROR: L/R counts out of range [0, " << wordLen << "] in '" << s << "'" << std::endl;
        assert(false);
    }

    for (int i = 0; i < rCount; ++i) active.push_back(this->blockSize - i);
    for (int i = 0; i < lCount; ++i) active.push_back(wordLen - i);
    return active;
}


// Phase 5: consume one copy of the bit currently represented by rawIdx.
// Copied verbatim from Div2SetMILP::consumeCopy().
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
    const bool may = abstractMayReach(live);
    markAbstractMay(a, may);
    markAbstractMay(b, may);
    return a;
}


void Div3SetMILP::resetState() {
    this->xCounter = 1;
    this->dCounter = 1;
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
    this->localTransitions.clear();
    this->abstractMay.clear();
    this->abstractCandidateCoords.clear();
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
    loadBdptTrailOracles();

    auto _bench_t1 = std::chrono::steady_clock::now();
    long long _bench_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t1 - _bench_t0).count();
    std::cerr << "[BENCH] phase=preprocess cipher=" << this->cipherName
              << " subset=3"
              << " elapsed_ms=" << _bench_ms
              << " n_ineq_loaded=" << _bench_total_ineqs
              << " n_l_ineq_loaded=" << _bench_total_l_ineqs << std::endl;
}


void Div3SetMILP::loadBdptTrailOracles() {
    this->sboxTrailOracles.clear();
    for (const auto& item : this->sboxInputSize) {
        const std::string& sboxName = item.first;
        const std::string kTrailFile = this->pathPrefix + sboxName + "_DivisionTrails.txt";
        const std::string lTrailFile = this->pathPrefix + sboxName + "_L_DivisionTrails.txt";
        BdptTrailOracle oracle;
        std::string error;
        if (!oracle.load(this->sboxInputSize[sboxName],
                         this->sboxOutputSize[sboxName],
                         kTrailFile, lTrailFile, error)) {
            std::cout << "ERROR: Cannot load BDPT trail oracle for "
                      << sboxName << ": " << error << std::endl;
            assert(false);
        }
        this->sboxTrailOracles[sboxName] = oracle;
    }
    std::cerr << "[BENCH] phase=bdpt_oracle cipher=" << this->cipherName
              << " n_oracles=" << this->sboxTrailOracles.size() << std::endl;
}


bool Div3SetMILP::abstractMayReach(int idx) const {
    if (idx <= 0) return false;
    auto it = this->abstractMay.find(idx);
    return it == this->abstractMay.end() ? true : it->second;
}


void Div3SetMILP::markAbstractMay(int idx, bool mayReach) {
    if (idx > 0) this->abstractMay[idx] = mayReach;
}


bool Div3SetMILP::validateOracleTransitions(
        GRBModel& model,
        BdptLocalTransition& failed) const {
    for (const BdptLocalTransition& tr : this->localTransitions) {
        auto oracleIt = this->sboxTrailOracles.find(tr.sboxName);
        if (oracleIt == this->sboxTrailOracles.end()) continue;

        int inputMask = 0;
        for (int i = 0; i < (int)tr.inputVars.size(); ++i) {
            GRBVar v = model.getVarByName("x" + std::to_string(tr.inputVars[i]));
            if (v.get(GRB_DoubleAttr_X) > 0.5) inputMask |= (1 << i);
        }
        int outputMask = 0;
        for (int i = 0; i < (int)tr.outputVars.size(); ++i) {
            GRBVar v = model.getVarByName("x" + std::to_string(tr.outputVars[i]));
            if (v.get(GRB_DoubleAttr_X) > 0.5) outputMask |= (1 << i);
        }

        const BdptTrailKind kind =
            (tr.mode == CHAIN_L) ? BdptTrailKind::L : BdptTrailKind::K;
        if (!oracleIt->second.allows(kind, inputMask, outputMask)) {
            failed = tr;
            return false;
        }
    }
    return true;
}


void Div3SetMILP::addBdptOracleCut(
        GRBModel& model,
        const BdptLocalTransition& failed) {
    GRBLinExpr cut = 0;
    for (int idx : failed.inputVars) {
        GRBVar v = model.getVarByName("x" + std::to_string(idx));
        if (v.get(GRB_DoubleAttr_X) > 0.5) cut += 1 - v;
        else cut += v;
    }
    for (int idx : failed.outputVars) {
        GRBVar v = model.getVarByName("x" + std::to_string(idx));
        if (v.get(GRB_DoubleAttr_X) > 0.5) cut += 1 - v;
        else cut += v;
    }
    model.addConstr(cut >= 1,
                    "bdpt_oracle_cut_" + std::to_string(this->oracleCutCounter++));
    model.update();
}


void Div3SetMILP::MGR() {
    std::cout << "\n===== Step 3: 3-subset BDPT MILP Modeling =====" << std::endl;
    std::cout << "Cipher: " << this->cipherName
              << ", Rounds: " << this->rounds
              << ", Active bits: " << this->activebitsSpec
              << ", Strategy: auto-bdpt-oracle-cegar"
              << ", Output: NBB-only" << std::endl;

    preprocess(); // Load both O_k and O_l reduced inequalities.

    this->runDir = this->pathPrefix + "milp/";
    (void)system(("mkdir -p " + this->runDir).c_str());
    this->resultsPath = this->runDir + "result_" + std::to_string(this->rounds)
                        + "_" + this->activebitsSpec + "_subset3.txt";

    // Run Algorithm 4: build + solve the model set P = {M_1, ..., M_{r-1}} and
    // report the balanced output coordinates.
    searchDistinguisher();

    std::cout << "\n===== 3-subset BDPT distinguisher search complete =====" << std::endl;
}


BdptSolveResult Div3SetMILP::solveMtReachableCoords(
        const std::string& lpFile,
        const std::vector<int>& outIdx,
        const std::set<int>& skip) {
    // DIAGNOSTIC SWITCH (default behaviour unchanged). The default keeps the
    // free-output minimize-and-pin bound proof (hybrid -> min-pin). Setting the
    // environment variable BDPT_SOLVER=perbit instead uses the paper's
    // Stopping Rule 2: pin the whole r-th round output to e_q and test
    // feasibility per coordinate (only the post-M_1 candidate frontier, via the
    // shared `skip` set). This isolates whether the slow terminal "prove
    // objective >= 2" step on long L-chains is a solving-strategy artifact
    // (per-bit finishes fast) or an intrinsic model looseness/size problem
    // (per-bit also stalls). Remove once the strategy question is settled.
    const char* strat = std::getenv("BDPT_SOLVER");
    if (strat != nullptr && std::string(strat) == "perbit") {
        return solveMtReachableCoordsPerBit(lpFile, outIdx, skip);
    }
    return solveMtReachableCoordsHybrid(lpFile, outIdx, skip);
}


BdptSolveResult Div3SetMILP::solveMtReachableCoordsPerBit(
        const std::string& lpFile,
        const std::vector<int>& outIdx,
        const std::set<int>& skip) {
    BdptSolveResult result;

    GRBEnv env = GRBEnv(true);
    env.set(GRB_IntParam_Threads, this->gurobiThreads);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.start();
    GRBModel model = GRBModel(env, lpFile);
    model.set(GRB_DoubleParam_TimeLimit, this->gurobiTimer);
    model.set(GRB_DoubleParam_NodefileStart, 0.5);
    model.set(GRB_IntParam_SolutionLimit, 1);
    result.strategy = "per-bit";

    std::vector<GRBVar> outVars(outIdx.size());
    for (int j = 0; j < (int)outIdx.size(); ++j) {
        outVars[j] = model.getVarByName("x" + std::to_string(outIdx[j]));
    }

    for (int q = 0; q < (int)outVars.size(); ++q) {
        if (skip.count(q)) continue;

        for (int j = 0; j < (int)outVars.size(); ++j) {
            const double fixed = (j == q) ? 1.0 : 0.0;
            outVars[j].set(GRB_DoubleAttr_LB, fixed);
            outVars[j].set(GRB_DoubleAttr_UB, fixed);
        }
        model.update();

        auto solveStart = std::chrono::steady_clock::now();
        model.optimize();
        auto solveEnd = std::chrono::steady_clock::now();
        result.solveCount++;
        result.solverSeconds += std::chrono::duration<double>(
            solveEnd - solveStart).count();

        int status = model.get(GRB_IntAttr_Status);
        result.lastStatus = status;
        result.coordinateStatus[q] = status;
        int solCount = model.get(GRB_IntAttr_SolCount);

        if (status == GRB_INFEASIBLE) {
            continue;
        }
        if (solCount > 0) {
            // One incumbent is a constructive proof that e_q is reachable; an
            // optimality proof is unnecessary for this feasibility question.
            result.reachable.insert(q);
            continue;
        }

        result.markIncomplete(
            status, "unsettled-per-bit-feasibility-at-coordinate-" +
                    std::to_string(q));
        break;
    }

    return result;
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
    // Paper Algorithm 4 / Stopping Rule 2 via ITERATIVE MINIMIZE-AND-PIN (the
    // reference repo's SolveModel technique, algorithm3_4__Cross_propagation.py).
    //
    // We must find every output coordinate q for which the unit vector e_q is a
    // reachable K_r*. The old code fixed K_r* = e_q for each q and ran a separate
    // feasibility/INFEASIBILITY check -- up to n solves per model, and the (~n/2)
    // UNREACHABLE coordinates each force Gurobi to PROVE infeasibility on the
    // loose L-chain+K-chain model, which is the slow part of MILP. For PRESENT 9r
    // this hangs on the middle cross rounds (see data/division/PRESENT/run_9_63*).
    //
    // Instead we minimize the K_r* weight (the .lp objective, paper Alg 3 line 4):
    //   * an optimum of 1 is a reachable unit vector e_j  -> j unknown; pin it to
    //     0 (UB=0) and re-solve to find the next one;
    //   * an optimum >= 2 (or INFEASIBLE) PROVES no remaining free coordinate is
    //     reachable as a unit vector -> the loop stops.
    // That is ONE bound proof per model instead of one infeasibility proof per
    // coordinate -- the dominant speed-up. BestObjStop=1 lets a weight-1 incumbent
    // terminate the solve immediately (it is always optimal since |K_r*| >= 1 on
    // any feasible trail), so only the terminal "optimum >= 2" solve pays a full
    // proof.
    model.set(GRB_DoubleParam_BestObjStop, 1.0);

    std::vector<GRBVar> outVars(outIdx.size());
    for (int j = 0; j < (int)outIdx.size(); ++j) {
        outVars[j] = model.getVarByName("x" + std::to_string(outIdx[j]));
    }

    // Coordinates already known unknown from an earlier M_t need not be found
    // again (only the union matters). Pinning them to 0 also tightens the
    // minimize without excluding any e_j for a still-free j (e_j has every other
    // output bit, including these, at 0 anyway).
    for (int q : skip) {
        if (q >= 0 && q < (int)outVars.size()) outVars[q].set(GRB_DoubleAttr_UB, 0.0);
    }
    model.update();

    while (true) {
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

        // A weight-1 incumbent is provably optimal (no feasible trail has K_r*
        // weight 0), so consume it regardless of solve status (OPTIMAL,
        // USER_OBJ_LIMIT via BestObjStop, or even TIME_LIMIT) and keep going.
        if (solCount > 0 && objR == 1) {
            BdptLocalTransition failedTransition;
            result.oracleChecks++;
            if (!validateOracleTransitions(model, failedTransition)) {
                addBdptOracleCut(model, failedTransition);
                result.oracleCuts++;
                continue;
            }

            int setIdx = -1;
            for (int j = 0; j < (int)outVars.size(); ++j) {
                if (outVars[j].get(GRB_DoubleAttr_X) > 0.5) { setIdx = j; break; }
            }
            if (setIdx < 0) {
                result.markIncomplete(status, "weight-one-solution-without-set-output");
                break;
            }
            result.reachable.insert(setIdx);  // e_setIdx reachable -> coord unknown
            outVars[setIdx].set(GRB_DoubleAttr_UB, 0.0);  // pin and look for the next
            model.update();
            continue;
        }

        if (status == GRB_OPTIMAL || status == GRB_INFEASIBLE) {
            // Proven optimum >= 2 (or no feasible / all-zero output): no remaining
            // free coordinate is reachable as a unit vector -> they are determined.
            break;
        }

        // Unsettled (TIME_LIMIT / other) with no terminal proof.
        std::cout << "WARNING: Gurobi status " << status << " (no proof) for "
                  << lpFile << "; model result is incomplete." << std::endl;
        result.markIncomplete(status, "unsettled-min-pin-terminal-proof");
        break;
    }
    return result;
}


BdptSolveResult Div3SetMILP::solveMtReachableCoordsHybrid(
        const std::string& lpFile,
        const std::vector<int>& outIdx,
        const std::set<int>& skip) {
    // The production default follows the paper/reference SolveModel strategy:
    // keep one minimize-and-pin model alive, pin every reachable unit output,
    // and pay one terminal proof instead of many per-coordinate infeasibility
    // proofs. Falling back to per-bit when many candidates remain recreates the
    // slow path seen on PRESENT Mt4.
    BdptSolveResult result = solveMtReachableCoordsMinPin(lpFile, outIdx, skip);
    result.strategy = "hybrid-min-pin";
    return result;
}


// Algorithm 4 second half (Stopping Rule 2 / lines 14-20): decide the parity of
// the q-th output bit by COUNTING the r-round pure-L trails of M_L that end at
// the unit vector ell^r = e_q. Distinct binary solutions of the MILP are in
// bijection with division trails for L, so Gurobi's solution pool gives the
// trail count; only its parity matters (odd => sum 1, even => sum 0). Returns
// a negative code when the count cannot be trusted (see header), so the caller
// keeps such a bit as 'b' instead of mislabelling it (soundness over recall).
int Div3SetMILP::classifyMLParity(const std::string& mlLpFile,
                                  const std::vector<int>& mlOutIdx,
                                  int coord, long long& solCount) {
    solCount = 0;

    GRBEnv env = GRBEnv(true);
    env.set(GRB_IntParam_Threads, this->gurobiThreads);
    env.set(GRB_IntParam_OutputFlag, 0);
    env.start();
    GRBModel model = GRBModel(env, mlLpFile);

    // Sanity check: the trail/solution bijection breaks if any binary variable
    // appears in no constraint -- such a variable is free to take both values,
    // doubling every count and silently forcing every parity to "even". Refuse
    // to label anything from this model rather than report wrong signs.
    int numVars = model.get(GRB_IntAttr_NumVars);
    for (int i = 0; i < numVars; ++i) {
        GRBVar v = model.getVar(i);
        if (model.getCol(v).size() == 0) {
            std::cout << "ERROR: M_L sanity check failed: variable "
                      << v.get(GRB_StringAttr_VarName)
                      << " appears in no constraint; parity counts cannot be trusted."
                      << std::endl;
            return -3;
        }
    }

    // Paper Algorithm 4 assumes an exact solution count for M_L. Use the same
    // user-configurable budget as every other solve (`timer N`); if the
    // enumeration cannot finish inside it, the bit keeps the sound 'b'
    // fallback instead of getting a wrong label.
    model.set(GRB_DoubleParam_TimeLimit, (double)this->gurobiTimer);

    // The .lp objective is kept as written (Minimize sum ell_i^r, paper
    // Algorithm 4 line 2). With the whole output fixed to e_q it is the
    // constant 1 on the feasible region, and PoolGap = infinity below makes
    // the pool enumerate every feasible solution regardless of objective.

    // Fix ell^r = e_q: outIdx[coord] = 1, every other output coordinate = 0.
    for (int j = 0; j < (int)mlOutIdx.size(); ++j) {
        GRBVar v = model.getVarByName("x" + std::to_string(mlOutIdx[j]));
        double b = (j == coord) ? 1.0 : 0.0;
        v.set(GRB_DoubleAttr_LB, b);
        v.set(GRB_DoubleAttr_UB, b);
    }

    // Systematically enumerate all feasible 0-1 solutions (each = one L-trail).
    // The cap is large enough for the exact parity in normal cases but bounds
    // memory/time; if the count reaches it, the true parity is unknown.
    const int POOL_CAP = 2000000;
    model.set(GRB_IntParam_PoolSearchMode, 2);
    model.set(GRB_IntParam_PoolSolutions, POOL_CAP);
    model.set(GRB_DoubleParam_PoolGap, GRB_INFINITY);
    model.optimize();

    int status = model.get(GRB_IntAttr_Status);
    if (status == GRB_INFEASIBLE) {
        solCount = 0;       // no L-trail reaches e_q => even => sum 0 (balanced)
        return 0;
    }
    if (status != GRB_OPTIMAL) {
        // TIME_LIMIT or other: the enumeration is incomplete -> parity unknown.
        solCount = model.get(GRB_IntAttr_SolCount);
        return -1;
    }
    solCount = model.get(GRB_IntAttr_SolCount);
    if (solCount >= POOL_CAP) return -2;     // capped: true count (parity) unknown
    return (int)(solCount & 1LL);
}


void Div3SetMILP::searchDistinguisher() {
    auto _bench_t0 = std::chrono::steady_clock::now();

    std::string base = this->runDir + this->cipherName + "_" + std::to_string(this->rounds)
                       + "_" + this->activebitsSpec + "_subset3";

    // Union of unknown output coordinates across the model set P. Algorithm 3
    // constructs M_t after t rounds of f_e, so the selected Key-XOR layers must
    // follow the IR semantics. For ARK-at-input SPNs (PRESENT/Rectangle) this
    // skips the initial whitening-like layer; for ARK-at-output rounds it drops
    // the final Key-XOR as in Remark 2.
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

    std::set<int> unknownCoords;
    int nModels = (int)scheduledCrossLayers.size();
    int modelsAttempted = 0;
    bool modelSetComplete = true;
    std::string incompleteReason;

    std::ofstream result(this->resultsPath, std::ios::trunc);
    result << "===== 3-subset BDPT integral distinguisher search =====\n";
    result << "Cipher: " << this->cipherName << ", Rounds: " << this->rounds
           << ", Active bits: " << this->activebitsSpec << "\n";
    result << "Strategy: auto-bdpt-oracle-cegar, Output: NBB-only\n";
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
        for (int q = 0; q < (int)outIdx.size(); ++q) {
            if (!this->abstractCandidateCoords.count(q)) solverSkip.insert(q);
        }
        const int skippedBefore = (int)unknownCoords.size();
        const int abstractSkipped = (int)outIdx.size() - (int)this->abstractCandidateCoords.size();
        int remainingCandidates = 0;
        for (int q : this->abstractCandidateCoords) {
            if (!unknownCoords.count(q)) remainingCandidates++;
        }

        // Pass the running unknown set as skip: a coordinate already shown
        // unknown by an earlier M_t need not be re-found here (the union is all
        // that matters); pinning it to 0 also tightens this model's minimize.
        BdptSolveResult solveResult =
            solveMtReachableCoords(lpFile, outIdx, solverSkip);
        std::set<int> reach = solveResult.reachable;
        if (!solveResult.complete) {
            modelSetComplete = false;
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
          << " oracle_checks=" << solveResult.oracleChecks
          << " oracle_cuts=" << solveResult.oracleCuts
          << " skipped_unknowns=" << skippedBefore
          << " abstract_candidates=" << this->abstractCandidateCoords.size()
          << " abstract_skipped=" << abstractSkipped
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
                  << " oracle_checks=" << solveResult.oracleChecks
                  << " oracle_cuts=" << solveResult.oracleCuts
                  << " status=" << solveResult.lastStatus << std::endl;

    }

    // ---- Algorithm 4, second half (Stopping Rule 2). Every DETERMINED output
    // coordinate (not reachable as a K_r* unit vector in any M_t) is a BALANCED
    // bit. EasyBC's engineering backend reports NBB by default: balanced means
    // no reachable K_r* unit coordinate was found in any certified M_t.
    std::vector<int> balanced;       // = determined: all balanced bits (NBB)
    for (int j = 0; j < this->blockSize; ++j) {
        if (!unknownCoords.count(j)) balanced.push_back(j);
    }

    auto writeCoordSet = [](std::ofstream& os, const char* label,
                            const std::vector<int>& v) {
        os << label << " (" << v.size() << "): {";
        bool first = true;
        for (int j : v) { os << (first ? "" : ",") << j; first = false; }
        os << "}\n";
    };

    std::ofstream r(this->resultsPath, std::ios::app);
    {
        r << "\nUnknown coords (" << unknownCoords.size() << "): {";
        bool f1 = true;
        for (int j : unknownCoords) { r << (f1 ? "" : ",") << j; f1 = false; }
        r << "}\n";
    }
    writeCoordSet(r, "Balanced bits (NBB)", balanced);
    r << "\n";
    r << "Model-set completeness: "
      << (modelSetComplete ? "COMPLETE" : "INCOMPLETE") << "\n";
    if (!modelSetComplete) {
        r << "Incomplete reason: " << incompleteReason << "\n";
        r << "Conservative fallback applied: unsettled coordinates were kept unknown\n";
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
              << this->blockSize << ")" << std::endl;
    if (!modelSetComplete)
        std::cout << "*** Model set incomplete; conservative unknown fallback applied: "
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
              << " model_set_complete=" << (modelSetComplete ? 1 : 0)
              << " strategy=auto-bdpt-oracle-cegar"
              << " n_balanced=" << balanced.size()
              << " n_unknown=" << unknownCoords.size()
              << " block_size=" << this->blockSize
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

    // Objective: Minimize sum of final-round output bits (K_r* for M_t and the
    // K-chain anchor; L_r for the M_L L-chain).
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
    for (int i = 1; i < this->dCounter; ++i) {
        binary << "d" << i << "\n";
    }
    binary << "End";
    binary.close();

    std::cout << modeTag << " MILP model written to: " << this->modelPath
              << "  (x1.." << (this->xCounter - 1) << ")" << std::endl;
}


void Div3SetMILP::buildChainModel(ChainMode mode, const std::string& modelFile) {
    auto _bench_t0 = std::chrono::steady_clock::now();

    resetState();
    this->selectedCrossLayer = -1;  // pure single-mode (no cross)
    this->pureMode = mode;
    this->chainMode = mode;
    this->modelPath = modelFile;

    std::ofstream clearFile(this->modelPath, std::ios::trunc);
    clearFile.close();

    programGenModel();

    const char* modeTag = (mode == CHAIN_L) ? "L-chain" : "K-chain";
    writeLpFile(modeTag);

    auto _bench_t1 = std::chrono::steady_clock::now();
    long long _bench_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t1 - _bench_t0).count();
    std::cerr << "[BENCH] phase=build cipher=" << this->cipherName
              << " subset=3 chain=" << (mode == CHAIN_L ? "L" : "K")
              << " rounds=" << this->rounds
              << " activebits=" << this->activebitsSpec
              << " elapsed_ms=" << _bench_ms
              << " n_xvars=" << (this->xCounter - 1)
              << " n_dvars=" << (this->dCounter - 1)
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

                        // Round bookkeeping for Key-XOR cross propagation.
                        // Pure builds use the fixed pureMode. M_t builds start
                        // in L-chain mode and switch to K-chain exactly when
                        // roundFunctionGenModel emits the selected IR Key-XOR
                        // layer.
                        this->currentRound = ++processed;
                        if (this->selectedCrossLayer < 0) {
                            this->chainMode = this->pureMode;
                        }

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
    this->abstractCandidateCoords.clear();
    for (int j = 0; j < (int)this->outputBitIndices.size(); ++j) {
        if (abstractMayReach(this->outputBitIndices[j])) {
            this->abstractCandidateCoords.insert(j);
        }
    }

    // Phase 5: pin dead tails to 0 (copied from Div2SetMILP::programGenModel).
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
        std::vector<int> activeVars = this->resolveActiveBitVars();
        std::set<int> activeSet(activeVars.begin(), activeVars.end());
        for (const auto & i : procedureH->getParameters().at(2)) {
            const int idx = this->xCounter;
            this->tanNameMxIndex[i->getNodeName()] = idx;
            markAbstractMay(idx, activeSet.count(idx) != 0);
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
                    markAbstractMay(kIdx, true);
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

    // Cross mode is explicit: paper uses only Proposition 1's not-all-one plus
    // dominance relations; exact preserves EasyBC's one-flip selector model.
    // Cross layer constraints over the selected IR Key-XOR layer:
    //   (a) ell_0^t + ... + ell_{s-1}^t <= s - 1                (not all-ones)
    //   (weight) Sum(k_i^t*) - Sum(ell_i^t) = 1                 (K_t* = ell ∨ e_j)
    // The weight increment is what keeps K_t* tight (without it, (a)+(b) admit
    // any superset of L_t and every output bit saturates).
    if (this->selectedCrossLayer >= 0 && !this->crossLBits.empty()) {
        BdptMILPcons::bdptCrossExactOneFlipC(
            this->modelPath, this->crossKBits,
            this->crossLBits, this->dCounter);
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
    markAbstractMay(outputIdx, abstractMayReach(inputIdx1) || abstractMayReach(inputIdx2));
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
    markAbstractMay(outputIdx, abstractMayReach(inputIdx1) || abstractMayReach(inputIdx2));
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

    BdptLocalTransition transition;
    transition.sboxName = sbox->getNodeName();
    transition.mode = this->chainMode;
    transition.inputVars = inputIdx;
    transition.outputVars = outputIdx;
    this->localTransitions.push_back(transition);

    int inputMask = 0;
    for (int i = 0; i < (int)inputIdx.size(); ++i) {
        if (abstractMayReach(inputIdx[i])) inputMask |= (1 << i);
    }
    int outputMask = 0;
    auto oracleIt = this->sboxTrailOracles.find(sbox->getNodeName());
    if (oracleIt != this->sboxTrailOracles.end()) {
        const BdptTrailKind kind =
            (this->chainMode == CHAIN_L) ? BdptTrailKind::L : BdptTrailKind::K;
        outputMask = oracleIt->second.possibleOutputs(kind, inputMask);
    } else if (inputMask != 0) {
        outputMask = (1 << outputSize) - 1;
    }
    for (int i = 0; i < (int)outputIdx.size(); ++i) {
        markAbstractMay(outputIdx[i], (outputMask & (1 << i)) != 0);
    }
}


void Div3SetMILP::PboxGenModel(const ThreeAddressNodePtr &pbox, const ThreeAddressNodePtr &input,
                                const ThreeAddressNodePtr &output) {
    // Permutation is pure variable reindexing for both K and L sets (copied from
    // Div2SetMILP::PboxGenModel). Each crossing bit counts as one read (Phase 5
    // copy-on-read).
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
