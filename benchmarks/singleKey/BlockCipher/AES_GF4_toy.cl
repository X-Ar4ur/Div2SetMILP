# AES_GF4_toy — 64-bit toy AES over GF(2^4).
#
# Purpose: stand-in for AES.cl until the AES.cl ffm/pboxm dimension mismatch
# (see doc/Linear-Layer_Phase_A_progress.md §3.1) is resolved. This file uses
# m=4 throughout so the pboxm element type and the 16×16 ffm table are
# consistent, which lets Phase A's FfMulGenModel pass its m_in == m_ffm check
# and actually emit disjointed-representation MILP constraints.
#
# Block: 16 nibbles × 4 bits = 64 bits, arranged as a 4×4 nibble state.
# S-box: PRESENT's 4-bit S-box (well-known, 4×4 differential/linear branch).
# Linear layer: ShiftRows (reuses AES's pbox index permutation, applied to
#               nibbles instead of bytes) followed by MixColumns over GF(2^4)
#               with the *same* coefficient matrix {2,3,1,1; ...} as AES.
# Field: GF(2^4) with irreducible polynomial x^4 + x + 1 (matches the ffm
#        table copied from Piccolo.cl / AES.cl, low 4 bits of p = 0011).
# Rounds: 12 total (1 whitening + 10 middle + 1 final, no MixColumns in last).
# Key:    12 × 64 = 768 bits master key, sliced 64 bits per round.

@cipher AES_GF4_toy_64_64

sbox uint4[16] s = {12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2};

pbox uint[16] p = {0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11};

pboxm uint4[4][4] M = {{2, 3, 1, 1}, {1, 2, 3, 1}, {1, 1, 2, 3}, {3, 1, 1, 2}};
ffm uint4[16][16] M = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
                    {0, 2, 4, 6, 8, 10, 12, 14, 3, 1, 7, 5, 11, 9, 15, 13},
                    {0, 3, 6, 5, 12, 15, 10, 9, 11, 8, 13, 14, 7, 4, 1, 2},
                    {0, 4, 8, 12, 3, 7, 11, 15, 6, 2, 14, 10, 5, 1, 13, 9},
                    {0, 5, 10, 15, 7, 2, 13, 8, 14, 11, 4, 1, 9, 12, 3, 6},
                    {0, 6, 12, 10, 11, 13, 7, 1, 5, 3, 9, 15, 14, 8, 2, 4},
                    {0, 6, 12, 10, 11, 13, 7, 1, 5, 3, 9, 15, 14, 8, 2, 4},
                    {0, 8, 3, 11, 6, 14, 5, 13, 12, 4, 15, 7, 10, 2, 9, 1},
                    {0, 9, 1, 8, 2, 11, 3, 10, 4, 13, 5, 12, 6, 15, 7, 14},
                    {0, 10, 7, 13, 14, 4, 9, 3, 15, 5, 8, 2, 1, 11, 6, 12},
                    {0, 11, 5, 14, 10, 1, 15, 4, 7, 12, 2, 9, 13, 6, 8, 3},
                    {0, 12, 11, 7, 5, 9, 14, 2, 10, 6, 1, 13, 15, 3, 4, 8},
                    {0, 13, 9, 4, 1, 12, 8, 5, 2, 15, 11, 6, 3, 14, 10, 7},
                    {0, 14, 15, 1, 13, 3, 2, 12, 9, 7, 6, 8, 4, 10, 11, 5},
                    {0, 15, 13, 2, 9, 6, 4, 11, 1, 14, 12, 3, 8, 7, 5, 10}};

r_fn uint1[64] round_function1(uint8 r, uint1[64] key, uint1[64] input) {
    uint1[64] rtn;
    for (i from 0 to 63) {
        rtn[i] = input[i] ^ key[i];
    }
    return rtn;
}

r_fn uint1[64] round_function2(uint8 r, uint1[64] key, uint1[64] input) {
    uint4[16] nibble_input;
    for (i from 0 to 15) {
        uint1[4] temp = View(input, i*4, i*4+3);
        nibble_input[i] = touint(temp[3], temp[2], temp[1], temp[0]);
    }

    uint4[16] nibble_s_out;
    for (i from 0 to 15) {
        nibble_s_out[i] = s<nibble_input[i]>;
    }

    uint4[16] nibble_p_out = p<nibble_s_out>;

    uint4[16] nibble_m_out;
    for (i from 0 to 3) {
        uint4[4] nibble_t_out = M * View(nibble_p_out, i*4, i*4+3);
        for (j from 0 to 3) {
            nibble_m_out[i*4+j] = nibble_t_out[j];
        }
    }

    uint1[64] m_out;
    for (i from 0 to 15) {
        for (j from 0 to 3) {
            m_out[i*4+j] = nibble_m_out[i][3-j];
        }
    }
    uint1[64] rtn;
    for (i from 0 to 63) {
        rtn[i] = m_out[i] ^ key[i];
    }
    return rtn;
}

r_fn uint1[64] round_function3(uint8 r, uint1[64] key, uint1[64] input) {
    uint4[16] nibble_input;
    for (i from 0 to 15) {
        uint1[4] temp = View(input, i*4, i*4+3);
        nibble_input[i] = touint(temp[3], temp[2], temp[1], temp[0]);
    }

    uint4[16] nibble_s_out;
    for (i from 0 to 15) {
        nibble_s_out[i] = s<nibble_input[i]>;
    }

    uint4[16] nibble_p_out = p<nibble_s_out>;

    uint1[64] m_out;
    for (i from 0 to 15) {
        for (j from 0 to 3) {
            m_out[i*4+j] = nibble_p_out[i][3-j];
        }
    }
    uint1[64] rtn;
    for (i from 0 to 63) {
        rtn[i] = m_out[i] ^ key[i];
    }
    return rtn;
}

fn uint1[64] enc(uint1[768] key, uint1[64] r_plaintext){
    r_plaintext = round_function1(1, View(key, 0, 63), r_plaintext);
    for (i from 2 to 11) {
        r_plaintext = round_function2(i, View(key, (i - 1) * 64, i * 64 - 1), r_plaintext);
    }
    r_plaintext = round_function3(12, View(key, 704, 767), r_plaintext);
    return r_plaintext;
}
