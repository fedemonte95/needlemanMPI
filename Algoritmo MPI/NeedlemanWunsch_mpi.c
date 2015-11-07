#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <string.h>

#include <mpi.h>

//#define BLOCK_SIZE 1000

signed char* sec_1;
signed char* sec_2;

size_t clear_whitespaces_do_mapping ( signed char* buffer, long size ) {
    size_t non_ws_index = 0, traverse_index = 0;

    while ( traverse_index < size ) {
        char curr_char = buffer[traverse_index];
        switch ( curr_char ) {
            case 'A': case 'C': case 'G': case 'T':
                /*this used to be a copy not also does mapping*/
                buffer[non_ws_index++] = curr_char;
                break;
        }
        ++traverse_index;
    }
    return non_ws_index;
}

signed char* read_file( FILE* file, size_t* n_chars ) {
    fseek (file, 0L, SEEK_END);
    long file_size = ftell (file);
    fseek (file, 0L, SEEK_SET);

    signed char *file_buffer = (signed char *)malloc((1+file_size)*sizeof(signed char));

    size_t n_read_from_file = fread(file_buffer, sizeof(signed char), file_size, file);
    file_buffer[file_size] = '\n';

    /* shams' sample inputs have newlines in them */
    *n_chars = clear_whitespaces_do_mapping(file_buffer, file_size);
    return file_buffer;
}

int main ( int argc, char* argv[] ) {

  MPI_Init(&argc, &argv);
  MPI_Status status;
  MPI_Request request;

  int i, j, k, a;
	//int **score;
  int* score;

	if ( argc < 3 ) {
		fprintf(stderr, "Usage: %s fileName1 fileName2\n", argv[0]);
		exit(1);
	}

	char* file_name_1 = argv[1];
	char* file_name_2 = argv[2];

	FILE* file_1 = fopen(file_name_1, "r");
	if (!file_1) { fprintf(stderr, "could not open file %s\n",file_name_1); exit(1); }
	size_t n_char_in_file_1 = 0;
	sec_1 = read_file(file_1, &n_char_in_file_1);
	//fprintf(stdout, "Size of input string 1 is %u\n", (unsigned)n_char_in_file_1 );
	
	FILE* file_2 = fopen(file_name_2, "r");
	if (!file_2) { fprintf(stderr, "could not open file %s\n",file_name_2); exit(1); }
	size_t n_char_in_file_2 = 0;
	sec_2 = read_file(file_2, &n_char_in_file_2);
	//fprintf(stdout, "Size of input string 2 is %u\n", (unsigned)n_char_in_file_2 );
	

  score = (int*)malloc(2*(n_char_in_file_1+1)*sizeof(int));

  int num_procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int BLOCK_SIZE = n_char_in_file_1;

  for (i = 1; i <= (n_char_in_file_1); i++){
    score[i] = -i;
  }
  for (i = rank+1; i <= n_char_in_file_2; i += num_procs) {
    score[n_char_in_file_1+1] = -(n_char_in_file_1+1);
    for (j = 1; j <= n_char_in_file_1; j += BLOCK_SIZE) {
      if (i > 1) { 
        int sc[BLOCK_SIZE+1];
        MPI_Recv(sc, BLOCK_SIZE+1, MPI_INT, (rank+num_procs-1) % num_procs, 
          i*n_char_in_file_1 + j, MPI_COMM_WORLD, &status);

        memcpy(&score[j-1], sc, sizeof(int)*(BLOCK_SIZE+1));
      }

      for (k = j; k <= j+BLOCK_SIZE; k++) { // loop unroll 8 times

        int match = score[k-1] + sec_2[i] == sec_1[k]? 1 : -1;
        int insert = score[n_char_in_file_1+1 + k-1] - 1;
			  int deletenw = score[k] - 1;

			  int values[] = {match, insert, deletenw};
        int max = match;
        int temp = 1;
  			for (temp; temp < 3; temp++){
  				if (values[temp] > max){
  					max = values[temp];
  				}
  			}
  			
  			score[k] = max;

      }

      if (i != n_char_in_file_2) {
        int sc[BLOCK_SIZE+1];
        memcpy(sc, &score[n_char_in_file_1+1 + j-1], sizeof(int)*(BLOCK_SIZE+1));

        MPI_Send(sc, BLOCK_SIZE+1, MPI_INT, (rank + 1) % num_procs, 
          (i+1)*n_char_in_file_1 + j, MPI_COMM_WORLD);
      }
    }
  }



  if (rank == (n_char_in_file_2 + num_procs - 1) % num_procs) { // Only print score of the process that computed the last row of the score matrix
    fprintf(stdout, "score: %d\n", score[2*(n_char_in_file_1+1) - 1]);
  }
  

  free(sec_1);
  free(sec_2);
  free(score);
  MPI_Finalize();

  return 0;
}

