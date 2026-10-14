# Builds the speedup table from output/times.csv
import csv
rows = list(csv.DictReader(open("output/times.csv")))
seq = {r["N"]: r for r in rows if r["algorithm"] == "sequential"}
out = open("output/speedup.csv", "w")
head = "algorithm,N,processes,compute_s,compute_speedup,efficiency,total_s,total_speedup"
print(head); out.write(head + "\n")
tot = lambda r: float(r["read_s"]) + float(r["compute_s"]) + float(r["write_s"])
for r in rows:
    s = seq.get(r["N"])
    if s is None:
        continue
    p = int(r["processes"]); c = float(r["compute_s"]); sc = float(s["compute_s"])
    line = "%s,%s,%d,%.3f,%.2f,%.2f,%.3f,%.2f" % (r["algorithm"], r["N"], p, c, sc / c, sc / c / p, tot(r), tot(s) / tot(r))
    print(line); out.write(line + "\n")
out.close()
