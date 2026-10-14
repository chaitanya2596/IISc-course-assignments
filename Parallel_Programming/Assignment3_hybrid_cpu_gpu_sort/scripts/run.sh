#!/bin/bash
#SBATCH --job-name=hybrid_sort
#SBATCH --partition=V100          # a GPU partition of your cluster
#SBATCH --gres=gpu:1
#SBATCH --nodes=1
#SBATCH --ntasks=32
#SBATCH --mem=150000M
#SBATCH --time=01:00:00
#SBATCH --output=output/slurm_%j.out
#SBATCH --error=output/slurm_%j.err

# Submit from the assignment folder:   sbatch scripts/run.sh
# Optional: N=4000000000 GPUCAP=1000000000 sbatch scripts/run.sh
module load gnu12/12.4.0 openmpi4/4.1.6 CUDA/12.8    # change to the modules of your cluster

set -e
make hybrid_sort
nvidia-smi --query-gpu=name,memory.total --format=csv,noheader

N=${N:-2000000000}            # number of integers to sort (8 GB)
GPUCAP=${GPUCAP:-500000000}   # integers the GPU is allowed to take; N is 4 times this
P=${P:-32}
MPI_FLAGS=${MPI_FLAGS-"--mca pml ob1 --mca btl self,vader"}   # shared memory inside one node

rm -f output/times.csv
echo "mode,processes,n,elements_per_cpu_process,elements_on_gpu,cpu_local_sort_s,gpu_sort_s,merge_s,total_s,correct" > output/times.csv

echo "===== small correctness runs ====="
mpirun -np 4 $MPI_FLAGS ./hybrid_sort cpu 1000003
mpirun -np 4 $MPI_FLAGS ./hybrid_sort hybrid 1000003 300000
mpirun -np 7 $MPI_FLAGS ./hybrid_sort hybrid 5000011 1200007 3

echo "===== CPU only, $P processes ====="
mpirun -np $P --bind-to core $MPI_FLAGS ./hybrid_sort cpu $N

echo "===== hybrid: $((P-1)) CPU processes + 1 GPU process, different GPU shares ====="
for W in 4 8 16; do
    mpirun -np $P --bind-to core $MPI_FLAGS ./hybrid_sort hybrid $N $GPUCAP $W
done
cat output/times.csv
