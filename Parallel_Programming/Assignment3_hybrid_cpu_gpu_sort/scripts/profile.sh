#!/bin/bash
#SBATCH --job-name=hsort_prof
#SBATCH --partition=V100
#SBATCH --gres=gpu:1
#SBATCH --nodes=1
#SBATCH --ntasks=8
#SBATCH --mem=60000M
#SBATCH --time=00:30:00
#SBATCH --output=output/profile_%j.out
#SBATCH --error=output/profile_%j.err

# CUDA profile (Nsight Systems) of a hybrid run: 8 processes, 400 million integers, GPU share 4.
# Submit from the assignment folder:   sbatch scripts/profile.sh
module load gnu12/12.4.0 openmpi4/4.1.6 CUDA/12.8
set -e
[ -x hybrid_sort ] || make hybrid_sort
nsys profile --stats=false --force-overwrite=true --trace=cuda,mpi -o output/profile_hybrid \
    mpirun -np 8 --mca pml ob1 --mca btl self,vader ./hybrid_sort hybrid 400000000 100000000 4
nsys stats --report cuda_gpu_kern_sum,cuda_api_sum,cuda_gpu_mem_time_sum --force-export=true --format csv output/profile_hybrid.nsys-rep > output/profile_hybrid_summary.txt 2>/dev/null || true
cat output/profile_hybrid_summary.txt
