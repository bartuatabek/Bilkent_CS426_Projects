/* Bartu Atabek
* 21602229
* CS 426 Project #1
*/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>

/* Appends new element to dynamic array */
void append(int **arr, int *size, int value) {
  *arr = realloc(*arr, (*size + 1) * sizeof(int));
  (*arr)[*size] = value;
  (*size)++;
}

/* Search for min element */
int find_min(int *arr, int size) {
  int min = arr[0];
  for (int i = 1; i < size; i++) {
    if (arr[i] < min) {
      min = arr[i];
    }
  }
  return min;
}

int main(int argc, char *argv[]) {
  int rank; /* Rank of the process */
  int proc; /* The worker processes */
  int num_proc; /* Total number of processes */
  int array_len; /* Length of the main array */
  int quotient; /* Size of subarray: array_len/num_proc*/
  int rem; /* Number of larger subarrays: array_len % num_proc */
  int sub_start;  /* Start of one of the subarrays */
  int sub_len;  /* Length of my subarray */
  int global_min; /* Minimum for the main array */
  int local_min; /* Local min from one process */
  int *search_array = NULL; /* The array to search */
  MPI_Status status; /* status for receive */

  double timeStart, timeEnd;

  /* Usual startup tasks */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /* Code for Process 0 */
  if (rank == 0) {
    if (argc <= 1) {
      fprintf(stderr, "Usage: %s filepath\n", argv[0]);
      return 1;
    }

    FILE *inputFile = fopen(argv[1], "r");

    if (inputFile == 0) {
      fprintf(stderr, "%s: failed to open file %s\n", argv[0], argv[1]);
      return 1;
    }

    char *input = NULL;
    size_t len = 0;
    ssize_t read;

    array_len = 0;
    search_array = malloc(sizeof(int) * array_len);

    while ((read = getline(&input, &len, inputFile)) != -1) {
      append(&search_array, &array_len, atoi(input));
    }

    fclose(inputFile);
    timeStart = MPI_Wtime();
    
    /* Get values needed for subarray sizes */
    quotient = array_len / num_proc;
    rem = array_len % num_proc;

    /* Some subarrays may be larger */
    for (proc = 1; proc < rem; proc++) {
      sub_len = quotient + 1;
      sub_start = proc * quotient + proc;
      MPI_Send(&sub_len, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
      MPI_Send(&(search_array[sub_start]), sub_len, MPI_INT, proc, 0, MPI_COMM_WORLD);
    }

    int proc_no = rem;
    if (rem == 0) {
      proc_no++;
    }

    for (proc = proc_no; proc < num_proc; proc++) {
      sub_len = quotient;
      sub_start = proc * quotient + rem;
      MPI_Send(&sub_len, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
      MPI_Send(&(search_array[sub_start]), sub_len, MPI_INT, proc, 0, MPI_COMM_WORLD);
    }

    if (rem == 0) {
      sub_len = quotient;
    } else {
      sub_len = (quotient + 1);
    }

    /* Find local min */
    local_min = find_min(search_array, sub_len);
    printf("Min for P%d is %d.\n", rank, local_min);

  } else {
    MPI_Recv(&sub_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    search_array = malloc(sub_len * sizeof(int));
    MPI_Recv(search_array, sub_len, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

    local_min = find_min(search_array, sub_len);
    printf("Min for P%d is %d.\n", rank, local_min);
  }

  /* All processes have the computed overall min */
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Allreduce(&local_min, &global_min, 1, MPI_FLOAT, MPI_MIN, MPI_COMM_WORLD);

  printf("Global Min for P%d is %d.\n", rank, global_min);
  timeEnd = MPI_Wtime();

  // Clean up
  free(search_array);
  MPI_Finalize();

  if (rank == 0) {
    printf("Running time = %f seconds.\n", (timeEnd - timeStart));
  }
  return 0;
}
