#include "division/Div3SetMILP.h"
#include "util/setup.h"
#include <chrono>

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
    int a = this->xCounter++;
    int b = this->xCounter++;
    DivMILPcons::divCopyC(this->modelPath, live, std::vector<int>{a, b});
    this->liveChain[live] = b;
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
    std::cout << "\n===== Step 3: 3-subset BDPT MILP Modeling (Phase 2) =====" << std::endl;
    std::cout << "Cipher: " << this->cipherName
              << ", Rounds: " << this->rounds
              << ", Active bits: " << this->activebitsSpec << std::endl;

    preprocess(); // Load both O_k and O_l reduced inequalities.

    std::string milpDir = this->pathPrefix + "milp/";
    (void)system(("mkdir -p " + milpDir).c_str());

    std::string base = milpDir + this->cipherName + "_" + std::to_string(this->rounds)
                       + "_" + this->activebitsSpec + "_subset3";

    // K-chain model: faithful clone of the Div2 model (zero-regression diff
    // anchor). L-chain model: full r-round L propagation (M_L), the base for the
    // Phase 4 decision.
    buildChainModel(CHAIN_K, base + "_K.lp");
    buildChainModel(CHAIN_L, base + "_L.lp");

    std::cout << "\n===== Phase 2 complete: K-chain and L-chain models generated =====" << std::endl;
    std::cout << "  K-chain (diff anchor vs Div2): " << base << "_K.lp" << std::endl;
    std::cout << "  L-chain (M_L):                 " << base << "_L.lp" << std::endl;
    std::cout << "  Key-XOR cross propagation (Phase 3) and per-bit decision /\n"
              << "  counting solver (Phase 4) are not wired yet; see\n"
              << "  doc/three_subset_bdpt_plan.md." << std::endl;
}


void Div3SetMILP::buildChainModel(ChainMode mode, const std::string& modelFile) {
    auto _bench_t0 = std::chrono::steady_clock::now();

    resetState();
    this->chainMode = mode;
    this->modelPath = modelFile;

    // Clear model file.
    std::ofstream clearFile(this->modelPath, std::ios::trunc);
    clearFile.close();

    // Step 1: Generate constraints via TAC traversal (writes to modelPath).
    programGenModel();

    // Step 2: Read generated constraints.
    std::ifstream file(this->modelPath);
    std::string constraints, line;
    while (std::getline(file, line)) { constraints += line + "\n"; }
    file.close();

    // Step 3: Rewrite as a complete .lp file (objective + initial + constraints).
    std::ofstream model(this->modelPath, std::ios::trunc);
    if (!model) {
        std::cout << "ERROR: Cannot write model file: " << this->modelPath << std::endl;
        assert(false);
    }

    model << "Minimize\n";
    for (int i = 0; i < (int)this->outputBitIndices.size(); ++i) {
        model << "x" << this->outputBitIndices[i];
        if (i < (int)this->outputBitIndices.size() - 1)
            model << " + ";
    }
    model << "\n";

    model << "Subject To\n";

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

    const char* modeTag = (mode == CHAIN_L) ? "L" : "K";
    std::cout << modeTag << "-chain MILP model written to: " << this->modelPath << std::endl;
    std::cout << "Variables: x1.." << (this->xCounter - 1);
    if (this->dCounter > 1)
        std::cout << ", d1.." << (this->dCounter - 1);
    std::cout << std::endl;

    auto _bench_t1 = std::chrono::steady_clock::now();
    long long _bench_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t1 - _bench_t0).count();
    std::cerr << "[BENCH] phase=build cipher=" << this->cipherName
              << " subset=3 chain=" << modeTag
              << " rounds=" << this->rounds
              << " activebits=" << this->activebitsSpec
              << " elapsed_ms=" << _bench_ms
              << " n_xvars=" << (this->xCounter - 1)
              << " n_dvars=" << (this->dCounter - 1)
              << " block_size=" << this->blockSize << std::endl;
}


void Div3SetMILP::programGenModel() {
    int roundCounter = this->rounds;
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

    std::string keyId = procedureH->getParameters().at(1).at(0)->getNodeName().substr(
            0, procedureH->getParameters().at(1).at(0)->getNodeName().find("0"));

    if (!this->rtnIdxSave.empty()) {
        for (int i = 0; i < (int)this->rtnIdxSave.size(); ++i)
            this->tanNameMxIndex[procedureH->getParameters().at(2).at(i)->getNodeName()] = this->rtnIdxSave[i];
        this->rtnIdxSave.clear();
        this->rtnMxIndex.clear();
    } else {
        for (const auto & i : procedureH->getParameters().at(2)) {
            this->tanNameMxIndex[i->getNodeName()] = this->xCounter;
            this->xCounter++;
        }
    }

    bool functionCallFlag;
    for (int i = 0; i < (int)procedureH->getBlock().size(); ++i) {
        functionCallFlag = false;
        ThreeAddressNodePtr ele = procedureH->getBlock().at(i);

        if (ele->getOp() == ASTNode::XOR) {
            // Phase 2: Key-XOR is still skipped (same as Div2SetMILP). Phase 3
            // replaces this branch with the L->K cross propagation at the split
            // round t.
            if (ele->getLhs()->getNodeName().find(keyId) != std::string::npos or
                ele->getRhs()->getNodeName().find(keyId) != std::string::npos) {}
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
                continue;
            }
            if (isConstant(ele->getLhs()) or isConstant(ele->getRhs())) {
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
