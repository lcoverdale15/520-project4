#!/bin/bash
#SBATCH --nodes=1
#SBATCH --ntasks=8               # Default, overridden by sbatch --ntasks=...
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=2G
#SBATCH --time=01:00:00
#SBATCH --constraint=moles

module load OpenMPI/4.1.4-GCC-11.3.0

# Optional: Compile the program (only if not done in Makefile)
mpicc -o MPI.o MPI.c

# Print the number of MPI tasks for confirmation
echo "Running with $SLURM_NTASKS MPI tasks"

# Run the program with perf for 10 repetitions
perf stat -r 10 -o perf_${SLURM_NTASKS}tasks.txt mpirun -np $SLURM_NTASKS ./MPI.o

