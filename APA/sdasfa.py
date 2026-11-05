# Lucrarea de laborator nr.4 - Floyd (prog. dinamica) vs Dijkstra cu heap (Greedy)
# Rulare: python3 lab4.py 10 100 200 300 400 500
import sys, random
import numpy as np

INF = 10**9


class Cnt:
    """Contoare: iteratii, comparatii, operatii aritmetice, actualizari."""
    def __init__(self):
        self.it = self.cmp = self.ar = self.upd = 0

    @property
    def nop(self):
        return self.cmp + self.ar

    def add(self, o):
        self.it += o.it; self.cmp += o.cmp; self.ar += o.ar; self.upd += o.upd


def m_max(n):
    return n * (n - 1) // 2


def m_pentru(caz, n, d=None):
    if caz == "favorabil":
        return n - 1
    if caz == "defavorabil":
        return m_max(n)
    m = int(d * m_max(n))
    if m < n - 1:
        raise ValueError(f"d={d} prea mic pentru n={n}: d >= 2/n = {2 / n:.4f}")
    return m


def graf_conex(n, m, rnd):
    """Graf conex cu exact m muchii: arbore aleator + muchii extra, costuri distincte."""
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
    return [(u, v, c) for (u, v), c in zip(muchii, costuri)]


def matrice(n, muchii):
    L = np.full((n, n), INF, dtype=np.int64)
    np.fill_diagonal(L, 0)
    for u, v, c in muchii:
        L[u, v] = L[v, u] = c
    return L


def liste(n, muchii):
    adj = [[] for _ in range(n)]
    for u, v, c in muchii:
        adj[u].append((v, c)); adj[v].append((u, c))
    return adj


# ---------------- Floyd (programare dinamica), matrice ----------------
def floyd(L, n, c):
    """Ciclul j este vectorizat cu NumPy; contoarele cresc cu n, adica cu numarul
    de iteratii ale ciclului j scalar (test, incrementare, adunare, comparatie)."""
    D = L.copy()
    for k in range(n):
        c.it += 1; c.cmp += 1; c.ar += 1               # ciclul k
        Dk = D[k]
        for i in range(n):
            c.it += 1; c.cmp += 1; c.ar += 1           # ciclul i
            cand = D[i, k] + Dk                         # n adunari
            mask = cand < D[i]                          # n comparatii
            c.it += n; c.cmp += 2 * n; c.ar += 2 * n    # ciclul j: test+comp. / incr.+adunare
            nr = int(mask.sum())
            if nr:
                D[i][mask] = cand[mask]; c.upd += nr
    return D


def floyd_scalar(L, n):
    D = [list(map(int, r)) for r in L]
    for k in range(n):
        for i in range(n):
            for j in range(n):
                if D[i][k] + D[k][j] < D[i][j]:
                    D[i][j] = D[i][k] + D[k][j]
    return D


# ---------------- min-heap binar cu contorizare ----------------
def h_push(h, x, c):
    h.append(x); i = len(h) - 1
    while True:
        c.it += 1; c.cmp += 1                           # i > 0
        if i == 0:
            break
        p = (i - 1) // 2; c.ar += 1                     # indicele parintelui
        c.cmp += 1                                      # h[i] < h[p]
        if h[i] < h[p]:
            h[i], h[p] = h[p], h[i]; i = p
        else:
            break


def h_pop(h, c):
    top = h[0]; last = h.pop()
    if h:
        h[0] = last; i = 0; n = len(h)
        while True:
            c.it += 1
            l = 2 * i + 1; c.ar += 1                    # indicele fiului stang
            c.cmp += 1                                  # l < n
            if l >= n:
                break
            r = l + 1; c.ar += 1
            c.cmp += 1                                  # r < n
            if r < n:
                c.cmp += 1                              # h[r] < h[l]
                if h[r] < h[l]:
                    l = r
            c.cmp += 1                                  # h[l] < h[i]
            if h[l] < h[i]:
                h[l], h[i] = h[i], h[l]; i = l
            else:
                break
    return top


# ---------------- Dijkstra (Greedy), liste de adiacenta + heap ----------------
def dijkstra(adj, n, s, c):
    D = [INF] * n
    for i in range(n):                                  # initializare
        c.it += 1; c.cmp += 1; c.ar += 1
    D[s] = 0
    h = []
    h_push(h, (0, s), c)
    while True:
        c.it += 1; c.cmp += 1                           
        if not h:
            break
        d, u = h_pop(h, c)
        c.cmp += 1                                     
        if d > D[u]:
            continue
        for v, w in adj[u]:                            
            c.it += 1; c.cmp += 1; c.ar += 1
            cand = d + w; c.ar += 1
            c.cmp += 1
            if cand < D[v]:
                D[v] = cand; c.upd += 1
                h_push(h, (cand, v), c)
    return D


