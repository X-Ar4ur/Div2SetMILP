#include "SboxDivTrails.h"

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
