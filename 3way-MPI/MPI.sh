#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=16 #Adjust based on number of processes we want (change for each test)
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=2G  # Increase memory per CPU to 2 because 1 is not enough
#SBATCH --time=01:00:00  # Set a time limit just incase something breaks
#SBATCH --constraint=moles
#SBATCH --output=slurm-%j.out
#SBATCH --error=slurm-%j.err



module load OpenMPI/4.1.4-GCC-11.3.0

mpicc -o MPI.o MPI.c
perf stat -r 10 -o perf_${SLURM_NTASKS}tasks.txt mpirun -np $SLURM_NTASKS ./MPI.o