#!/bin/bash
#SBATCH --job-name=matmul_mpi
#SBATCH --partition=cpu           # a CPU partition of your cluster
#SBATCH --nodes=2                 # 64 processes need two 32-core nodes; sequential and 16 processes use the first node
#SBATCH --ntasks-per-node=32
#SBATCH --exclusive
#SBATCH --time=06:00:00
#SBATCH --output=output/slurm_%j.out
#SBATCH --error=output/slurm_%j.err

# Submit from the assignment folder:   sbatch scripts/run.sh
# Optional: SIZES="6400 12800" PROCS="16 64" sbatch scripts/run.sh
module load gnu12/12.4.0 openmpi4/4.1.6 2>/dev/null   # change to the compiler / MPI modules of your cluster

# Transport used by Open MPI. On this cluster the default (UCX) crashes when a run spans two nodes,
# so plain TCP between nodes + shared memory inside a node is selected. Clear MPI_FLAGS if your
# cluster's default transport works.
MPI_FLAGS=${MPI_FLAGS-"--mca pml ob1 --mca btl tcp,self,vader"}

set -e
make
SIZES=${SIZES:-"6400 12800 25600"}
PROCS=${PROCS:-"16 64"}
rm -f output/times.csv output/correctness.txt
echo "algorithm,N,processes,read_s,compute_s,write_s" > output/times.csv

for N in $SIZES; do
    A=input/mat_${N}_A.bin; B=input/mat_${N}_B.bin
    [ -f $A ] || ./Mat_generator $N input

    ./seq_matmul $N $A $B output/C_seq_$N.bin
    for P in $PROCS; do
        for ALG in cannon summa; do
            mpirun -np $P --bind-to core $MPI_FLAGS ./$ALG $N $A $B output/C_${ALG}_${N}_$P.bin
            # the parallel result must be byte for byte the same file as the sequential result
            if cmp -s output/C_seq_$N.bin output/C_${ALG}_${N}_$P.bin; then
                echo "N=$N p=$P $ALG: identical to sequential" | tee -a output/correctness.txt
            else
                echo "N=$N p=$P $ALG: DIFFERENT from sequential" | tee -a output/correctness.txt
            fi
            rm -f output/C_${ALG}_${N}_$P.bin
        done
    done
    # the matrices are large (2.6 GB each at N = 25600), keep only the timing and correctness records
    rm -f output/C_seq_$N.bin $A $B
done

python3 scripts/speedup.py
