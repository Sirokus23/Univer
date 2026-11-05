import argparse
import random
import sys

sys.setrecursionlimit(10000)
INF = float("inf")

class Cnt:
    def __init__(self):
        self.it = self.cmp = self.ar = self.lg = 0

    @property
    def nop(self):
        return self.cmp + self.ar + self.lg

def m_max(n):
    return n * (n - 1) // 2

def m_pentru(caz, n, d=None):
    if caz == "favorabil":
        return n - 1
    if caz == "defavorabil":
        return m_max(n)
    m = int(d * m_max(n))
    if m < n - 1:
        raise ValueError(f"d={d} prea mic pentru n={n}: d >= 2/n = {2/n:.4f}")
    return m

def graf_conex(n, m, rnd):
    noduri = list(range(n))
    rnd.shuffle(noduri)
    muchii = set()
    for i in range(1, n):
        u, v = noduri[i], noduri[rnd.randrange(i)]
        muchii.add((min(u, v), max(u, v)))
    while len(muchii) < m:
        u, v = rnd.sample(range(n), 2)
        muchii.add((min(u, v), max(u, v)))
    costuri = rnd.sample(range(1, 10 * m + 1), m)
    return [(c, u, v) for (u, v), c in zip(muchii, costuri)]

def merge_sort(a, c):
    if len(a) <= 1:
        return a
    mid = len(a) // 2
    st, dr = merge_sort(a[:mid], c), merge_sort(a[mid:], c)
    r, i, j = [], 0, 0
    while i < len(st) and j < len(dr):
        c.cmp += 3; c.lg += 1; c.it += 1
        if st[i][0] <= dr[j][0]:
            r.append(st[i]); i += 1
        else:
            r.append(dr[j]); j += 1
        c.ar += 1
    c.cmp += 2
    r.extend(st[i:]); r.extend(dr[j:])
    return r

def kruskal(n, muchii, c):
    muchii = merge_sort(list(muchii), c)
    parinte = list(range(n))
    rang = [0] * n

    def find(x):
        rad = x
        while True:
            c.cmp += 1
            if parinte[rad] == rad:
                break
            rad = parinte[rad]
        while True:
            c.cmp += 1
            if parinte[x] == rad:
                break
            parinte[x], x = rad, parinte[x]
            c.ar += 1
        return rad

    A, k = [], 0
    while True:
        cost, u, v = muchii[k]
        k += 1; c.ar += 1; c.it += 1
        cu, cv = find(u), find(v)
        c.cmp += 1
        if cu != cv:
            c.cmp += 1
            if rang[cu] < rang[cv]:
                cu, cv = cv, cu
            parinte[cv] = cu
            c.cmp += 1
            if rang[cu] == rang[cv]:
                rang[cu] += 1; c.ar += 1
            A.append((u, v, cost)); c.ar += 1
        c.cmp += 1
        if len(A) == n - 1:
            break
    return A

def prim(n, muchii, c):
    C = [[INF] * n for _ in range(n)]
    for cost, u, v in muchii:
        C[u][v] = C[v][u] = cost
    A, vecin, mincost = [], [0] * n, [0] * n
    for i in range(1, n):
        c.it += 1; c.cmp += 1
        vecin[i] = 0
        mincost[i] = C[i][0]
    for _ in range(n - 1):
        c.it += 1; c.cmp += 1
        mn, k = INF, -1
        for j in range(1, n):
            c.it += 1; c.cmp += 3; c.lg += 1
            if 0 < mincost[j] < mn:
                mn, k = mincost[j], j
        A.append((k, vecin[k], mn)); c.ar += 1
        mincost[k] = -1
        for j in range(1, n):
            c.it += 1; c.cmp += 2
            if C[k][j] < mincost[j]:
                mincost[j] = C[k][j]
                vecin[j] = k
    return A

def experiment(n, m, runs, rnd):
    s = {"K": [0, 0], "P": [0, 0]}
    for _ in range(runs):
        g = graf_conex(n, m, rnd)
        ck, cp = Cnt(), Cnt()
        ak, ap = kruskal(n, g, ck), prim(n, g, cp)
        assert sum(x[2] for x in ak) == sum(x[2] for x in ap), "MST diferit!"
        s["K"][0] += ck.it; s["K"][1] += ck.nop
        s["P"][0] += cp.it; s["P"][1] += cp.nop
    return {a: (v[0] // runs, v[1] // runs) for a, v in s.items()}

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("n", nargs="*", type=int, help="valori n (liber)")
    ap.add_argument("--d", nargs=3, type=float, default=[0.3, 0.5, 0.7])
    ap.add_argument("--runs", type=int, default=10)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--sweep", type=int, metavar="N",
                    help="Nop pentru n=N la densitati crescatoare")
    a = ap.parse_args()
    if a.sweep:
        rnd = random.Random(a.seed)
        print(f"Nop la n={a.sweep} (medie din {a.runs} experimente)")
        print("   d    |     m  |  Kruskal  |    Prim   | Kruskal/Prim")
        for d in (0.01, 0.02, 0.03, 0.04, 0.05, 0.075, 0.1, 0.15, 0.2, 0.3):
            if d < 2 / a.sweep:
                continue
            m = m_pentru("mediu", a.sweep, d)
            r = experiment(a.sweep, m, a.runs, rnd)
            print(f"{d:>7.3f} | {m:>6} | {r['K'][1]:>9} | {r['P'][1]:>9} | "
                  f"{r['K'][1]/r['P'][1]:>6.2f}")
        return
    ns = a.n or [10, 100, 200, 300, 400, 500]
    if len(set(a.d)) != 3 or not all(0 < x < 1 for x in a.d):
        sys.exit("d1,d2,d3 trebuie sa fie distincte si in (0,1)")
    rnd = random.Random(a.seed)

    cazuri = [("favorabil", None, "m = n-1")]
    for i, d in enumerate(a.d, 1):
        cazuri.append(("mediu", d, f"m = {{n(n-1)/2 * d{i}}}, d{i}={d}"))
    cazuri.append(("defavorabil", None, "m = n(n-1)/2"))

    for t, (caz, d, titlu) in enumerate(cazuri, 1):
        rez, ms = {}, {}
        for n in ns:
            try:
                ms[n] = m_pentru(caz, n, d)
                rez[n] = experiment(n, ms[n], a.runs, rnd)
            except ValueError as e:
                rez[n] = None; print("!", e)
        for idx, nume in ((0, "iteratii"), (1, "Nop")):
            print(f"\nTabelul x.{t} - {nume} - caz {caz}: {titlu} "
                  f"(medie din {a.runs} experimente)")
            print("Algoritm | " + " | ".join(f"{n:>9}" for n in ns))
            vm = [f"{ms[n]:>9}" if rez[n] else f"{'-':>9}" for n in ns]
            print(f"{'m':<8} | " + " | ".join(vm))
            for key, an in (("K", "Kruskal"), ("P", "Prim")):
                vals = [f"{rez[n][key][idx]:>9}" if rez[n] else f"{'-':>9}"
                        for n in ns]
                print(f"{an:<8} | " + " | ".join(vals))

if __name__ == "__main__":
    main()