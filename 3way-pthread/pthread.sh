#!/bin/bash
#SBATCH --constraint=moles
#SBATCH --nodes=2
#SBATCH --cpus-per-task=8
#SBATCH --mem-per-cpu=1G
gcc pthread.c -o pthread.o
perf stat -o perf_16core.txt ./pthread.o
