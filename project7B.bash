#!/bin/bash
#SBATCH -J project7B_code
#SBATCH -A cs475-575
#SBATCH -p class
#SBATCH -N 8 # number of nodes
#SBATCH -n 8 # number of tasks
#SBATCH -o project7B_code.out
#SBATCH -e project7B_code.err
#SBATCH --mail-type=END,FAIL
#SBATCH --mail-user=liuxiaor@oregonstate.edu
module load slurm
module load openmpi/3.1
for cpu_num in 1 2 4 8 16 32
do
    mpic++ project7B_code.cpp -o project7B_code -lm
    mpiexec -mca btl self,tcp -np $cpu_num project7B_code
done