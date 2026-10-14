#!/bin/bash
#SBATCH --job-name=ds221_a1
#SBATCH --partition=cpu           # a CPU partition of your cluster
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --time=03:00:00
#SBATCH --output=output/slurm_%j.out
#SBATCH --error=output/slurm_%j.err

# Submit from the assignment folder:   sbatch scripts/run.sh     (or run it directly: bash scripts/run.sh)
# Optional: SIZES="5 10 100" BLOCK=32 sbatch scripts/run.sh
module load gnu12/12.4.0 2>/dev/null   # change to the compiler module of your cluster

set -e
SIZES=${SIZES:-"5 10 20 50 100 200 400 800"}
BLOCK=${BLOCK:-32}
CXX=${CXX:-g++}
# build only if needed (on some clusters the compute nodes cannot compile; then run
#   g++ -std=c++11 -O2 -o ds221_driver src/main.cpp && g++ -std=c++11 -O2 -o gen_inputs src/gen_inputs.cpp
# on the login node first)
[ -x ds221_driver ] || $CXX -std=c++11 -O2 -o ds221_driver src/main.cpp
[ -x gen_inputs ] || $CXX -std=c++11 -O2 -o gen_inputs src/gen_inputs.cpp
mkdir -p input/generated output/run

echo "n,Q1_dense_to_csr_ns,Q2a_dense_mult_ns,Q2b_block_mult_ns,Q2c_sparse_mult_ns,Q3_map_ns,Q3_vector_ns" > output/times.csv
for n in $SIZES; do
    ./gen_inputs $n $BLOCK input/generated
    I=input/generated; O=output/run
    ./ds221_driver $I/p1_$n.txt $O/o1_$n.txt $I/p2a_$n.txt $O/o2a_$n.txt $I/p2b_$n.txt $O/o2b_$n.txt \
                   $I/p2c_$n.txt $O/o2c_$n.txt $I/p3_$n.txt $O/o3_$n.txt $O/time_$n.csv > /dev/null
    echo "--- n = $n"
    python3 scripts/check.py $I/p1_$n.txt $O/o1_$n.txt $I/p2a_$n.txt $O/o2a_$n.txt $I/p2b_$n.txt $O/o2b_$n.txt \
                             $I/p2c_$n.txt $O/o2c_$n.txt $I/p3_$n.txt $O/o3_$n.txt
    echo "$n,$(tail -1 $O/time_$n.csv)" >> output/times.csv
done
echo; cat output/times.csv
