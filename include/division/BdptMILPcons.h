//
// 3-subset BDPT MILP Constraint Generation Functions
// Algorithm 3: Key-XOR cross propagation (L -> K*)
//
// These writers emit the two constraints of Proposition 1 / Algorithm 3
// (lines 9-10) for the t-th Key-XOR operation, where the round key is XORed
// with the key-covered bits of the state:
//
//   (a) ell_0^t + ell_1^t + ... + ell_{s-1}^t <= s - 1
//       The L-vector entering the Key-XOR is not all-ones over the s
//       key-covered bits (so at least one zero position exists to flip).
//
//   (b) K_t* & L_t = L_t   <=>   k_i^t* >= ell_i^t  for every bit i
//       The K* vector dominates the L vector bitwise (L_t subset of K_t*).
//
// Note: the reference repo's Cross_propagation.py implements a SIMON-specific
// encoding (it pins the key word's K to all-ones at the split); for SPN ciphers
// we implement Proposition 1 (a)+(b) directly as the paper specifies. See
// doc/three_subset_bdpt_plan.md §7 risk 1.
//

#ifndef EASYBC_BDPTMILPCONS_H
#define EASYBC_BDPTMILPCONS_H

#include <fstream>
#include <iostream>
#include <vector>
#include <string>

namespace BdptMILPcons {

    // Constraint (b), one per key-covered bit:
    //   x{kIdx} - x{lIdx} >= 0     (k_i >= ell_i, i.e. ell_i in K* whenever set)
    void bdptCrossDominanceC(std::string path, int lIdx, int kIdx);

    // Constraint (a), once per Key-XOR layer over the key-covered L bits:
    //   x{l0} + x{l1} + ... + x{l_{s-1}} <= s - 1
    // lIndices holds the MILP indices of the s key-covered L bits (ell_i^t).
    void bdptCrossNotAllOneC(std::string path, const std::vector<int>& lIndices);

    // Hamming-weight increment, once per Key-XOR layer:
    //   (x{k0} + ... ) - (x{l0} + ... ) = 1
    // Proposition 1 builds K_t* as ell ∨ e_j for a SINGLE key-covered zero
    // position j, i.e. K_t* has exactly one more set bit than L_t. Constraints
    // (a)+(b) alone only force K_t* ⊇ L_t (any superset), which over-approximates
    // K_r and saturates every output bit; this equality pins the increment to 1.
    // kIndices / lIndices are the per-bit K_t* and L_t MILP indices of the layer
    // (full-block Key-XOR, s = n: every state bit is key-covered).
    void bdptCrossWeightIncrementC(std::string path,
                                   const std::vector<int>& kIndices,
                                   const std::vector<int>& lIndices);

    // Exact selector encoding for Proposition 1:
    // choose exactly one key-covered zero bit of L_t and set that bit in K_t*.
    //
    // For each paired bit:
    //   sum d_i = 1
    //   d_i + l_i <= 1
    //   k_i - l_i - d_i = 0
    //
    // dCounter is the caller-owned binary auxiliary counter. The writer emits
    // d{dCounter}, d{dCounter+1}, ... and advances dCounter past the last one.
    void bdptCrossExactOneFlipC(std::string path,
                                const std::vector<int>& kIndices,
                                const std::vector<int>& lIndices,
                                int& dCounter);

    // Paper Algorithm 3 / Proposition 1 baseline. This deliberately emits only
    // the two relations stated in the paper:
    //   * L is not the all-one vector over the key-covered positions;
    //   * K* dominates L coordinate-wise.
    // It does not introduce selector variables or the stronger weight equation.
    void bdptCrossPaperC(std::string path,
                         const std::vector<int>& kIndices,
                         const std::vector<int>& lIndices);

}

#endif //EASYBC_BDPTMILPCONS_H
