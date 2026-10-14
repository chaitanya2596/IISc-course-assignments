#!/bin/bash
#SBATCH --job-name=mpi_msort
#SBATCH --partition=cpu           # a CPU partition of your cluster
#SBATCH --nodes=2                 # 64 processes need two 32-core nodes; 8, 16 and 32 run on the first node
#SBATCH --ntasks-per-node=32
#SBATCH --exclusive
#SBATCH --time=00:20:00
#SBATCH --output=output/slurm_%j.out
#SBATCH --error=output/slurm_%j.err

# Submit from the question folder:   sbatch scripts/run.sh
module load gnu12/12.4.0 openmpi4/4.1.6 2>/dev/null   # change to the compiler / MPI modules of your cluster

# Transport used by Open MPI. On this cluster the default (UCX) crashes when a run spans two nodes,
# so plain TCP between nodes + shared memory inside a node is selected. Clear MPI_FLAGS if your
# cluster's default transport works.
MPI_FLAGS=${MPI_FLAGS-"--mca pml ob1 --mca btl tcp,self,vader"}

set -e
make
[ -f input/input.txt ] || ./inputGenerator 100000
rm -f output/speedup.csv
echo "processes,elements,quicksort_ms,tree_ms,tree_speedup,process0_ms,process0_speedup,correct" > output/speedup.csv
for p in 1 8 16 32 64; do
    mpirun -np $p --bind-to core $MPI_FLAGS ./mergesort
    echo
done
cat output/speedup.csv
