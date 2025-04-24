#!/bin/bash
#SBATCH --job-name=pthreads_test
#SBATCH --output=result_%j.txt
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --constraint=mole
#SBATCH --time=00:10:00
#SBATCH --mem=4G

# Change this to your actual path
./pthread.c 4
