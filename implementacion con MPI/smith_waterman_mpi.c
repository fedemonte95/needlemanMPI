#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include <string.h>

#include <mpi.h>

#define GAP_PENALTY -1
#define TRANSITION_PENALTY -2
#define TRANSVERSION_PENALTY -4
#define MATCH 2

//#define BLOCK_SIZE 1000

signed char* string_1;
signed char* string_2;

enum Nucleotide {GAP=0, ADENINE, CYTOSINE, GUANINE, THYMINE};

signed char char_mapping ( char c ) {
    signed char to_be_returned = -1;
    switch(c) {
        case '_': to_be_returned = GAP; break;
        case 'A': to_be_returned = ADENINE; break;
        case 'C': to_be_returned = CYTOSINE; break;
        case 'G': to_be_returned = GUANINE; break;
        case 'T': to_be_returned = THYMINE; break;
    }
    return to_be_returned;
}

void print_matrix ( int** matrix, int n_rows, int n_columns ) {
    int i, j;
    for ( i = 0; i < n_rows; ++i ) {
        for ( j = 0; j < n_columns; ++j ) {
            fprintf(stdout, "%d ", matrix[i][j]);
        }
        fprintf(stdout, "\n");
    }
    fprintf(stdout,"--------------------------------\n");
}

static char alignment_score_matrix[5][5] =
{
    {GAP_PENALTY,GAP_PENALTY,GAP_PENALTY,GAP_PENALTY,GAP_PENALTY},
    {GAP_PENALTY,MATCH,TRANSVERSION_PENALTY,TRANSITION_PENALTY,TRANSVERSION_PENALTY},
    {GAP_PENALTY,TRANSVERSION_PENALTY, MATCH,TRANSVERSION_PENALTY,TRANSITION_PENALTY},
    {GAP_PENALTY,TRANSITION_PENALTY,TRANSVERSION_PENALTY, MATCH,TRANSVERSION_PENALTY},
    {GAP_PENALTY,TRANSVERSION_PENALTY,TRANSITION_PENALTY,TRANSVERSION_PENALTY, MATCH}
};

