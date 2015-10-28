CFLAG=-O3
BIN=.
DATA=/usr/local/cs133/lab4/data
CC=mpicc

all: sw

sw: smith_waterman_mpi.c
	$(CC) $(CFLAG) -o $(BIN)/smith_waterman_mpi smith_waterman_mpi.c

clean:
	rm $(BIN)/smith_waterman_mpi || true

run: run_sw 

run_sw:
	mpirun --bind-to-core -np 16 $(BIN)/smith_waterman_mpi $(DATA)/largeX.txt $(DATA)/largeY.txt
