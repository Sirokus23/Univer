import random

nop = 0
calls = 0   # numar de apeluri de functii
iters = 0   # numar de iteratii ale buclelor

def merge(a, tmp, lo, mid, hi):
    global nop, calls, iters
    calls += 1
    i = lo
    nop += 1
    while i <= hi:
        iters += 1
        tmp[i] = a[i]
        i += 1
        nop += 2

    i = lo
    j = mid + 1
    k = lo
    nop += 4
    while i <= mid and j <= hi:
        iters += 1
        nop += 1
        if tmp[i] <= tmp[j]:
            a[k] = tmp[i]
            i += 1
        else:
            a[k] = tmp[j]
            j += 1
        k += 1
        nop += 5

    nop += 1
    while i <= mid:
        iters += 1
        a[k] = tmp[i]
        k += 1
        i += 1
        nop += 3

    iters += hi - j + 1


def merge_sort(a, tmp, lo, hi):
    global nop, calls
    calls += 1
    nop += 1
    if lo >= hi:
        return
    mid = (lo + hi) // 2
    nop += 2
    merge_sort(a, tmp, lo, mid)
    merge_sort(a, tmp, mid + 1, hi)
    nop += 1
    merge(a, tmp, lo, mid, hi)

def partition(a, lo, hi):
    global nop, calls, iters
    calls += 1
    pivot = a[hi]
    i = lo - 1
    j = lo
    nop += 2
    while j < hi:
        iters += 1
        nop += 1
        if a[j] <= pivot:
            i += 1
            nop += 1
            a[i], a[j] = a[j], a[i]
        j += 1
        nop += 2
    i += 1
    nop += 1
    a[i], a[hi] = a[hi], a[i]
    return i


def quick_sort(a, lo, hi):
    global nop, calls, iters
    calls += 1
    stack = [(lo, hi)]
    while stack:
        iters += 1
        lo, hi = stack.pop()
        nop += 1
        if lo < hi:
            p = partition(a, lo, hi)
            stack.append((lo, p - 1))
            stack.append((p + 1, hi))
            nop += 2


def randomized_quick_sort(a, lo, hi):
    global nop, calls, iters
    calls += 1
    stack = [(lo, hi)]
    while stack:
        iters += 1
        lo, hi = stack.pop()
        nop += 1
        if lo < hi:
            r = random.randint(lo, hi)
            a[r], a[hi] = a[hi], a[r]
            p = partition(a, lo, hi)
            stack.append((lo, p - 1))
            stack.append((p + 1, hi))
            nop += 2

def quick_best(out, lo, length):
    if length == 0:
        return
    if length == 1:
        out[0] = lo
        return
    m = length // 2
    rs = length - m - 1
    quick_best(out, lo, m)
    if rs > 0:
        r = [0] * rs
        quick_best(r, lo + m + 1, rs)
        out[m] = r[rs - 1]
        for i in range(rs - 1):
            out[m + 1 + i] = r[i]
    out[length - 1] = lo + m

def merge_worst(v, length, out, off=0):
    if length <= 1:
        if length == 1:
            out[off] = v[0]
        return
    h1 = (length + 1) // 2
    h2 = length // 2
    e = [v[2 * i] for i in range(h1)]
    o = [v[2 * i + 1] for i in range(h2)]
    merge_worst(e, h1, out, off)
    merge_worst(o, h2, out, off + h1)

def count(algo, inp, n):
    global nop, calls, iters
    work = inp[:]
    tmp = [0] * n
    nop = 0
    calls = 0
    iters = 0
    if algo == 0:
        merge_sort(work, tmp, 0, n - 1)
    elif algo == 1:
        quick_sort(work, 0, n - 1)
    else:
        randomized_quick_sort(work, 0, n - 1)
    assert work == list(range(n)), "algorithm %d did not sort correctly" % algo
    return nop, calls, iters


def print_table(title, sizes, data):
    print("\n  %s" % title)
    print("%8s %14s %14s %14s" % ("n", "Merge", "Quick", "RandQuick"))
    for i in range(len(sizes)):
        print("%8d %14d %14d %14d" % (sizes[i], data[0][i], data[1][i], data[2][i]))


sizes = sorted(int(x) for x in input("Values of n (>= 2), separated by spaces: ").split())
cases = ["BEST", "AVERAGE", "WORST"]
res = [[[] for _ in range(3)] for _ in range(3)]

for n in sizes:
    base = list(range(n))
    rnd = base[:]
    random.shuffle(rnd)
    qb = [0] * n
    quick_best(qb, 0, n)
    mw = [0] * n
    merge_worst(base, n, mw)

    inputs = [
        [base, rnd, mw],  
        [qb, rnd, base],  
        [qb, rnd, base],     
    ]
    for a in range(3):
        for c in range(3):
            res[a][c].append(count(a, inputs[a][c], n))

for c in range(3):
    print("\n==================== %s CASE ====================" % cases[c])
    # extragem fiecare metrica (0 = NoP, 1 = apeluri, 2 = iteratii)
    nop_data = [[t[0] for t in res[a][c]] for a in range(3)]
    call_data = [[t[1] for t in res[a][c]] for a in range(3)]
    iter_data = [[t[2] for t in res[a][c]] for a in range(3)]
    print_table("Numar de operatii (NoP)", sizes, nop_data)
    print_table("Numar de apeluri de functii", sizes, call_data)
    print_table("Numar de iteratii", sizes, iter_data)