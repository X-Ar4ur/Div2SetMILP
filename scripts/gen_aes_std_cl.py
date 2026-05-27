#!/usr/bin/env python3
"""Generate benchmarks/singleKey/BlockCipher/AES_std.cl — paper-standard AES
over GF(2^8).

Why this script exists: the GF(2^8) multiplication table (`ffm uint8[256][256]`)
has 65536 entries and cannot be hand-written. This script generates the whole
.cl file: S-box / pbox / pboxm / key schedule / enc are taken verbatim from the
prototype AES.cl, while the round-function bodies follow the *verified*
AES_GF4_toy.cl template scaled m=4 -> m=8 (which also fixes AES.cl's two latent
bugs: MixColumns reading byte_m_out instead of byte_p_out, and the output
packing missing the element index i).

Field: GF(2^8) with irreducible polynomial x^8 + x^4 + x^3 + x + 1 = 0x11b
(low 8 bits = 0x1b). Self-consistency probes read by PrimitiveMatrix:
    ffm[2][128]   = 0x1b = 27
    ffm[3][128]   = 0x9b = 155
    ffm[128][2]   = 27

Run from the repo root:
    python scripts/gen_aes_std_cl.py
"""

import os

# AES S-box, verbatim from benchmarks/singleKey/BlockCipher/AES.cl:3
SBOX = [
    99, 124, 119, 123, 242, 107, 111, 197, 48, 1, 103, 43, 254, 215, 171, 118,
    202, 130, 201, 125, 250, 89, 71, 240, 173, 212, 162, 175, 156, 164, 114, 192,
    183, 253, 147, 38, 54, 63, 247, 204, 52, 165, 229, 241, 113, 216, 49, 21,
    4, 199, 35, 195, 24, 150, 5, 154, 7, 18, 128, 226, 235, 39, 178, 117,
    9, 131, 44, 26, 27, 110, 90, 160, 82, 59, 214, 179, 41, 227, 47, 132,
    83, 209, 0, 237, 32, 252, 177, 91, 106, 203, 190, 57, 74, 76, 88, 207,
    208, 239, 170, 251, 67, 77, 51, 133, 69, 249, 2, 127, 80, 60, 159, 168,
    81, 163, 64, 143, 146, 157, 56, 245, 188, 182, 218, 33, 16, 255, 243, 210,
    205, 12, 19, 236, 95, 151, 68, 23, 196, 167, 126, 61, 100, 93, 25, 115,
    96, 129, 79, 220, 34, 42, 144, 136, 70, 238, 184, 20, 222, 94, 11, 219,
    224, 50, 58, 10, 73, 6, 36, 92, 194, 211, 172, 98, 145, 149, 228, 121,
    231, 200, 55, 109, 141, 213, 78, 169, 108, 86, 244, 234, 101, 122, 174, 8,
    186, 120, 37, 46, 28, 166, 180, 198, 232, 221, 116, 31, 75, 189, 139, 138,
    112, 62, 181, 102, 72, 3, 246, 14, 97, 53, 87, 185, 134, 193, 29, 158,
    225, 248, 152, 17, 105, 217, 142, 148, 155, 30, 135, 233, 206, 85, 40, 223,
    140, 161, 137, 13, 191, 230, 66, 104, 65, 153, 45, 15, 176, 84, 187, 22,
]

AES_MODULUS = 0x11b  # x^8 + x^4 + x^3 + x + 1


def gfmul(a, b):
    """Multiply a*b in GF(2^8) modulo 0x11b."""
    p = 0
    for _ in range(8):
        if b & 1:
            p ^= a
        hi = a & 0x80
        a = (a << 1) & 0xFF
        if hi:
            a ^= (AES_MODULUS & 0xFF)
        b >>= 1
    return p


