#!/bin/bash
#SBATCH --job-name=omp_hist
#SBATCH --partition=cpu           # a CPU partition of your cluster
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32        # the largest thread count used is 32
#SBATCH --exclusive
#SBATCH --time=00:20:00
#SBATCH --output=output/slurm_%j.out
#SBATCH --error=output/slurm_%j.err

# Submit from the question folder:   sbatch scripts/run.sh     (or simply run it: bash scripts/run.sh)
module load gnu12/12.4.0 2>/dev/null   # change to the compiler module of your cluster

set -e
make
[ -f input/input.txt ] || ./inputGenerator 200000 50
export OMP_PROC_BIND=close
export OMP_PLACES=cores
export OMP_WAIT_POLICY=active     # threads stay awake between the repeated parallel regions
./histogram
