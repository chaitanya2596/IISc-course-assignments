#!/bin/bash
#SBATCH --job-name=sssp_stress
#SBATCH --partition=A100
#SBATCH --gres=gpu:1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --time=01:00:00
#SBATCH --output=output/stress_%j.out
#SBATCH --error=output/stress_%j.err
# Repeated runs over several bucket widths, every run checked against Dijkstra.
# Submit from the assignment folder:   sbatch scripts/stress.sh     (sbatch -p V100 scripts/stress.sh for another GPU)
module load gnu12/12.4.0 CUDA/12.8
nvidia-smi --query-gpu=name --format=csv,noheader
REPS=${REPS:-8}
total=0; wrong=0
for rep in $(seq 1 $REPS); do for d in 0 200 1000 5000 20000 100000; do
    out=$(./sssp input/test_graph.mtx $d 1 500 1000 10000 50000 100000)
    c=$(echo "$out" | grep -c "   CORRECT"); w=$(echo "$out" | grep -c "   WRONG")
    total=$((total + c + w)); wrong=$((wrong + w))
    [ "$w" -gt 0 ] && echo "rep $rep delta $d: $w wrong" && echo "$out" | grep mismatch | head -3
done; done
echo "source runs checked against Dijkstra: $total, wrong: $wrong"
