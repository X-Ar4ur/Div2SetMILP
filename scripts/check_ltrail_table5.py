# Replicate EasyBC SboxDivTrails::createLDivisionTrails for PRESENT and compare to paper Table 5.
from collections import defaultdict
sbox = [12,5,6,11,9,0,10,13,3,14,15,8,4,7,1,2]
n = 4; N = 16

def bitprod(u,x): return 1 if (u & x)==u else 0
def truth(u): return [bitprod(u, sbox[x]) for x in range(N)]
def mobius(t):
    t=t[:]
    for i in range(n):
        for j in range(1<<i):
            for k in range(1<<(n-1-i)):
                i1 = k + (1<<(n-1-i)) + j*(1<<(n-i))
                i2 = k + j*(1<<(n-i))
                t[i1]^=t[i2]
    return t
ANF=[[] for _ in range(N)]
for u in range(1,N):
    t=mobius(truth(u))
    ANF[u]=[m for m in range(N) if t[m]]

allones=N-1
pairs=set(); pairs.add((0,0))
for l in range(1,N):
    for u in range(1,N):
        if l in ANF[u] and allones not in ANF[u]:
            pairs.add((l,u))
pairs.add((allones,allones))
easy=defaultdict(set)
for (l,u) in pairs: easy[l].add(u)

def vA(v): return v[0] + 2*v[1] + 4*v[2] + 8*v[3]   # b0=LSB
def vB(v): return v[3] + 2*v[2] + 4*v[1] + 8*v[0]   # b0=MSB

table5 = {
 (0,0,0,0):[[0,0,0,0]],
 (0,0,0,1):[[0,0,0,1],[0,1,0,1],[1,0,0,0],[1,1,0,0]],
 (0,0,1,0):[[0,0,1,0],[0,1,1,0],[1,0,0,0],[1,1,0,0]],
 (0,0,1,1):[[0,0,1,1],[0,1,0,0],[0,1,0,1],[0,1,1,0],[1,0,0,1],[1,0,1,0],[1,0,1,1],[1,1,0,0]],
 (0,1,0,0):[[0,0,0,1],[0,1,0,0],[1,0,0,1],[1,1,0,0]],
 (0,1,0,1):[[0,1,0,1],[1,0,0,1],[1,1,0,0]],
 (0,1,1,0):[[0,0,0,1],[0,1,1,0],[1,0,0,0],[1,0,0,1],[1,0,1,0],[1,1,0,0]],
 (0,1,1,1):[[0,0,1,0],[0,0,1,1],[0,1,1,0],[1,0,0,0],[1,0,0,1],[1,0,1,1],[1,1,0,1]],
 (1,0,0,0):[[0,0,0,1],[0,0,1,0],[0,0,1,1],[0,1,0,0],[1,0,0,0],[1,1,0,0]],
 (1,0,0,1):[[0,0,1,1],[0,1,0,0],[0,1,0,1],[0,1,1,0],[1,0,1,0],[1,1,1,0]],
 (1,0,1,0):[[0,0,1,0],[0,1,0,0],[0,1,0,1],[0,1,1,1],[1,0,0,1],[1,0,1,0],[1,0,1,1],[1,1,0,1],[1,1,1,0]],
 (1,0,1,1):[[0,0,1,0],[0,0,1,1],[0,1,0,0],[0,1,1,0],[0,1,1,1],[1,0,0,0],[1,0,1,0],[1,1,0,0],[1,1,0,1]],
 (1,1,0,0):[[0,0,1,0],[0,0,1,1],[1,0,0,1],[1,1,0,0]],
 (1,1,0,1):[[0,0,1,0],[0,1,0,0],[0,1,1,1],[1,0,0,0],[1,0,0,1],[1,0,1,0],[1,1,1,0]],
 (1,1,1,0):[[0,1,0,1],[0,1,1,1],[1,0,1,1],[1,1,0,1],[1,1,1,0]],
 (1,1,1,1):[[1,1,1,1]],
}

for conv,vf in [("A(b0=LSB)",vA),("B(b0=MSB)",vB)]:
    paper=defaultdict(set)
    for lv,us in table5.items():
        li=vf(list(lv))
        for u in us: paper[li].add(vf(u))
    ok=True; diffs=[]
    for l in sorted(set(easy)|set(paper)):
        if easy[l]!=paper[l]:
            ok=False; diffs.append((l, sorted(easy[l]), sorted(paper[l])))
    teasy=sum(len(v) for v in easy.values()); tpaper=sum(len(v) for v in paper.values())
    print("=== Convention %s: easy_total=%d paper_total=%d MATCH=%s ===" % (conv,teasy,tpaper,ok))
    for (l,e,p) in diffs[:20]:
        only_easy=sorted(set(e)-set(p)); only_paper=sorted(set(p)-set(e))
        print("  l=%2d (%s): only_in_easy=%s only_in_paper=%s" % (l, format(l,'04b'), only_easy, only_paper))
