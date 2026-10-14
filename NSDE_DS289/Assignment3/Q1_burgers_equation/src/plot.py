# Plots the stored time levels of the three Burgers runs and a comparison at the final time.
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

def load(f):
    head = open(f).readline().strip().split(",")
    return head[1:], np.loadtxt(f, delimiter=",", skiprows=1)

cases = [("output/64_alpha0_Eu_upwind.csv", "Euler + upwind, N = 64, alpha = 0"),
         ("output/1024_alpha0_Eu_upwind.csv", "Euler + upwind, N = 1024, alpha = 0"),
         ("output/1024_alpha0.001_Eu_CD2.csv", "Euler + CD2, N = 1024, alpha = 0.001"),
         ("output/1024_alpha0_Eu_upwind_nonconservative.csv", "non-conservative upwind, N = 1024")]
fig, ax = plt.subplots(2, 2, figsize=(13, 8))
final = []
for a, (f, title) in zip(list(ax.flat) + [None], cases):
    t, d = load(f)
    final.append((d[:, 0], d[:, -1], title))
    if a is None:
        continue
    for k in range(0, len(t), 2):
        a.plot(d[:, 0], d[:, k + 1], label=t[k], lw=1)
    a.plot(d[:, 0], d[:, -1], "k", label=t[-1], lw=1.3)
    a.set_title(title); a.set_xlabel("x"); a.set_ylabel("u"); a.legend(fontsize=7, ncol=2)
a = ax.flat[3]
for x, u, title in final:
    a.plot(x, u, label=title, lw=1)
a.set_title("comparison at t = 0.075"); a.set_xlabel("x"); a.set_ylabel("u"); a.legend(fontsize=7)
plt.tight_layout(); plt.savefig("output/burgers.png", dpi=120)
print("wrote output/burgers.png")
