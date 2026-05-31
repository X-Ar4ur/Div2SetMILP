#include "SboxDivTrails.h"
#include <chrono>

SboxDivTrails::SboxDivTrails(std::string name, std::vector<int> sbox)
    : name_(std::move(name)), sbox_(std::move(sbox)) {
    // 计算 S-box 位宽 (等价于 Python 中的 SboxSize)
    int len = static_cast<int>(sbox_.size());
    // 验证 sbox 大小是 2 的幂
    assert((len & (len - 1)) == 0 && "S-box size must be a power of 2");
    sboxBitSize_ = static_cast<int>(log2(len));
}

/**
 * 位乘积函数 π_u(x)
 * 若 u 是 x 的子集 (u & x == u)，返回 1；否则返回 0
 */
int SboxDivTrails::bitProduct(int u, int x) {
    return (u & x) == u ? 1 : 0;
}

/**
 * 获取布尔函数 π_u(y) 的真值表，其中 y = sbox(x)
 * 对每个 x，计算 π_u(sbox(x))
 */
std::vector<int> SboxDivTrails::getTruthTable(int u) {
    std::vector<int> table;
    table.reserve(sbox_.size());
    for (int i = 0; i < static_cast<int>(sbox_.size()); ++i) {
        table.push_back(bitProduct(u, sbox_[i]));
    }
    return table;
}

/**
 * 莫比乌斯变换：将真值表转化为代数正规形 (ANF)
 * 对应 algorithm2 中的 ProcessTable
 */
void SboxDivTrails::processTable(std::vector<int>& table) {
    int n = sboxBitSize_;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < (1 << i); ++j) {
            for (int k = 0; k < (1 << (n - 1 - i)); ++k) {
                int idx1 = k + (1 << (n - 1 - i)) + j * (1 << (n - i));
                int idx2 = k + j * (1 << (n - i));
                table[idx1] = table[idx1] ^ table[idx2];
            }
        }
    }
}

/**
 * 计算所有布尔函数的 ANF
 * ANF[i] 包含使 π_i(sbox(x)) 的 ANF 中系数为 1 的所有项
 */
std::vector<std::vector<int>> SboxDivTrails::createANF() {
    int sboxLen = static_cast<int>(sbox_.size());
    std::vector<std::vector<int>> ANF(sboxLen);

    for (int i = 1; i < sboxLen; ++i) {
        std::vector<int> table = getTruthTable(i);
        processTable(table);
        std::vector<int> sqr;
        for (int j = 0; j < sboxLen; ++j) {
            if (table[j] != 0) {
                sqr.push_back(j);
            }
        }
        ANF[i] = sqr;
    }
    return ANF;
}

/**
 * 核心方法：计算 S-box 的所有 Division Trails
 *
 * 对于每对 (输入向量 i, 输出向量 j):
 *   1. 检查 j 是否是 i 的可行 division trail 目标（通过 ANF 判断）
 *   2. 使用冗余消除：若 j1 ⊆ j2 且 j1 已在集合中，则 j2 冗余
 *   3. 将 (i, j) 转为二进制向量格式 [i_bits | j_bits]
 */
void SboxDivTrails::createDivisionTrails() {
    auto _bench_t0 = std::chrono::steady_clock::now();
    std::vector<std::vector<int>> ANF = createANF();
    int sboxLen = static_cast<int>(sbox_.size());
    int n = sboxBitSize_;

    divTrails_.clear();

    // 添加零向量
    std::vector<int> zeroVec(2 * n, 0);
    divTrails_.push_back(zeroVec);

    // 遍历所有非零输入向量
    for (int i = 1; i < sboxLen; ++i) {
        std::vector<int> sqn; // 当前输入 i 对应的最小输出向量集合

        // 遍历所有非零输出向量
        for (int j = 1; j < sboxLen; ++j) {
            bool flag = false;

            // 检查 ANF[j] 中是否存在 entry 使得 i ⊆ entry
            for (int entry : ANF[j]) {
                if ((i | entry) == entry) {
                    flag = true;
                    break;
                }
            }

            if (flag) {
                // 冗余消除
                std::vector<int> sqn1;
                bool flagAdd = true;

                for (int t1 : sqn) {
                    if ((t1 | j) == j) {
                        // t1 ⊆ j，j 是冗余的
                        flagAdd = false;
                        break;
                    } else if ((t1 | j) == t1) {
                        // j ⊆ t1，t1 是冗余的
                        sqn1.push_back(t1);
                    }
                }

                if (flagAdd) {
                    for (int t2 : sqn1) {
                        sqn.erase(std::remove(sqn.begin(), sqn.end(), t2), sqn.end());
                    }
                    sqn.push_back(j);
                }
            }
        }

        // 将 (i, num) 转为二进制向量格式：位 k ↔ sbox_in[k] / sbox_out[k]
        // (LSB-first: trail[k] 存的是位权 2^k 的系数，与 .cl 中 sbox_in[k] 对齐)
        for (int num : sqn) {
            std::vector<int> trail(2 * n, 0);

            for (int bit = 0; bit < n; ++bit) {
                trail[bit] = (i >> bit) & 1;
            }
            for (int bit = 0; bit < n; ++bit) {
                trail[n + bit] = (num >> bit) & 1;
            }

            divTrails_.push_back(trail);
        }
    }

    std::cout << "Division Trails of " << name_ << " computed: "
              << divTrails_.size() << " trails found." << std::endl;

    auto _bench_t1 = std::chrono::steady_clock::now();
    long long _bench_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t1 - _bench_t0).count();
    std::cerr << "[BENCH] phase=trail sbox=" << name_
              << " elapsed_ms=" << _bench_ms
              << " n_trails=" << divTrails_.size() << std::endl;
}