# Round-function bodies: AES_GF4_toy.cl pattern scaled m=4 -> m=8.
ROUND_FUNCTIONS = """\
r_fn uint1[128] round_function1(uint8 r, uint1[128] key, uint1[128] input) {
    uint1[128] rtn;
    for (i from 0 to 127) {
        rtn[i] = input[i] ^ key[i];
    }
    return rtn;
}

r_fn uint1[128] round_function2(uint8 r, uint1[128] key, uint1[128] input) {
    uint8[16] byte_input;
    for (i from 0 to 15) {
        uint1[8] temp = View(input, i*8, i*8+7);
        byte_input[i] = touint(temp[7], temp[6], temp[5], temp[4], temp[3], temp[2], temp[1], temp[0]);
    }

    uint8[16] byte_s_out;
    for (i from 0 to 15) {
        byte_s_out[i] = s<byte_input[i]>;
    }

    uint8[16] byte_p_out = p<byte_s_out>;

    uint8[16] byte_m_out;
    for (i from 0 to 3) {
        uint8[4] byte_t_out = M * View(byte_p_out, i*4, i*4+3);
        for (j from 0 to 3) {
            byte_m_out[i*4+j] = byte_t_out[j];
        }
    }

    uint1[128] m_out;
    for (i from 0 to 15) {
        for (j from 0 to 7) {
            m_out[i*8+j] = byte_m_out[i][7-j];
        }
    }
    uint1[128] rtn;
    for (i from 0 to 127) {
        rtn[i] = m_out[i] ^ key[i];
    }
    return rtn;
}

r_fn uint1[128] round_function3(uint8 r, uint1[128] key, uint1[128] input) {
    uint8[16] byte_input;
    for (i from 0 to 15) {
        uint1[8] temp = View(input, i*8, i*8+7);
        byte_input[i] = touint(temp[7], temp[6], temp[5], temp[4], temp[3], temp[2], temp[1], temp[0]);
    }

    uint8[16] byte_s_out;
    for (i from 0 to 15) {
        byte_s_out[i] = s<byte_input[i]>;
    }

    uint8[16] byte_p_out = p<byte_s_out>;

    uint1[128] m_out;
    for (i from 0 to 15) {
        for (j from 0 to 7) {
            m_out[i*8+j] = byte_p_out[i][7-j];
        }
    }
    uint1[128] rtn;
    for (i from 0 to 127) {
        rtn[i] = m_out[i] ^ key[i];
    }
    return rtn;
}

fn uint1[128] enc(uint1[1792] key, uint1[128] r_plaintext){
    r_plaintext = round_function1(1, View(key, 0, 127), r_plaintext);
    for (i from 2 to 13) {
        r_plaintext = round_function2(i, View(key, (i - 1) * 128, i * 128 - 1), r_plaintext);
    }
    r_plaintext = round_function3(11, View(key, 1664, 1791), r_plaintext);
    return r_plaintext;
}
"""


def build_cl():
    parts = []
    parts.append("@cipher AES_std_128_128\n\n")
    parts.append("sbox uint8[256] s = {" + ", ".join(str(v) for v in SBOX) + "};\n\n")
    parts.append("pbox uint[16] p = {0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11};\n\n")
    parts.append("pboxm uint8[4][4] M = {{2, 3, 1, 1}, {1, 2, 3, 1}, {1, 1, 2, 3}, {3, 1, 1, 2}};\n")

    rows = []
    for a in range(256):
        rows.append("{" + ",".join(str(gfmul(a, b)) for b in range(256)) + "}")
    parts.append("ffm uint8[256][256] M = {\n" + ",\n".join(rows) + "};\n\n")

    parts.append(ROUND_FUNCTIONS)
    return "".join(parts)


def main():
    # Self-checks: the probe cells PrimitiveMatrix relies on.
    assert gfmul(2, 128) == 0x1b, gfmul(2, 128)
    assert gfmul(3, 128) == 0x9b, gfmul(3, 128)
    assert gfmul(128, 2) == 0x1b, gfmul(128, 2)
    # 2 * 2^7 with reduction sanity; also check identity and a known cell.
    assert gfmul(1, 200) == 200
    assert gfmul(0, 255) == 0

    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    out_path = os.path.join(
        repo_root, "benchmarks", "singleKey", "BlockCipher", "AES_std.cl"
    )
    content = build_cl()
    with open(out_path, "w", newline="\n", encoding="utf-8") as f:
        f.write(content)
    print("wrote", out_path, "(%d bytes)" % len(content.encode("utf-8")))


if __name__ == "__main__":
    main()
