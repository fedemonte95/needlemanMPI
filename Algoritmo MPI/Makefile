CFLAG=-O3
BIN=.
CC=mpicc

all: sw

sw: smith_waterman_mpi.c
	$(CC) $(CFLAG) -o $(BIN)/NeedlemanWunsch_mpi NeedlemanWunsch_mpi.c

clean:
	rm $(BIN)/NeedlemanWunsch_mpi || true

run: run_sw 

run_sw:
	mpirun --bind-to-core -np 16 $(BIN)/NeedlemanWunsch_mpi sec1.txt sec2.txt
