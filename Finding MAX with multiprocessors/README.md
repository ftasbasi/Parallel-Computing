This project is tested on HPC server but you may try on your own with PC.
You just need to have MPICC on your device.

Compile with:
mpicc projectname.c -o projectname -lm

Run with:
mpiexec -n x projectname ...  >>> x is number of processors you will run in parallel. the rest is only taking input from stdin arguments.