size_t clear_whitespaces_do_mapping ( signed char* buffer, long size ) {
    size_t non_ws_index = 0, traverse_index = 0;

    while ( traverse_index < size ) {
        char curr_char = buffer[traverse_index];
        switch ( curr_char ) {
            case 'A': case 'C': case 'G': case 'T':
                /*this used to be a copy not also does mapping*/
                buffer[non_ws_index++] = char_mapping(curr_char);
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
	string_1 = read_file(file_1, &n_char_in_file_1);
	//fprintf(stdout, "Size of input string 1 is %u\n", (unsigned)n_char_in_file_1 );
	
	FILE* file_2 = fopen(file_name_2, "r");
	if (!file_2) { fprintf(stderr, "could not open file %s\n",file_name_2); exit(1); }
	size_t n_char_in_file_2 = 0;
	string_2 = read_file(file_2, &n_char_in_file_2);
	//fprintf(stdout, "Size of input string 2 is %u\n", (unsigned)n_char_in_file_2 );
	
	int BLOCK_SIZE = n_char_in_file_1 >= 1000 ? 1000 : n_char_in_file_1/2;

  score = (int*)malloc(2*(n_char_in_file_1+1)*sizeof(int));

  int num_procs, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  
  struct timeval begin,end;
  gettimeofday(&begin,0);

  for (i = 0; i < (n_char_in_file_1+1); i++)
    score[i] = GAP_PENALTY*i;
  score[n_char_in_file_1+1] = GAP_PENALTY*(rank+1);

  for (i = rank+1; i <= n_char_in_file_2; i += num_procs) {
    score[n_char_in_file_1+1] = GAP_PENALTY*(i);
    for (j = 1; j <= n_char_in_file_1; j += BLOCK_SIZE) {
      if (i > 1) { 
        int sc[BLOCK_SIZE+1];
        MPI_Recv(sc, BLOCK_SIZE+1, MPI_INT, (rank+num_procs-1) % num_procs, 
          i*n_char_in_file_1 + j, MPI_COMM_WORLD, &status);

        memcpy(&score[j-1], sc, sizeof(int)*(BLOCK_SIZE+1));
      }

      for (k = j; k < j+BLOCK_SIZE && k < n_char_in_file_1+1; ) { // loop unroll 8 times
        signed char char_1 = string_1[k-1];
        signed char char_2 = string_2[i-1];

        int diag_score = score[k-1] + alignment_score_matrix[char_2][char_1];
        int left_score = score[n_char_in_file_1+1 + k-1] + alignment_score_matrix[char_1][GAP];
        int  top_score = score[k  ] + alignment_score_matrix[GAP][char_2];

        int bigger_of_left_top = (left_score > top_score) ? left_score : top_score;
        score[n_char_in_file_1+1 + k] = (bigger_of_left_top > diag_score) ? bigger_of_left_top : diag_score;

        /**/
        ++k;
        char_1 = string_1[k-1];
        char_2 = string_2[i-1];
        diag_score = score[k-1] + alignment_score_matrix[char_2][char_1];
        left_score = score[n_char_in_file_1+1 + k-1] + alignment_score_matrix[char_1][GAP];
        top_score = score[k  ] + alignment_score_matrix[GAP][char_2];
        bigger_of_left_top = (left_score > top_score) ? left_score : top_score;
        score[n_char_in_file_1+1 + k] = (bigger_of_left_top > diag_score) ? bigger_of_left_top : diag_score;
        
        ++k;
        char_1 = string_1[k-1];
        char_2 = string_2[i-1];
        diag_score = score[k-1] + alignment_score_matrix[char_2][char_1];
        left_score = score[n_char_in_file_1+1 + k-1] + alignment_score_matrix[char_1][GAP];
        top_score = score[k  ] + alignment_score_matrix[GAP][char_2];
        bigger_of_left_top = (left_score > top_score) ? left_score : top_score;
        score[n_char_in_file_1+1 + k] = (bigger_of_left_top > diag_score) ? bigger_of_left_top : diag_score;

        ++k;
        char_1 = string_1[k-1];
        char_2 = string_2[i-1];
        diag_score = score[k-1] + alignment_score_matrix[char_2][char_1];
        left_score = score[n_char_in_file_1+1 + k-1] + alignment_score_matrix[char_1][GAP];
        top_score = score[k  ] + alignment_score_matrix[GAP][char_2];
        bigger_of_left_top = (left_score > top_score) ? left_score : top_score;
        score[n_char_in_file_1+1 + k] = (bigger_of_left_top > diag_score) ? bigger_of_left_top : diag_score;
        
        ++k;
        char_1 = string_1[k-1];
        char_2 = string_2[i-1];
        diag_score = score[k-1] + alignment_score_matrix[char_2][char_1];
        left_score = score[n_char_in_file_1+1 + k-1] + alignment_score_matrix[char_1][GAP];
        top_score = score[k  ] + alignment_score_matrix[GAP][char_2];
        bigger_of_left_top = (left_score > top_score) ? left_score : top_score;
        score[n_char_in_file_1+1 + k] = (bigger_of_left_top > diag_score) ? bigger_of_left_top : diag_score;

        ++k;
        char_1 = string_1[k-1];
        char_2 = string_2[i-1];
        diag_score = score[k-1] + alignment_score_matrix[char_2][char_1];
        left_score = score[n_char_in_file_1+1 + k-1] + alignment_score_matrix[char_1][GAP];
        top_score = score[k  ] + alignment_score_matrix[GAP][char_2];
        bigger_of_left_top = (left_score > top_score) ? left_score : top_score;
        score[n_char_in_file_1+1 + k] = (bigger_of_left_top > diag_score) ? bigger_of_left_top : diag_score;
        
        ++k;
        char_1 = string_1[k-1];
        char_2 = string_2[i-1];
        diag_score = score[k-1] + alignment_score_matrix[char_2][char_1];
        left_score = score[n_char_in_file_1+1 + k-1] + alignment_score_matrix[char_1][GAP];
        top_score = score[k  ] + alignment_score_matrix[GAP][char_2];
        bigger_of_left_top = (left_score > top_score) ? left_score : top_score;
        score[n_char_in_file_1+1 + k] = (bigger_of_left_top > diag_score) ? bigger_of_left_top : diag_score;

        ++k;
        char_1 = string_1[k-1];
        char_2 = string_2[i-1];
        diag_score = score[k-1] + alignment_score_matrix[char_2][char_1];
        left_score = score[n_char_in_file_1+1 + k-1] + alignment_score_matrix[char_1][GAP];
        top_score = score[k  ] + alignment_score_matrix[GAP][char_2];
        bigger_of_left_top = (left_score > top_score) ? left_score : top_score;
        score[n_char_in_file_1+1 + k] = (bigger_of_left_top > diag_score) ? bigger_of_left_top : diag_score;

        ++k;
        /**/
      }

      if (i != n_char_in_file_2) {
        int sc[BLOCK_SIZE+1];
        memcpy(sc, &score[n_char_in_file_1+1 + j-1], sizeof(int)*(BLOCK_SIZE+1));

        MPI_Send(sc, BLOCK_SIZE+1, MPI_INT, (rank + 1) % num_procs, 
          (i+1)*n_char_in_file_1 + j, MPI_COMM_WORLD);
      }
    }
  }


  gettimeofday(&end,0);

  if (rank == (n_char_in_file_2 + num_procs - 1) % num_procs) { // Only print score of the process that computed the last row of the score matrix
    fprintf(stdout, "The computation took %f seconds\n",
      ((end.tv_sec - begin.tv_sec)*1000000+(end.tv_usec - begin.tv_usec))*1.0/1000000);
    fprintf(stdout, "score: %d\n", score[2*(n_char_in_file_1+1) - 1]);
  }

  free(string_1);
  free(string_2);
  free(score);
  MPI_Finalize();

  /*
  fprintf(stdout, "The computation took %f seconds\n",
    ((end.tv_sec - begin.tv_sec)*1000000+(end.tv_usec - begin.tv_usec))*1.0/1000000);
	//fprintf(stdout, "score: %d\n", score[n_char_in_file_2][n_char_in_file_1]);
	fprintf(stdout, "[%d] score: %d\n", rank, score[2*(n_char_in_file_1+1) - 1]);
  */
  return 0;
}