/**
 * 将 Division Trails 输出到文件
 */
void SboxDivTrails::printDivisionTrails(const std::string& filename) {
    if (divTrails_.empty()) {
        createDivisionTrails();
    }

    std::ofstream fileobj(filename);
    fileobj << "Division Trails of sbox:" << std::endl;
    for (const auto& trail : divTrails_) {
        fileobj << "[";
        for (int k = 0; k < static_cast<int>(trail.size()); ++k) {
            fileobj << trail[k];
            if (k < static_cast<int>(trail.size()) - 1) {
                fileobj << ", ";
            }
        }
        fileobj << "]" << std::endl;
    }
    fileobj << std::endl;
    fileobj.close();

    std::cout << "Division Trails written to: " << filename << std::endl;
}

/**
 * 计算 S-box 的所有 L-set Division Trails（3-subset BDPT，Algorithm 1 的 L 部分）
 *
 * 忠实于参考实现 algorithm1/sbox.py：
 *   - 复用 createANF()：ANF[u] = π_u(y) 展开成 x 多项式后的单项式集合（u 为输出掩码）。
 *   - 对 l, u ∈ [1, 2ⁿ)：(l, u) 是合法 L-trail ⟺ π_u(y) 含单项式 π_l(x)（即 l ∈ ANF[u]）
 *     且其单项式集合不含全 1 单项式（2ⁿ-1 ∉ ANF[u]，等价于 S_∩ = ∩_φ(U\downset(φ)) ≠ ∅）。
 *   - 末尾显式追加 (0‖0) 零向量与 (全1‖全1) 全 1 trail（通用规则会自排除全 1）。
 *   - 不做 K-trail 那种 ⪰ up-set 去冗余（L 是精确相等语义）。
 * 经验证：SIMON "S-box" 共 30 条、PRESENT S-box 共 84 条，与论文 Table 3 一致。
 */
void SboxDivTrails::createLDivisionTrails() {
    auto _bench_t0 = std::chrono::steady_clock::now();
    std::vector<std::vector<int>> ANF = createANF();  // ANF[0] 保持为空（createANF 从 1 开始）
    int sboxLen = static_cast<int>(sbox_.size());     // 2ⁿ
    int n = sboxBitSize_;
    int allOnes = sboxLen - 1;                         // 2ⁿ-1：全 1 单项式 / 全 1 向量

    lDivTrails_.clear();

    // 显式追加零向量 (0‖0)
    lDivTrails_.push_back(std::vector<int>(2 * n, 0));

    // 规则生成：l, u 都从 1 遍历（与参考实现一致）
    for (int l = 1; l < sboxLen; ++l) {
        for (int u = 1; u < sboxLen; ++u) {
            // 条件 (a)：l ∈ ANF[u]（π_u(y) 含 π_l(x)）
            bool containsL = false;
            for (int mono : ANF[u]) {
                if (mono == l) { containsL = true; break; }
            }
            if (!containsL) continue;

            // 条件 (b)：全 1 单项式 ∉ ANF[u]（S_∩ ≠ ∅）
            bool hasAllOnes = false;
            for (int mono : ANF[u]) {
                if (mono == allOnes) { hasAllOnes = true; break; }
            }
            if (hasAllOnes) continue;

            // 打包为 LSB-first [in | out]（与 createDivisionTrails 同约定，
            // 位 k ↔ sbox_in[k]/sbox_out[k]，对齐 .cl 与下游 SboxM/约简/Div3 管线）
            std::vector<int> trail(2 * n, 0);
            for (int bit = 0; bit < n; ++bit) trail[bit] = (l >> bit) & 1;
            for (int bit = 0; bit < n; ++bit) trail[n + bit] = (u >> bit) & 1;
            lDivTrails_.push_back(trail);
        }
    }

    // 显式追加全 1 trail (全1‖全1)：通用规则因 π_{全1}(y) 含全 1 单项式而自排除它，
    // 但它是合法 L-trail（1 → 1）。镜像参考实现的末尾追加。
    lDivTrails_.push_back(std::vector<int>(2 * n, 1));

    std::cout << "L-Division Trails of " << name_ << " computed: "
              << lDivTrails_.size() << " trails found." << std::endl;

    auto _bench_t1 = std::chrono::steady_clock::now();
    long long _bench_ms = std::chrono::duration_cast<std::chrono::milliseconds>(_bench_t1 - _bench_t0).count();
    std::cerr << "[BENCH] phase=trail sbox=" << name_ << "_L"
              << " elapsed_ms=" << _bench_ms
              << " n_trails=" << lDivTrails_.size() << std::endl;
}

/**
 * 将 L-set Division Trails 输出到文件
 */
void SboxDivTrails::printLDivisionTrails(const std::string& filename) {
    if (lDivTrails_.empty()) {
        createLDivisionTrails();
    }

    std::ofstream fileobj(filename);
    fileobj << "L-Division Trails of sbox:" << std::endl;
    for (const auto& trail : lDivTrails_) {
        fileobj << "[";
        for (int k = 0; k < static_cast<int>(trail.size()); ++k) {
            fileobj << trail[k];
            if (k < static_cast<int>(trail.size()) - 1) {
                fileobj << ", ";
            }
        }
        fileobj << "]" << std::endl;
    }
    fileobj << std::endl;
    fileobj.close();

    std::cout << "L-Division Trails written to: " << filename << std::endl;
}
