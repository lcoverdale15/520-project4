TARGET = run_openmp
CPUS ?= 4  # Default to 4 if not provided

# Default target to run the executable
.PHONY: all
all: run

# Run the MPI job using sbatch
.PHONY: run
run:
	export CPUS=$(CPUS) && sbatch --cpus-per-task=$(CPUS) OpenMP.sh

