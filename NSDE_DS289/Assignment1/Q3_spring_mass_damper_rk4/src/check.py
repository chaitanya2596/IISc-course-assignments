# Compares the RK4 displacement with the exact solution of m x'' + c x' + k x = 0, x(0)=1, v(0)=0
# (under-damped case) and prints the observed order of accuracy.
import csv, math, glob
p = [float(l) for l in open("input/sytstemParameters.txt") if l.strip() and not l.startswith("#")]
m, k, c = p
wn = math.sqrt(k/m); z = c/(2*math.sqrt(k*m))
if z >= 1: raise SystemExit("exact-solution check is written for the under-damped case only")
wd = wn*math.sqrt(1-z*z)
exact = lambda t: math.exp(-z*wn*t)*(math.cos(wd*t) + z*wn/wd*math.sin(wd*t))
res = []
for f in glob.glob("output/springMassDamper_rk4_%f_*.csv" % c):
    h = float(f[:-4].split("_")[-1])
    rows = list(csv.reader(open(f)))[1:]
    err = max(abs(float(r[1]) - exact(float(r[0]))) for r in rows)
    res.append((h, err))
res.sort(reverse=True); prev = None
print("m=%g k=%g c=%g  damping ratio=%.4f" % (m, k, c, z))
for h, e in res:
    print("h=%-8g max |x - exact| = %.3e%s" % (h, e, "" if prev is None else "   order=%.2f" % math.log(prev/e, 2)))
    prev = e
