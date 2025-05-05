.PHONY: all 3way-MPI 3way-OpenMP 3way-pthread

# Default CPU count
CPUS ?= 4

all: 3way-MPI 3way-OpenMP 3way-pthread

3way-MPI:
	$(MAKE) -C 3way-MPI CPUS=$(CPUS)

3way-OpenMP:
	$(MAKE) -C 3way-OpenMP CPUS=$(CPUS)

3way-pthread:
	$(MAKE) -C 3way-pthread CPUS=$(CPUS)
