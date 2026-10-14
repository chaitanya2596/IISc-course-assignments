# Finite element model of the bar  Ac E d2u/dx2 = P(x),  u(0) = u(L) = 0
# Linear two-node elements of length dx. Weak form:  Ac E * integral(u' v') dx = - integral(P v) dx
#   element stiffness  = (Ac E / dx) * [[1, -1], [-1, 1]]
#   element load       = -(P dx / 2) * [1, 1]
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

Ac = 0.1        # cross-sectional area, m^2
E = 200e9       # Young's modulus, N/m^2
L = 10.0        # length, m
P = 100.0       # distributed load, N/m
dx = 0.5        # element length, m

nel = int(round(L / dx))
num = nel + 1
x = np.linspace(0.0, L, num)

# assembly
K = np.zeros((num, num))
F = np.zeros(num)
ke = (Ac * E / dx) * np.array([[1.0, -1.0], [-1.0, 1.0]])
fe = -(P * dx / 2.0) * np.array([1.0, 1.0])
for e in range(nel):
    nodes = [e, e + 1]
    for a in range(2):
        F[nodes[a]] += fe[a]
        for b in range(2):
            K[nodes[a], nodes[b]] += ke[a, b]

# fixed ends: u = 0 at the first and last node
for n in (0, num - 1):
    K[n, :] = 0.0
    K[:, n] = 0.0
    K[n, n] = 1.0
    F[n] = 0.0

u = np.linalg.solve(K, F)

# exact solution u = P x (x - L) / (2 Ac E)
exact = P * x * (x - L) / (2.0 * Ac * E)

np.savetxt("output/deflection.csv", np.column_stack([x, u, exact]), delimiter=",",
           header="x,u_fem,u_exact", comments="", fmt="%.10e")
print("nodes = %d, elements = %d" % (num, nel))
print("largest deflection (FEM)   = %.6e m at x = %.2f m" % (u.min(), x[u.argmin()]))
print("largest deflection (exact) = %.6e m" % exact.min())
print("max |u_fem - u_exact| over the nodes = %.3e m" % np.abs(u - exact).max())

plt.figure()
xf = np.linspace(0, L, 400)
plt.plot(xf, P * xf * (xf - L) / (2.0 * Ac * E), "k-", lw=1, label="exact")
plt.plot(x, u, marker="*", color="green", ls="--", label="FEM, dx = 0.5 m")
plt.xlabel("x (m)"); plt.ylabel("u (m)"); plt.title("Deflection of the bar"); plt.legend(); plt.grid(alpha=0.3)
plt.savefig("output/Verticaldeflection.png", dpi=130, bbox_inches="tight")
