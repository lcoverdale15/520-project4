#!/bin/bash
#SBATCH --constraint=moles
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=2G  # Increase memory per CPU to 2 because 1 is not enough
#SBATCH --time=01:00:00  # Set a time limit just incase something breaks

gcc pthread.c -o pthread.o -lpthread
perf stat -o perf_${SLURM_CPUS_PER_TASK}core.txt ./pthread.o $SLURM_CPUS_PER_TASK

