# Compares implicit Euler with the exact solution y = exp(-x) - exp(-200000 x).
import csv, glob
for f in sorted(glob.glob("output/EUI_*.csv")):
    rows = list(csv.reader(open(f)))[1:]
    err = max(abs(float(r[1]) - float(r[2])) for r in rows)
    print("%s  steps=%d  max |y - exact| = %.3e" % (f, len(rows)-1, err))
