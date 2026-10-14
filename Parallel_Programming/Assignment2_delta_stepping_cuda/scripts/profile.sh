#!/bin/bash
#SBATCH --job-name=sssp_prof
#SBATCH --partition=V100
#SBATCH --gres=gpu:1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --time=00:30:00
#SBATCH --output=output/profile_%j.out
#SBATCH --error=output/profile_%j.err

# CUDA profile (Nsight Systems) of the GPU delta-stepping for source 1, at the default delta and at delta = 5000.
# Submit from the assignment folder:   sbatch scripts/profile.sh
module load gnu12/12.4.0 CUDA/12.8
set -e
[ -x sssp ] || make sssp
for d in 0 5000; do
    echo "===== delta = $d"
    nsys profile --stats=false --force-overwrite=true -o output/profile_delta${d} ./sssp input/test_graph.mtx $d 1 > /dev/null
    nsys stats --report cuda_gpu_kern_sum,cuda_api_sum,cuda_gpu_mem_time_sum --force-export=true --format csv output/profile_delta${d}.nsys-rep > output/profile_delta${d}_summary.txt 2>/dev/null || true
    cat output/profile_delta${d}_summary.txt
done
