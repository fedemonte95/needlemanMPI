#! /bin/sh

mpirun --bind-to-core -np 16 ./smith_waterman_mpi $1 $2

