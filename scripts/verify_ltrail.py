#!/usr/bin/env python3
"""
Offline verification of the S-box L-trail computation (paper Algorithm 1).

Goal: check whether EasyBC's createLDivisionTrails() is missing the paper's
SizeReduce_l step (Algorithm 1, line 21), which removes an L-output vector ell'
if it dominates some K-output vector u (ell' >= u). The hypothesis is that the
missing reduction leaves over-permissive L-trails (e.g. a weight-3 input thinning
to a weight-1 output), which keeps post-cross K* low-weight and saturates the
BDPT model set on RECTANGLE.

Convention: n-bit S-box, bit b has value 2^b (LSB-first), matching EasyBC's
trail packing trail[b] = (val>>b)&1.
"""
import sys

def anf_table(sbox, n):
    """ANF[u] = set of input monomials a present in pi_u(y) = prod_{i:u_i=1} y_i.
    Computed via the Mobius transform of f_u(x) = pi_u(S(x))."""
    size = 1 << n
    anf = [set() for _ in range(size)]
    for u in range(size):
        # f_u(x) = 1 iff all output bits selected by u are 1
        f = [0] * size
        for x in range(size):
            y = sbox[x]
            val = 1
            for i in range(n):
                if (u >> i) & 1:
                    val &= (y >> i) & 1
            f[x] = val
        # Mobius transform: coef_a = XOR_{x subset of a} f(x)
        coef = f[:]
        for i in range(n):
            for a in range(size):
                if (a >> i) & 1:
                    coef[a] ^= coef[a ^ (1 << i)]
        anf[u] = {a for a in range(size) if coef[a]}
    return anf

def dominates(a, b):
    """a >= b  (b is a submask of a)."""
    return (a & b) == b

def compute_ltrails(sbox, n, apply_sizereduce):
    size = 1 << n
    allones = size - 1
    anf = anf_table(sbox, n)

    # K-output set for input k = all-ones (BDPT initial k = 1^n, kept by
    # Stopping Rule 1 through the whole L-chain):
    #   Kbar = { u : pi_u(y) contains a monomial >= allones } = { u : allones in ANF[u] }
    Kbar_allones = {u for u in range(size) if allones in anf[u]}

    trails = set()
    trails.add((0, 0))  # explicit zero trail
    for l in range(1, size):
        # Lbar(l) = { u : l in ANF[u] and allones not in ANF[u] }  (membership rule)
        Lbar = {u for u in range(1, size)
                if (l in anf[u]) and (allones not in anf[u])}
        if apply_sizereduce:
            # SizeReduce_l: drop u in Lbar if u >= some K-output u' in Kbar
            Lbar = {u for u in Lbar
                    if not any(dominates(u, up) for up in Kbar_allones)}
        for u in Lbar:
            trails.add((l, u))
    trails.add((allones, allones))  # explicit all-ones trail (1 -> 1)
    return trails, Kbar_allones

def fmt(v, n):
    return "".join(str((v >> b) & 1) for b in range(n))  # LSB-first

def main():
    # RECTANGLE S-box (from benchmarks/.../Rectangle.cl)
    sbox = [6,5,12,10,1,14,7,9,11,0,3,13,8,15,4,2]
    n = 4

    before, Kbar = compute_ltrails(sbox, n, apply_sizereduce=False)
    after, _     = compute_ltrails(sbox, n, apply_sizereduce=True)

    print(f"RECTANGLE S-box, n={n}")
    print(f"K-output set for k=all-ones (Kbar): {sorted(fmt(u,n) for u in Kbar)}")
    print(f"L-trails WITHOUT SizeReduce_l : {len(before)}  (paper Table 3 |L|=80 -> expect 80)")
    print(f"L-trails WITH    SizeReduce_l : {len(after)}")
    removed = sorted(before - after)
    print(f"Removed by SizeReduce_l ({len(removed)}):")
    for (l, u) in removed:
        print(f"   in={fmt(l,n)} -> out={fmt(u,n)}  (|in|={bin(l).count('1')}, |out|={bin(u).count('1')})")

    # Focus: weight-3 inputs (the partial S-box created by one constant bit).
    print("\n--- weight-3 inputs: min output weight before vs after SizeReduce_l ---")
    for l in range(1 << n):
        if bin(l).count("1") != 3:
            continue
        outs_b = [u for (ll, u) in before if ll == l]
        outs_a = [u for (ll, u) in after if ll == l]
        mb = min((bin(u).count("1") for u in outs_b), default=None)
        ma = min((bin(u).count("1") for u in outs_a), default=None)
        print(f"   in={fmt(l,n)}: min|out| before={mb}  after={ma}   "
              f"(outs after: {sorted(fmt(u,n) for u in outs_a)})")

if __name__ == "__main__":
    main()
