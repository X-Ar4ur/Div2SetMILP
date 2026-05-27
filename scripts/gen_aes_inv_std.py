#!/usr/bin/env python3
"""Generate the inverse-AES benchmark + precomputed S-box inequality cache.

Why: reproducing the paper's AES 4-round result aligns with the authors' own
open-source model (https://github.com/mhgharieb/MILP_DivisionProprerty_LinearLayer,
AES/4-round), which analyses the AES *equivalent inverse cipher* (inverse S-box
S^-1 + InvMixColumns). Their `constants.py::SboxInqs` is the EXACT bit-based
division-property model of S^-1, which lets us bypass the SageMath convex-hull
step that hangs on 8-bit S-boxes.

Convention bridge (verified offline):
  - The repo uses MSB-first within-byte ordering (CreateStateVariable uses
    reversed(range(8))). EasyBC is LSB-first. So each SboxInqs row's input half
    (cols 0..7) and output half (cols 8..15) are *reversed* to convert to
    EasyBC's [in||out] LSB-first form; the constant (col 16) is unchanged.
  - With that reversal, SboxInqs is an EXACT model of S^-1's division trails in
    EasyBC's convention (0 trail violations, 0 impossible points allowed) -- this
    script recomputes S^-1's division trails and asserts it.
  - inflate(InvMixColumns, LSB-first) reproduces the repo's `Matrix` under the
    same MSB-first reversal, so EasyBC's own PrimitiveMatrix::inflate is correct;
    we do NOT import the repo matrix.

Outputs:
  - benchmarks/singleKey/BlockCipher/AES_inv_std.cl
  - benchmarks/precomputed_sbox_ineqs/AES_inv_std__sboxs_Reduce_Inequalities.txt

Run from the repo root (needs network to fetch SboxInqs once):
    python scripts/gen_aes_inv_std.py
"""

import os
import sys
import ast
import urllib.request

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gen_aes_std_cl import SBOX, gfmul, ROUND_FUNCTIONS  # reuse forward S-box, GF(2^8) mul, round bodies

REPO_CONSTANTS = ('https://raw.githubusercontent.com/mhgharieb/'
                  'MILP_DivisionProprerty_LinearLayer/master/AES/4-round/constants.py')

# Forward AES ShiftRows as encoded (and verified to run) in AES_std.cl:
FWD_SHIFTROWS = [0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12, 1, 6, 11]

# AES InvMixColumns coefficients (GF(2^8)): rows of the inverse MDS matrix.
INV_MIXCOLUMNS = [[14, 11, 13, 9],
                  [9, 14, 11, 13],
                  [13, 9, 14, 11],
                  [11, 13, 9, 14]]

N = 8


def inverse_sbox(sbox):
    inv = [0] * 256
    for x in range(256):
        inv[sbox[x]] = x
    return inv


def inverse_perm(p):
    q = [0] * len(p)
    for i, v in enumerate(p):
        q[v] = i
    return q


def division_trails(sbox):
    """Minimal-output division trails of an 8-bit S-box, as (u, v) int pairs.

    Matches EasyBC's SboxDivTrails convention (validated against EasyBC's
    forward AES trails elsewhere)."""
    present = {}
    for v in range(256):
        tt = [0] * 256
        for x in range(256):
            sx = sbox[x]
            val = 1
            for i in range(N):
                if (v >> i) & 1:
                    val &= (sx >> i) & 1
            tt[x] = val
        a = tt[:]
        for i in range(N):
            for x in range(256):
                if (x >> i) & 1:
                    a[x] ^= a[x ^ (1 << i)]
        present[v] = [w for w in range(256) if a[w]]
    trails = set()
    for u in range(256):
        reach = [v for v in range(256) if any((w & u) == u for w in present[v])]
        for v in reach:
            if not any(v2 != v and (v2 & v) == v2 for v2 in reach):
                trails.add((u, v))
    return trails


def fetch_sboxinqs():
    src = urllib.request.urlopen(REPO_CONSTANTS, timeout=60).read().decode()
    for node in ast.parse(src).body:
        if (isinstance(node, ast.Assign) and isinstance(node.targets[0], ast.Name)
                and node.targets[0].id == 'SboxInqs'):
            return ast.literal_eval(node.value)
    raise RuntimeError('SboxInqs not found in repo constants.py')


def to_lsb_first(ineq):
    """Reverse each byte half (MSB-first -> LSB-first); keep the constant."""
    inp = ineq[0:8][::-1]
    out = ineq[8:16][::-1]
    return list(inp) + list(out) + [ineq[16]]


def validate(reduced_ineqs, sinv_trails):
    def sat(point, iq):
        return sum(iq[i] * point[i] for i in range(16)) >= -iq[16]

    def point(u, v):
        return [(u >> i) & 1 for i in range(8)] + [(v >> i) & 1 for i in range(8)]

    trailset = set(sinv_trails)
    bad = sum(1 for (u, v) in sinv_trails
              if not all(sat(point(u, v), iq) for iq in reduced_ineqs))
    allowed = 0
    for u in range(256):
        for v in range(256):
            if (u, v) in trailset:
                continue
            if all(sat(point(u, v), iq) for iq in reduced_ineqs):
                allowed += 1
    return bad, allowed


def build_inv_cl():
    sinv = inverse_sbox(SBOX)
    inv_shiftrows = inverse_perm(FWD_SHIFTROWS)

    parts = []
    parts.append("@cipher AES_inv_std_128_128\n\n")
    parts.append("sbox uint8[256] s = {" + ", ".join(str(v) for v in sinv) + "};\n\n")
    parts.append("pbox uint[16] p = {" + ", ".join(str(v) for v in inv_shiftrows) + "};\n\n")
    pboxm_rows = ", ".join("{" + ", ".join(str(c) for c in row) + "}" for row in INV_MIXCOLUMNS)
    parts.append("pboxm uint8[4][4] M = {" + pboxm_rows + "};\n")
    rows = ["{" + ",".join(str(gfmul(a, b)) for b in range(256)) + "}" for a in range(256)]
    parts.append("ffm uint8[256][256] M = {\n" + ",\n".join(rows) + "};\n\n")
    parts.append(ROUND_FUNCTIONS)
    return "".join(parts)


def main():
    repo_ineqs = fetch_sboxinqs()
    reduced = [to_lsb_first(iq) for iq in repo_ineqs]
    assert all(len(iq) == 17 for iq in reduced)

    sinv = inverse_sbox(SBOX)
    sinv_trails = division_trails(sinv)
    bad, allowed = validate(reduced, sinv_trails)
    print("S^-1 trails:", len(sinv_trails),
          "| reversed SboxInqs: violations=%d allowed=%d" % (bad, allowed))
    assert bad == 0 and allowed == 0, \
        "Reversed SboxInqs is NOT an exact model of S^-1 trails -- abort."

    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    cl_path = os.path.join(repo_root, "benchmarks", "singleKey", "BlockCipher", "AES_inv_std.cl")
    with open(cl_path, "w", newline="\n", encoding="utf-8") as f:
        f.write(build_inv_cl())
    print("wrote", cl_path)

    cache_dir = os.path.join(repo_root, "benchmarks", "precomputed_sbox_ineqs")
    os.makedirs(cache_dir, exist_ok=True)
    cache_path = os.path.join(cache_dir, "AES_inv_std__sboxs_Reduce_Inequalities.txt")
    with open(cache_path, "w", newline="\n", encoding="utf-8") as f:
        for iq in reduced:
            f.write(" ".join(str(c) for c in iq) + "\n")
    print("wrote", cache_path, "(%d inequalities)" % len(reduced))


if __name__ == "__main__":
    main()
