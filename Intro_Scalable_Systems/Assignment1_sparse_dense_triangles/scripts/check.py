"""Independent check of the outputs of main.cpp with numpy.
Usage: python3 scripts/check.py <p1 in> <p1 out> <p2a in> <p2a out> <p2b in> <p2b out> <p2c in> <p2c out> <p3 in> <p3 out>
Prints one line per problem: OK or the first difference."""
import sys
import numpy as np

def ints(line):
    return [int(x) for x in line.split()]

def read_dense(lines, start, n):
    return np.array([ints(lines[start + i]) for i in range(n)], dtype=np.int64)

def to_csr(M):
    vals, cols, rows = [], [], [0]
    for r in M:
        nz = np.nonzero(r)[0]
        vals += list(r[nz]); cols += list(nz); rows.append(len(vals))
    return [int(v) for v in vals], [int(c) for c in cols], rows

def csr_lines(path):
    L = open(path).read().split("\n")
    return [ints(L[k]) if k < len(L) else [] for k in range(3)]

def report(name, ok, detail=""):
    print("%-4s %s %s" % (name, "OK" if ok else "WRONG", detail))
    return ok

a = sys.argv[1:]
allok = True

# Q1 dense -> CSR
L = open(a[0]).read().split("\n"); n = int(L[0])
A = read_dense(L, 1, n)
allok &= report("Q1", csr_lines(a[1]) == list(to_csr(A)), "n=%d nonzeros=%d" % (n, np.count_nonzero(A)))

# Q2a / Q2b dense products
for name, fin, fout, skip in (("Q2a", a[2], a[3], 1), ("Q2b", a[4], a[5], 1)):
    L = open(fin).read().split("\n"); n = ints(L[0])[0]
    A = read_dense(L, 1, n); B = read_dense(L, 1 + n, n)
    out = np.array([ints(l) for l in open(fout).read().split("\n") if l.strip()], dtype=np.int64)
    ok = out.shape == (n, n) and np.array_equal(out, A @ B)
    allok &= report(name, ok, "n=%d" % n + ("" if name == "Q2a" else " block=%d" % ints(L[0])[1]))

# Q2c sparse product
L = [l for l in open(a[6]).read().split("\n")]
n = ints(L[0])[0]
def dense_from(v, c, r):
    M = np.zeros((n, n), dtype=np.int64)
    for i in range(n):
        for k in range(r[i], r[i + 1]):
            M[i, c[k]] = v[k]
    return M
A = dense_from(ints(L[1]), ints(L[2]), ints(L[3])); B = dense_from(ints(L[4]), ints(L[5]), ints(L[6]))
allok &= report("Q2c", csr_lines(a[7]) == list(to_csr(A @ B)), "n=%d nonzeros A=%d B=%d" % (n, np.count_nonzero(A), np.count_nonzero(B)))

# Q3 directed triangles i -> j -> k -> i, pattern = (w(i,j), w(j,k), w(k,i)), counted once per starting vertex
L = open(a[8]).read().split("\n"); n = int(L[0])
W = read_dense(L, 1, n); E = W > 0
base = int(W.max()) + 1
counts = {}
for i in range(n):
    mask = E[i][:, None] & E & E[:, i][None, :]   # j, k with i->j, j->k, k->i
    j, k = np.nonzero(mask)
    if len(j) == 0:
        continue
    code = (W[i, j] * base + W[j, k]) * base + W[k, i]
    u, c = np.unique(code, return_counts=True)
    for x, y in zip(u, c):
        counts[int(x)] = counts.get(int(x), 0) + int(y)
ref = {(x // base // base, x // base % base, x % base): c for x, c in counts.items()}
rows = [tuple(ints(l)) for l in open(a[9]).read().split("\n") if l.strip()]
half = len(rows) // 2
mapPart = {r[:3]: r[3] for r in rows[:half]}
vecPart = {r[:3]: r[3] for r in rows[half:]}
ok = len(rows) == 2 * len(ref) and mapPart == ref and vecPart == ref and rows[:half] == sorted(rows[:half])
allok &= report("Q3", ok, "n=%d triangles=%d distinct patterns=%d (map and vector results both checked)" % (n, sum(ref.values()), len(ref)))

sys.exit(0 if allok else 1)
