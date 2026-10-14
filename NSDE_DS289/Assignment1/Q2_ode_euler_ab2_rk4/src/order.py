# Prints the observed order of accuracy from the error tables written by the solver.
import csv, math
for name in ["EU", "ABS", "RK4"]:
    rows = list(csv.reader(open("output/errorFile%s.csv" % name)))[1:]
    h = [float(r[0]) for r in rows]; mx = [float(r[1]) for r in rows]; av = [float(r[2]) for r in rows]
    print("%-4s step sizes %s" % (name, h))
    for i in range(1, len(h)):
        print("     h=%-8g max err=%-12.4e order=%.2f   avg err=%-12.4e order=%.2f" % (
            h[i], mx[i], math.log(mx[i-1]/mx[i], 2), av[i], math.log(av[i-1]/av[i], 2)))