# ---------------- Dijkstra (Greedy), matrice de costuri, fara heap ----------------
def dijkstra_mat(Lm, n, s, c):
    """Dijkstra clasic pe matricea costurilor, fara heap: minimul se cauta liniar
    in D (n iteratii), apoi se parcurge linia lui u (n iteratii) -> Theta(n^2)."""
    D = [INF] * n
    vis = [False] * n
    for i in range(n):                                  # initializare
        c.it += 1; c.cmp += 1; c.ar += 1
    D[s] = 0
    for _ in range(n):
        c.it += 1; c.cmp += 1; c.ar += 1                # ciclul exterior
        u = -1; best = INF
        for v in range(n):                              # cautarea minimului
            c.it += 1; c.cmp += 1; c.ar += 1
            c.cmp += 2                                  # not vis[v] si D[v] < best
            if not vis[v] and D[v] < best:
                best = D[v]; u = v
        c.cmp += 1                                      # u == -1
        if u == -1:
            break
        vis[u] = True
        row = Lm[u]
        for v in range(n):                              # parcurgere linie matrice
            c.it += 1; c.cmp += 1; c.ar += 1
            w = row[v]
            c.cmp += 2                                  # vis[v] si w >= INF
            if vis[v] or w >= INF:
                continue
            cand = D[u] + w; c.ar += 1
            c.cmp += 1
            if cand < D[v]:
                D[v] = cand; c.upd += 1
    return D


if __name__ == "__main__":
    ns = [int(a) for a in sys.argv[1:]] or [10, 100, 200, 300, 400, 500]
    EXP = 10
    ALL_MAX = 500                                       # Dijkstra din toate sursele
    d1, d2, d3 = 0.3, 0.5, 0.7
    cazuri = [("favorabil", "favorabil", 0), ("mediu1", "mediu", d1), ("mediu2", "mediu", d2),
              ("mediu3", "mediu", d3), ("defavorabil", "defavorabil", 1)]

    W = [12, 8, 13, 14, 14, 14, 14, 12, 9]
    antet = ["Caz", "m", "Algoritm", "it", "cmp", "ar", "nop", "upd", "Verificat"]

    def linie(vals):
        txt = [str(vals[0]).ljust(W[0]), str(vals[1]).rjust(W[1]), str(vals[2]).ljust(W[2])]
        for i in range(3, 8):
            v = vals[i]
            txt.append((f"{v:,}" if isinstance(v, int) else str(v)).rjust(W[i]))
        txt.append(str(vals[8]).center(W[8]))
        return "  ".join(txt)

    sep = "-" * (sum(W) + 2 * (len(W) - 1))

    for n in ns:
        print(f"\n=== n = {n} ===")
        print(linie(antet))
        print(sep, flush=True)
        for nume, tip, d in cazuri:
            m = m_pentru(tip, n, d)
            rnd = random.Random(12345 + n)
            F, J, M, A = Cnt(), Cnt(), Cnt(), Cnt(); ok = True; na = 0
            for e in range(EXP):
                muchii = graf_conex(n, m, rnd)
                L, adj = matrice(n, muchii), liste(n, muchii)
                cf, cd, cm = Cnt(), Cnt(), Cnt()
                Fl = floyd(L, n, cf)
                if n <= 30:
                    ok &= (Fl.tolist() == floyd_scalar(L, n))
                d0 = dijkstra(adj, n, 0, cd)
                ok &= (list(d0) == Fl[0].tolist())
                Lm = L.tolist()
                ok &= (dijkstra_mat(Lm, n, 0, cm) == Fl[0].tolist())
                if n <= ALL_MAX and (n <= 100 or e == 0):
                    ca = Cnt()
                    for s in range(n):
                        ok &= (dijkstra_mat(Lm, n, s, ca) == Fl[s].tolist())
                    A.add(ca); na += 1
                F.add(cf); J.add(cd); M.add(cm)
            for alg, x, k in (("dijkstra", J, EXP), ("dijkstra_mat", M, EXP), ("floyd", F, EXP),
                              ("dijkstra_all", A, na)):
                if k == 0:
                    continue
                v = [x.it // k, x.cmp // k, x.ar // k, x.nop // k, x.upd // k]
                print(linie([nume, m, alg] + v + ["OK" if ok else "EROARE"]), flush=True)
        print(sep, flush=True)