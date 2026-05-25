#include "util/PrimitiveMatrix.h"

#include <cassert>
#include <iostream>

namespace PrimitiveMatrix {

    // GF(2^m) multiplication by x: a -> a*x mod p
    //   - Left shift by one bit inside the m-bit window
    //   - If the bit that was about to be shifted out (old bit m-1) was 1,
    //     XOR with the low-m-bit representation of p
    static int gfMulByX(int a, int m, int modulus) {
        int mask = (1 << m) - 1;
        int highBit = (a >> (m - 1)) & 1;
        int shifted = (a << 1) & mask;
        if (highBit) shifted ^= (modulus & mask);
        return shifted;
    }

    int inferModulus(const std::vector<int>& ffmFlat, int m) {
        int dim = 1 << m;
        assert((int)ffmFlat.size() == dim * dim &&
               "ffmFlat size mismatch: expected (2^m) * (2^m) entries");
        // ffm[2][2^(m-1)] = 2 * 2^(m-1) = 2^m, which after reduction in GF(2^m)
        // is the low-m-bit pattern of p.
        return ffmFlat[2 * dim + (1 << (m - 1))];
    }

    void validateFfm(const std::vector<int>& ffmFlat, int m, int modulus) {
        int dim = 1 << m;
        assert((int)ffmFlat.size() == dim * dim &&
               "ffmFlat size mismatch in validateFfm");

        // Recompute a few product cells from scratch and compare with the
        // table. We pick (a, b) pairs that exercise modular reduction so a
        // wrong modulus is caught quickly:
        //   (2, 2^(m-1))   = 2^m  -> exercises reduction
        //   (3, 2^(m-1))   = 3 * 2^(m-1)
        //   (2^(m-1), 2)   = same value, transpose check (commutativity)
        std::vector<std::pair<int,int>> probes = {
            {2, 1 << (m - 1)},
            {3, 1 << (m - 1)},
            {1 << (m - 1), 2},
        };
        for (auto pr : probes) {
            int a = pr.first, b = pr.second;
            // Compute a * b via repeated mul-by-x and accumulation.
            int acc = 0;
            int shifted = a;
            for (int k = 0; k < m; ++k) {
                if ((b >> k) & 1) acc ^= shifted;
                shifted = gfMulByX(shifted, m, modulus);
            }
            int expected = ffmFlat[a * dim + b];
            if (acc != expected) {
                std::cerr << "[PrimitiveMatrix::validateFfm] FATAL: ffm table "
                          << "is inconsistent with inferred modulus.\n"
                          << "  m=" << m << ", modulus(low m bits)=0x"
                          << std::hex << modulus << std::dec
                          << ", probe a=" << a << " b=" << b
                          << ", table=" << expected
                          << ", recomputed=" << acc << std::endl;
                assert(false && "ffm/pboxm modulus mismatch");
            }
        }
    }

    BinaryMatrix multByMatrix(int a, int m, int modulus) {
        BinaryMatrix T(m, std::vector<int>(m, 0));
        // Column j is the bit vector of a * x^j mod p.
        int col = a;
        for (int j = 0; j < m; ++j) {
            for (int r = 0; r < m; ++r) {
                T[r][j] = (col >> r) & 1;
            }
            col = gfMulByX(col, m, modulus);
        }
        return T;
    }

    BinaryMatrix inflate(const std::vector<int>& gfMatrixFlat,
                         int s, int m, int modulus) {
        assert((int)gfMatrixFlat.size() == s * s &&
               "gfMatrixFlat size mismatch: expected s*s entries");
        int n = s * m;
        BinaryMatrix P(n, std::vector<int>(n, 0));

        // Cache per-coefficient mult matrices to avoid recomputation when the
        // same coefficient repeats (very common for MDS matrices like AES's
        // {2,3,1,1} rows).
        std::vector<bool> cached(1 << m, false);
        std::vector<BinaryMatrix> cache(1 << m);

        for (int i = 0; i < s; ++i) {
            for (int j = 0; j < s; ++j) {
                int a = gfMatrixFlat[i * s + j];
                if (!cached[a]) {
                    cache[a] = multByMatrix(a, m, modulus);
                    cached[a] = true;
                }
                const BinaryMatrix& Ta = cache[a];
                int rowBase = i * m;
                int colBase = j * m;
                for (int r = 0; r < m; ++r) {
                    for (int c = 0; c < m; ++c) {
                        P[rowBase + r][colBase + c] = Ta[r][c];
                    }
                }
            }
        }

        return P;
    }

}  // namespace PrimitiveMatrix
