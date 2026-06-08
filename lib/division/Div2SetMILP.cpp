#include "division/Div2SetMILP.h"
#include "util/setup.h"
#include <chrono>

extern std::map<std::string, std::vector<int>> allBox;
extern std::string cipherName;


Div2SetMILP::Div2SetMILP(std::vector<ProcedureHPtr> procedureHs, int rounds,
                         const std::string& activebitsSpec, const std::string& cipherName)
        : procedureHs(std::move(procedureHs)), rounds(rounds), activebitsSpec(activebitsSpec) {
    this->Box = allBox;
    this->cipherName = cipherName;
    this->pathPrefix = std::string(DPATH) + "division/" + this->cipherName + "/";
}


// Parse activebitsSpec and return the list of x-variable indices (x1..x{blockSize})
// that should be initialized to 1. Recognized forms:
//   "<N>"         Plain integer. Semantics is cipher-specific so the EasyBC output
//                 matches the reference implementation under MILP_Division_Property-master:
//                   - Rectangle: follows rectangle.py Init(). For i = 0..N-1 activate
//                     variable[(i+2)%4][15 - i/4]; other state bits are 0. The 4×16
//                     state maps to n_input via n_input[r*16 + c] = variable[r][c]
//                     (matching the row-rotation assignment from Rectangle.cl's pbox).
//                   - LBlock: follows lblock.py Init(). For activebits ≤ 32, the right
//                     word (y) gets N bits, left word (x) stays 0; for activebits > 32,
//                     y is full (32) and the remaining (N-32) bits go to x. Within each
//                     half the pattern is variable[7 - i/4][i % 4] — interpreted as
//                     two-nibble packing consistent with LBlock.cl.
//                   - Other ciphers (PRESENT, GIFT, TWINE): top N bits of the block
//                     (x{blockSize}, x{blockSize-1}, ..., x{blockSize-N+1}). This
//                     matches present.py / twine.py / gift.py which activate the highest
//                     N variable indices.
//   "R<k>"        Right-half: top k bits of the right word. Right word occupies the
//                 high half of the block (x{wordLen+1}..x{blockSize}). Used for SIMON
//                 /Simeck activebits <= word_length.
//   "L<m>R<k>"    Left-half top m bits + right-half top k bits. Used for SIMON/Simeck
//                 with activebits > word_length.
//   "L<m>"        Left-half top m bits only.
//   "hex:<HH..>"  Explicit bitmask. Hex is interpreted big-endian over variable indices:
//                 the most-significant nibble covers x{blockSize}..x{blockSize-3}, the
//                 next nibble x{blockSize-4}..x{blockSize-7}, and so on. Bit 1 in the
//                 mask activates that variable. Useful when the cipher-specific preset
//                 doesn't match the desired pattern.
// wordLen is assumed to be blockSize / 2 (matches SIMON/Simeck/Feistel ciphers).
std::vector<int> Div2SetMILP::resolveActiveBitVars() const {
    std::vector<int> active;
    if (this->activebitsSpec.empty() || this->blockSize <= 0) return active;

    const std::string& s = this->activebitsSpec;

    // -------- hex:<HH..> explicit bitmask --------
    if (s.size() > 4 && s.substr(0, 4) == "hex:") {
        std::string hex = s.substr(4);
        // strip optional "0x" / underscores for readability
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
        // Most-significant nibble covers x{blockSize}..x{blockSize-3}.
        for (int nib = 0; nib < hexLen; ++nib) {
            char c = clean[nib];
            int v;
            if (c >= '0' && c <= '9') v = c - '0';
            else if (c >= 'a' && c <= 'f') v = 10 + (c - 'a');
            else if (c >= 'A' && c <= 'F') v = 10 + (c - 'A');
            else { std::cout << "ERROR: non-hex char '" << c << "' in mask" << std::endl; assert(false); }
            // High bit of nibble → first var covered by this nibble.
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

        // Rectangle: reference rectangle.py Init() uses variable[(i+2)%4][15 - i/4].
        // Rectangle.cl assigns rotation-R row r to n_input[r*16 .. r*16+15], matching the
        // reference's row order (row 0 rot 0, row 1 rot 1, row 2 rot 12, row 3 rot 13).
        if (this->cipherName == "Rectangle") {
            if (this->blockSize != 64) {
                std::cout << "ERROR: Rectangle preset expects block size 64, got " << this->blockSize << std::endl;
                assert(false);
            }
            for (int i = 0; i < n; ++i) {
                int row = (i + 2) % 4;
                int col = 15 - (i / 4);
                int idx = row * 16 + col;  // 0-based n_input index
                active.push_back(idx + 1);  // x-variable is 1-indexed
            }
            return active;
        }

        // LBlock: reference lblock.py Init() activates the right half (y) first,
        // then the left half (x). In LBlock.cl, x1..x{wordLen} is the left half
        // consumed by F, and x{wordLen+1}..x{blockSize} is the right half.
        // Within each half the pattern is variable[7 - i/4][i%4], which in .cl bit-layout
        // corresponds to the (i%4)*8 + (7 - i/4) position of the half.
        if (this->cipherName == "LBlock") {
            int wordLen = this->blockSize / 2;
            if (wordLen != 32) {
                std::cout << "ERROR: LBlock preset expects block size 64, got " << this->blockSize << std::endl;
                assert(false);
            }
            auto halfIdx = [&](int i) -> int {
                // variable[7 - i/4][i%4] packs nibble-indexed 8×4 layout.
                // Map back to flat half index: flatOfHalf = (7 - i/4) * 4 + (i % 4).
                return (7 - (i / 4)) * 4 + (i % 4);
            };
            int yActive = std::min(n, 32);
            int xActive = std::max(0, n - 32);
            for (int i = 0; i < yActive; ++i) active.push_back(wordLen + halfIdx(i) + 1); // y/right half
            for (int i = 0; i < xActive; ++i) active.push_back(halfIdx(i) + 1);           // x/left half
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

    // Right-half top k: x{blockSize}, x{blockSize-1}, ..., x{blockSize-k+1}
    for (int i = 0; i < rCount; ++i) active.push_back(this->blockSize - i);
    // Left-half top m: x{wordLen},   x{wordLen-1},   ..., x{wordLen-m+1}
    for (int i = 0; i < lCount; ++i) active.push_back(wordLen - i);
    return active;
}



// Phase 5: consume one copy of the bit currently represented by rawIdx.
// Follows any prior splits via liveChain to reach the current live tail,
// allocates (a, b), emits x_live = x_a + x_b, records liveChain[live] = b,
// and returns a. Subsequent reads of this bit (via the same raw index or
// any alias) will follow liveChain to b and split further.
int Div2SetMILP::consumeCopy(int rawIdx) {
    if (rawIdx <= 0) return rawIdx;
    int live = rawIdx;
    while (this->liveChain.count(live)) live = this->liveChain[live];
    int a = this->xCounter++;
    int b = this->xCounter++;
    DivMILPcons::divCopyC(this->modelPath, live, std::vector<int>{a, b});
    this->liveChain[live] = b;
    return a;
}


void Div2SetMILP::preprocess() {
    auto _bench_t0 = std::chrono::steady_clock::now(); // 记录当前时间点
    int _bench_total_ineqs = 0;
    auto iterator = this->Box.begin();
    while (iterator != this->Box.end()) {
        if (iterator->first.substr(0, 4) == "sbox") {
            std::string sboxName = iterator->first;
            sboxSizeGet(sboxName, iterator->second);

            std::string ineqFile = this->pathPrefix + sboxName + "_Reduce_Inequalities.txt";
            std::ifstream file(ineqFile);
            if (!file) {
                std::cout << "ERROR: Cannot open reduced inequalities file: " << ineqFile << std::endl;
                std::cout << "Reduced inequalities are generated automatically by "
                          << "./EasyBC -div " << this->cipherName << " [rounds] [activebits] "
                          << "before MILP Modeling starts." << std::endl;
                assert(false);
            }

            std::vector<std::vector<int>> ineqs;
            std::string line;
            int expectedSize = sboxInputSize[sboxName] + sboxOutputSize[sboxName] + 1;
            int lineNo = 0;
            while (std::getline(file, line)) {
                lineNo++;
                if (line.empty()) continue;
                std::vector<int> ineq;

                // 将字符串line包装成输入流，依次提取其中的整数。
                std::istringstream iss(line);
                int val;
                while (iss >> val) {
                    ineq.push_back(val);
                }
                if (!ineq.empty()) {
                    if ((int)ineq.size() != expectedSize) {
                        std::cout << "ERROR: Invalid reduced inequality in file: " << ineqFile << std::endl;
                        std::cout << "line: " << lineNo << ", expected " << expectedSize << " coefficients, but got " << ineq.size() << "." << std::endl;
                        assert(false);
                    }
                    ineqs.push_back(ineq);
                }
            }
            file.close();

            this->sboxDivIneqs[sboxName] = ineqs;
            _bench_total_ineqs += (int)ineqs.size();
            std::cout << "Loaded " << ineqs.size() << " reduced inequalities for " << sboxName
                      << " (input=" << sboxInputSize[sboxName] << ", output=" << sboxOutputSize[sboxName] << ")" << std::endl;
        }
        iterator++;
    }

    auto _bench_t1 = std::chrono::steady_clock::now();
    long long _bench_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t1 - _bench_t0).count();
    std::cerr << "[BENCH] phase=preprocess cipher=" << this->cipherName
              << " elapsed_ms=" << _bench_ms
              << " n_ineq_loaded=" << _bench_total_ineqs << std::endl;
}


void Div2SetMILP::MGR() {
    std::cout << "\n===== Step 3: Start Division Property MILP Modeling =====" << std::endl;
    std::cout << "Rounds: " << this->rounds << ", Active bits: " << this->activebitsSpec << std::endl;

    preprocess(); // 加载Sbox的约简不等式

    std::string milpDir = this->pathPrefix + "milp/";
    (void)system(("mkdir -p " + milpDir).c_str());

    this->modelPath = milpDir + this->cipherName + "_" + std::to_string(this->rounds)
                      + "_" + this->activebitsSpec + ".lp";
    this->resultsPath = milpDir + "result_" + std::to_string(this->rounds)
                        + "_" + this->activebitsSpec + ".txt";

    std::ofstream clearFile(this->modelPath, std::ios::trunc);
    clearFile.close();

    buildModel();
    iterativeSolver();

    std::cout << "\n===== Division Property MILP Model Generated =====" << std::endl;
}


void Div2SetMILP::buildModel() {
    auto _bench_t0 = std::chrono::steady_clock::now();
    // 步骤1：通过TAC遍历生成约束（写入modelPath）
    programGenModel();

    // 步骤2：读取生成的约束
    std::ifstream file(this->modelPath);
    std::string constraints, line;
    while (std::getline(file, line)) { constraints += line + "\n"; }
    file.close();

    // 步骤3：重写为完整的 .lp 文件
    std::ofstream model(this->modelPath, std::ios::trunc);
    if (!model) {
        std::cout << "ERROR: Cannot write model file: " << this->modelPath << std::endl;
        assert(false);
    }

    // Objective function: minimize sum of output bits at final round
    model << "Minimize\n";
    for (int i = 0; i < (int)this->outputBitIndices.size(); ++i) {
        model << "x" << this->outputBitIndices[i];
        if (i < (int)this->outputBitIndices.size() - 1)
            model << " + ";
    }
    model << "\n";

    // Subject To
    model << "Subject To\n";

    // Initial division property constraints.
    // Input variables are x1..x{blockSize}. resolveActiveBitVars() translates
    // the activebitsSpec ("60" / "R31" / "L1R32") into the set of variable
    // indices that should be 1; everything else is pinned to 0.
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

    // Round function constraints
    model << constraints;

    model.close();

    // Binary variable declarations
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

    std::cout << "MILP model written to: " << this->modelPath << std::endl;
    std::cout << "Variables: x1.." << (this->xCounter - 1);
    if (this->dCounter > 1)
        std::cout << ", d1.." << (this->dCounter - 1);
    std::cout << std::endl;

    auto _bench_t1 = std::chrono::steady_clock::now();
    long long _bench_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t1 - _bench_t0).count();
    std::cerr << "[BENCH] phase=build cipher=" << this->cipherName
              << " rounds=" << this->rounds
              << " activebits=" << this->activebitsSpec
              << " elapsed_ms=" << _bench_ms
              << " n_xvars=" << (this->xCounter - 1)
              << " n_dvars=" << (this->dCounter - 1)
              << " block_size=" << this->blockSize << std::endl;
}


void Div2SetMILP::programGenModel() {
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

    // Capture output bit indices from the last round's return values
    this->outputBitIndices.clear();
    for (int & i : this->rtnIdxSave) {
        this->outputBitIndices.push_back(i);
    }

    // Phase 5: pin dead tails to 0.
    //
    // A tail is the final endpoint of a liveChain chain: an index that
    // appears as a value but never as a key in liveChain. For each raw
    // bit X that was split k times, we emitted a chain
    //   X = a1 + b1,  b1 = a2 + b2,  ...,  b_{k-1} = a_k + b_k
    // with a1..a_k each consumed by a reader and b_k left as the tail.
    // Summing: X = a1 + a2 + ... + a_k + b_k.
    // If b_k is carried to the final output (present in outputBitIndices)
    // or was carried forward to become next-round input and subsequently
    // split again (already a liveChain key), leave it alone. Otherwise it
    // is a genuinely dead tail — pin it to 0 so the chain telescopes to
    // an exact k-way COPY: X = a1 + ... + a_k.
    std::set<int> chainKeys;
    std::set<int> chainValues;
    for (const auto& kv : this->liveChain) {
        chainKeys.insert(kv.first);
        chainValues.insert(kv.second);
    }
    std::set<int> outSet(this->outputBitIndices.begin(), this->outputBitIndices.end());
    std::ofstream modelApp(this->modelPath, std::ios::app);
    for (int tail : chainValues) {
        if (chainKeys.count(tail)) continue;  // not actually a tail (was split further)
        if (outSet.count(tail)) continue;     // carried into the objective
        modelApp << "x" << tail << " = 0\n";
    }
    modelApp.close();
}

// ProcedureH("round_function"):
//   parameters:
//     at(0) = [r]          — 轮计数器（常量）
//     at(1) = [key0, key1, ..., key31]   — 32 个密钥位
//     at(2) = [input0, input1, ..., input63]  — 64 个明文输入位
//   block: [... 三地址码指令序列 ...]
//   returns: [rtn0, rtn1, ..., rtn63]   — 64 个输出位
void Div2SetMILP::roundFunctionGenModel(const ProcedureHPtr &procedureH) {
    this->constantTan.clear(); // 记录哪些 TAC 变量应当被视作常量
    this->consTanNameMxVal.clear(); // 记录常量值的映射

    // First parameter is the round number (constant)
    this->consTanNameMxVal[procedureH->getParameters().at(0).at(0)->getNodeName()] = this->rndParamR;
    this->constantTan.push_back(procedureH->getParameters().at(0).at(0)->getNodeName());

    // Second parameter is the key (skip key-related operations)
    // 识别第二个参数“key”的名字前缀，后面用于跳过 key 相关操作
    std::string keyId = procedureH->getParameters().at(1).at(0)->getNodeName().substr(
            0, procedureH->getParameters().at(1).at(0)->getNodeName().find("0"));

    // Third parameter is the plaintext input
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

    // Traverse TAC instructions
    bool functionCallFlag;
    for (int i = 0; i < (int)procedureH->getBlock().size(); ++i) {
        functionCallFlag = false;
        ThreeAddressNodePtr ele = procedureH->getBlock().at(i);

        if (ele->getOp() == ASTNode::XOR) {
            // Skip key XOR and constant operations
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
            // Modular addition not supported in bit-based 2-subset division property
            if (ele->getLhs()->getNodeType() == NodeType::UINT or ele->getRhs()->getNodeType() == NodeType::UINT) {
                continue;
            }
            if (isConstant(ele->getLhs()) or isConstant(ele->getRhs())) {
                continue;
            }
            std::cout << "WARNING: Modular ADD/MINUS operation encountered in division property analysis.\n"
                      << "  This is not supported for bit-based 2-subset division property.\n"
                      << "  Use SPN ciphers (PRESENT, GIFT, SKINNY, etc.) for this analysis." << std::endl;
            assert(false);
        } else if (ele->getOp() == ASTNode::PUSH) {  // 不太了解这一部分
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
            // Matrix/linear layer - for division property, handled as XOR chains if needed
            // Skip pboxm operations (not needed for SPN ciphers using pbox)
            ThreeAddressNodePtr left = ele->getLhs();
            if (left->getOp() == ASTNode::FFTIMES) {
                std::cout << "WARNING: Matrix-vector multiplication (pboxm) in division property analysis.\n"
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

        // Variable aliasing for non-function-call operations
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

    // Save return value indices for next round.
    // Phase 5: if the bit was split during this round (via consumeCopy),
    // the raw index stored in tanNameMxIndex is already spent. Follow
    // liveChain to the current live tail — that's the "remaining value"
    // of the bit, to be consumed by the next round's reads.
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


void Div2SetMILP::sboxFunctionGenModel(const ProcedureHPtr &procedureH,
                                        std::vector<ThreeAddressNodePtr> input,
                                        std::vector<ThreeAddressNodePtr> output) {
    this->sboxNameMxIndex.clear();

    int idx = 0;
    for (const auto & i : procedureH->getParameters().at(0)) {
        this->sboxNameMxIndex[i->getNodeName()] = this->tanNameMxIndex[input[idx]->getNodeName()];
        idx++;
    }

    // No A-variable active S-box constraint for division property
    // (unlike DiffSBMILP which adds A-variable constraints here)

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
            std::cout << "WARNING: Modular ADD in sbox function - not supported for division property." << std::endl;
            assert(false);
        } else if (ele->getOp() == ASTNode::MINUS) {
            if (ele->getLhs()->getNodeType() == NodeType::UINT or ele->getRhs()->getNodeType() == NodeType::UINT) {
                continue;
            }
            if (isConstant(ele->getLhs()) or isConstant(ele->getRhs())) {
                continue;
            }
            std::cout << "WARNING: Modular MINUS in sbox function - not supported for division property." << std::endl;
            assert(false);
        } else
            assert(false);

        // Variable aliasing for non-function-call operations
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

    // Map sbox function return values back to caller
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


void Div2SetMILP::XORGenModel(const ThreeAddressNodePtr &left, const ThreeAddressNodePtr &right,
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

    // Handle SYMBOLINDEX references
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

    // Allocate new variables if not found
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

    // Phase 5: split a fresh copy out of each read operand so repeated reads
    // of the same underlying bit end up with independent MILP variables.
    inputIdx1 = consumeCopy(inputIdx1);
    inputIdx2 = consumeCopy(inputIdx2);

    // Division property XOR: x_out - x_in1 - x_in2 = 0
    DivMILPcons::divXorC(this->modelPath, inputIdx1, inputIdx2, outputIdx);
}


void Div2SetMILP::ANDGenModel(const ThreeAddressNodePtr &left, const ThreeAddressNodePtr &right,
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

    // Handle SYMBOLINDEX and NOT references
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

    // Allocate new variables if not found
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

    // Phase 5: split a fresh copy out of each read operand (see XORGenModel).
    inputIdx1 = consumeCopy(inputIdx1);
    inputIdx2 = consumeCopy(inputIdx2);

    // Division property AND: t >= u, t >= v, t <= u + v
    DivMILPcons::divAndC(this->modelPath, inputIdx1, inputIdx2, outputIdx);
}


void Div2SetMILP::SboxGenModel(const ThreeAddressNodePtr &sbox, const ThreeAddressNodePtr &input,
                                const ThreeAddressNodePtr &output) {
    std::vector<int> inputIdx = extIdxFromTOUINTorBOXINDEX(input);
    int outputSize = sboxOutputSize[sbox->getNodeName()];
    std::vector<int> outputIdx;
    for (int i = 0; i < outputSize; ++i) {
        outputIdx.push_back(this->xCounter);
        this->tanNameMxIndex[output->getNodeName() + "_$B$_" + std::to_string(i)] = this->xCounter;
        this->xCounter++;
    }

    // Phase 5: split a fresh copy out of each input bit. S-box inputs
    // are consumed reads and must be distinct MILP variables from any
    // other reader of the same upstream bit.
    for (int &idx : inputIdx) idx = consumeCopy(idx);

    // Apply division trail inequalities.
    // Division trails are stored LSB-first (SboxDivTrails.cpp): coefficient k
    // corresponds to sbox_in[k] / sbox_out[k]. inputIdx/outputIdx returned by
    // extIdxFromTOUINTorBOXINDEX are also indexed so that position k is the
    // variable bound to sbox_in[k]. They align directly — no reversal.
    DivMILPcons::divSboxC(this->modelPath, inputIdx, outputIdx, this->sboxDivIneqs[sbox->getNodeName()]);
}


void Div2SetMILP::PboxGenModel(const ThreeAddressNodePtr &pbox, const ThreeAddressNodePtr &input,
                                const ThreeAddressNodePtr &output) {
    // Permutation is just variable reindexing, but each bit crossing the
    // pbox counts as one READ of its upstream bit. Phase 5: peel off a
    // fresh copy per input bit so SIMON-style repeated pbox uses of the
    // same l_input bit get independent MILP variables.
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


void Div2SetMILP::iterativeSolver() {
    std::cout << "\nStarting iterative MILP solver..." << std::endl;

    time_t startTime = time(NULL);
    clock_t startClock = clock();
    auto _bench_t0 = std::chrono::steady_clock::now();

    GRBEnv env = GRBEnv(true);
    env.set(GRB_IntParam_Threads, this->gurobiThreads);
    env.start();
    GRBModel model = GRBModel(env, this->modelPath);
    model.set(GRB_DoubleParam_TimeLimit, this->gurobiTimer);
    // Memory optimization settings
    model.set(GRB_DoubleParam_NodefileStart, 0.5);
    model.set(GRB_IntParam_MIPFocus, 1);
    model.set(GRB_IntParam_Cuts, 1); // add

    auto _bench_t_loaded = std::chrono::steady_clock::now();
    long long _bench_load_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t_loaded - _bench_t0).count();
    int _bench_n_vars = model.get(GRB_IntAttr_NumVars);
    int _bench_n_cons = model.get(GRB_IntAttr_NumConstrs);
    std::cerr << "[BENCH] phase=model_load cipher=" << this->cipherName
              << " rounds=" << this->rounds
              << " activebits=" << this->activebitsSpec
              << " elapsed_ms=" << _bench_load_ms
              << " n_vars=" << _bench_n_vars
              << " n_cons=" << _bench_n_cons << std::endl;

    int counter = 0;
    std::vector<std::string> setZero;
    bool globalFlag = false;
    int _bench_last_status = -1;
    int _bench_n_iter = 0;

    // Clear result file
    std::ofstream clearResult(this->resultsPath, std::ios::trunc);
    clearResult.close();

    while (counter < this->blockSize) {
        model.optimize();
        int status = model.get(GRB_IntAttr_Status);
        _bench_last_status = status;
        _bench_n_iter++;

        if (status == GRB_OPTIMAL) {
            double objVal = model.get(GRB_DoubleAttr_ObjVal);

            if (objVal > 1) {
                globalFlag = true;
                break;
            } else {
                // Write current iteration result
                std::ofstream result(this->resultsPath, std::ios::app);
                result << "************************************COUNTER = " << counter << "\n";
                result << "The objective value = " << (int)objVal << "\n";

                // Find the objective variable with value 1 and set its upper bound to 0
                int numVars = model.get(GRB_IntAttr_NumVars);
                GRBVar *vars = model.getVars();

                // Output variable values for this iteration
                for (int i = 0; i < (int)this->outputBitIndices.size(); ++i) {
                    std::string varName = "x" + std::to_string(this->outputBitIndices[i]);
                    GRBVar var = model.getVarByName(varName);
                    double val = var.get(GRB_DoubleAttr_X);
                    if (val != 0) {
                        result << varName << "=" << val << "\n";
                    }
                }
                result.close();

                // Find first output variable with value 1 and fix it to 0
                bool found = false;
                for (int i = 0; i < (int)this->outputBitIndices.size(); ++i) {
                    std::string varName = "x" + std::to_string(this->outputBitIndices[i]);
                    GRBVar var = model.getVarByName(varName);
                    double val = var.get(GRB_DoubleAttr_X);
                    if (val == 1) {
                        setZero.push_back(varName);
                        var.set(GRB_DoubleAttr_UB, 0);
                        model.update();
                        counter++;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    // No output variable with value 1 found - should not happen with obj=1
                    globalFlag = true;
                    break;
                }

                delete[] vars;
            }
        } else if (status == GRB_INFEASIBLE) {
            globalFlag = true;
            break;
        } else {
            std::cout << "Gurobi returned unexpected status: " << status << std::endl;
            break;
        }
    }

    time_t endTime = time(NULL);
    clock_t endClock = clock();
    double clockTime = (double)(endClock - startClock) / CLOCKS_PER_SEC;
    double wallTime = difftime(endTime, startTime);

    // Write final results
    std::ofstream result(this->resultsPath, std::ios::app);
    if (globalFlag) {
        result << "\nIntegral Distinguisher Found!\n\n";
        std::cout << "\n*** Integral Distinguisher Found! ***" << std::endl;
    } else {
        result << "\nIntegral Distinguisher does NOT exist\n\n";
        std::cout << "\n*** Integral Distinguisher does NOT exist ***" << std::endl;
    }

    result << "Those are the coordinates set to zero:\n";
    for (const auto& var : setZero) {
        result << var << "\n";
    }
    result << "\n";

    std::vector<std::string> outputVars;
    for (int idx : this->outputBitIndices) {
        outputVars.push_back("x" + std::to_string(idx));
    }
    std::set<std::string> zeroSet(setZero.begin(), setZero.end());
    std::vector<std::string> balancedVars;
    for (const auto& var : outputVars) {
        if (zeroSet.find(var) == zeroSet.end()) {
            balancedVars.push_back(var);
        }
    }
    auto joinVars = [](const std::vector<std::string>& vars) {
        std::ostringstream oss;
        for (int i = 0; i < (int)vars.size(); ++i) {
            if (i > 0) oss << ",";
            oss << vars[i];
        }
        return oss.str();
    };
    result << "Output bits: " << joinVars(outputVars) << "\n";
    result << "Set zero: " << joinVars(setZero) << "\n";
    result << "Balanced bits: " << joinVars(balancedVars) << "\n";
    result << "\n";

    result << "Time used (clock) = " << clockTime << "s\n";
    result << "Time used (wall)  = " << wallTime << "s\n";
    result.close();

    std::cout << "Coordinates set to zero: " << setZero.size() << "/" << this->blockSize << std::endl;
    std::cout << "Time: " << wallTime << "s (wall), " << clockTime << "s (clock)" << std::endl;
    std::cout << "Results saved to: " << this->resultsPath << std::endl;

    auto _bench_t1 = std::chrono::steady_clock::now();
    long long _bench_solve_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t1 - _bench_t_loaded).count();
    long long _bench_total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t1 - _bench_t0).count();
    std::cerr << "[BENCH] phase=solve cipher=" << this->cipherName
              << " rounds=" << this->rounds
              << " activebits=" << this->activebitsSpec
              << " elapsed_ms=" << _bench_solve_ms
              << " total_ms=" << _bench_total_ms
              << " gurobi_status=" << _bench_last_status
              << " distinguisher_found=" << (globalFlag ? 1 : 0)
              << " n_zero_coords=" << setZero.size()
              << " block_size=" << this->blockSize
              << " n_iter=" << _bench_n_iter
              << " threads=" << this->gurobiThreads << std::endl;
}
