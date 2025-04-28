#!/bin/bash
#SBATCH --constraint=moles
#SBATCH --nodes=1
#SBATCH --mem-per-cpu=2G
export OMP_NUM_THREADS=4
gcc -fopenmp OpenMP.c -o OpenMP.o
perf stat -o perf_1core.txt ./OpenMP.o