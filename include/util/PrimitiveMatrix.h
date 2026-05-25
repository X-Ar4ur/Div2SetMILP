//
// PrimitiveMatrix — utilities for converting a matrix over GF(2^m) into its
// equivalent "primitive" GF(2) matrix at the bit level.
//
// Used by Div2SetMILP (complex linear layer division-property modelling) and
// available for any future module that needs to inflate `pboxm` into a binary
// matrix. See doc/complex_linear_layer_division_plan.md for context.
//
// Bit-ordering convention (must stay aligned with the rest of EasyBC):
//   - LSB-first within each byte. Bit k of a GF(2^m) element a is the
//     coefficient of x^k in its polynomial representation (k = 0..m-1).
//   - This matches Div2SetMILP::SboxGenModel's "Division trails are stored
//     LSB-first" remark and extIdxFromTOUINTorBOXINDEX's left-to-right walk
//     over the BOXINDEX chain (position 0 = LSB).
//

#ifndef EASYBC_PRIMITIVEMATRIX_H
#define EASYBC_PRIMITIVEMATRIX_H

#include <vector>

namespace PrimitiveMatrix {

    using BinaryMatrix = std::vector<std::vector<int>>;

    // Infer the low-m bits of the irreducible polynomial p from a GF(2^m)
    // multiplication table laid out row-major as ffmFlat[a*2^m + b] = a*b in
    // GF(2^m). Uses the identity 2 * 2^(m-1) = 2^m ≡ p_{low m bits} (mod p).
    //
    // Returns the integer whose binary representation gives the low m bits of
    // p (the implicit x^m term is *not* included).
    //
    // Precondition: ffmFlat.size() == (1 << m) * (1 << m). Asserts otherwise.
    int inferModulus(const std::vector<int>& ffmFlat, int m);

    // Spot-check that ffmFlat is consistent with the given modulus: recompute
    // a few products from scratch (using multByMatrix) and compare with the
    // table. Asserts on mismatch so a stale ffm in a `.cl` file fails loudly
    // rather than producing silently wrong constraints.
    void validateFfm(const std::vector<int>& ffmFlat, int m, int modulus);

    // m x m GF(2) matrix T_a that implements left-multiplication by the
    // constant a in GF(2^m): for any b in GF(2^m), the bit vector of a*b
    // equals T_a applied to the bit vector of b (LSB-first).
    //
    // Column j of T_a is the bit vector of a * x^j mod p.
    // Returned matrix is in row-major form: result[r][c] is the coefficient
    // at output bit r contributed by input bit c.
    BinaryMatrix multByMatrix(int a, int m, int modulus);

    // Inflate an s x s GF(2^m) matrix into the equivalent (s*m) x (s*m)
    // GF(2) primitive matrix.
    //
    // gfMatrixFlat: row-major flat storage of the s x s matrix (length s*s).
    //   gfMatrixFlat[i*s + j] is the GF(2^m) coefficient at row i, column j.
    //
    // The returned matrix P is indexed so that
    //   P[i*m + r][j*m + c] = (T_{gfMatrixFlat[i*s+j]})[r][c]
    // i.e. block (i, j) of P is the multiplication matrix of gfMatrix(i, j).
    // Equivalently: if input s bytes are flattened LSB-first into a length-sm
    // GF(2) vector u and output s bytes likewise into v, then v = P * u.
    BinaryMatrix inflate(const std::vector<int>& gfMatrixFlat,
                         int s, int m, int modulus);

}

#endif //EASYBC_PRIMITIVEMATRIX_H
