#!/bin/bash
#SBATCH --job-name=sssp_dbg
#SBATCH --partition=V100
#SBATCH --gres=gpu:1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --time=00:30:00
#SBATCH --output=output/dbg_%j.out
#SBATCH --error=output/dbg_%j.err
# Stress test for correctness: many repetitions over several bucket widths, every run checked against Dijkstra.
module load gnu12/12.4.0 CUDA/12.8
set -e
make sssp
total=0; wrong=0
for rep in 1 2 3 4 5 6 7 8; do for d in 0 200 1000 5000 20000 100000; do
    out=$(./sssp input/test_graph.mtx $d 1 500 1000 10000 50000 100000 || true)
    c=$(echo "$out" | grep -c "CORRECT" || true); w=$(echo "$out" | grep -c "   WRONG" || true)
    total=$((total + c + w)); wrong=$((wrong + w))
    [ "$w" -gt 0 ] && echo "rep $rep delta $d: $w wrong" && echo "$out" | grep mismatch | head -3
done; done
echo "runs checked against Dijkstra: $total, wrong: $wrong"
