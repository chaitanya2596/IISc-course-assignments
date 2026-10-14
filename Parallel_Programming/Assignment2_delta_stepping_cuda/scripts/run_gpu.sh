#!/bin/bash
#SBATCH --job-name=sssp_gpu
#SBATCH --partition=V100          # any GPU partition of your cluster
#SBATCH --gres=gpu:1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --time=00:30:00
#SBATCH --output=output/slurm_%j.out
#SBATCH --error=output/slurm_%j.err

# Submit from the assignment folder:   sbatch scripts/run_gpu.sh
# The large graph is expected at input/test_graph.mtx (see README for the download link).
module load gnu12/12.4.0 CUDA/12.8    # change to the compiler / CUDA modules of your cluster

set -e
make sssp
nvidia-smi --query-gpu=name,memory.total --format=csv,noheader

GRAPH=input/test_graph.mtx
SOURCES="1 500 1000 10000 50000 100000"

echo "===== small test graphs ====="
for t in 3 5 8 10; do ./sssp input/testCase_$t.mtx 0 0 | grep -v "dist\["; done

echo "===== large graph, delta = average edge weight + 1 ====="
./sssp $GRAPH 0 $SOURCES

echo "===== effect of the bucket width delta ====="
for d in 200 1000 5000 20000 100000; do
    echo "--- delta = $d"
    ./sssp $GRAPH $d $SOURCES | grep -E "sequential|GPU delta|speedup|ALL|SOME"
done
