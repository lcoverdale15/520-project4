# Running the Programs

To run all three programs from the root directory, or to run an individual program from within its own directory, use the following command:

```bash
make CPUS={number of CPUs you want}
eg: make CPUS=4
```
This will output the slurm file and a textfile with data on how the run went into the right directory
