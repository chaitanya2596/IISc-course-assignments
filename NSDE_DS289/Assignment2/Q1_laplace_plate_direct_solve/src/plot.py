# Contour plot of the temperature field written by ./heat
import sys
import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
npx, npy = (int(sys.argv[1]), int(sys.argv[2])) if len(sys.argv) > 2 else (128, 256)
T = np.array([[float(v) for v in l.strip().strip(",").split(",")] for l in open("output/xMat%dx%d.csv" % (npx, npy))])
x = np.linspace(2, 3, npx); y = np.linspace(4, 6, npy)
plt.figure(figsize=(5, 8))
cs = plt.contourf(x, y, T, 30, cmap="jet"); plt.colorbar(cs, label="T")
plt.contour(x, y, T, 10, colors="k", linewidths=0.4)
plt.xlabel("x"); plt.ylabel("y"); plt.title("Temperature, %d x %d grid" % (npx, npy))
plt.savefig("output/contour_%dx%d.png" % (npx, npy), dpi=130, bbox_inches="tight")
print("wrote output/contour_%dx%d.png" % (npx, npy))
