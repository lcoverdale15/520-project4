#!/bin/bash
#SBATCH --constraint=moles
#SBATCH --cpus-per-task=4
#SBATCH --nodes=1
#SBATCH --mem-per-cpu=2G
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
gcc -fopenmp OpenMP.c -o OpenMP.o
perf stat -o perf_1core.txt ./OpenMP.o
