//*****************************
// Bartu Atabek
//  CS 426: Parallel Computing
// Project II
//*****************************
#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

typedef int bool;
enum { false, true };

/*
* This function takes a float array, size of the array and a function pointer
* to a function who takes a float and returns a float. It will apply that
* function to each ellement of the array.
*/
float *MPI_Map_Func(float *arr, int size, float (*func)(float)) {
  int rank; /* Rank of the process */
  int proc; /* The worker processes */
  int num_proc; /* Total number of processes */
  int quotient; /* Size of subarray: size/num_proc */
  int rem; /* Number of larger subarrays: size % num_proc */
  int *sendcounts; /* Array specifying the number of elements to send to each processor */
  int *displs; /* Specifies the displacements */
  float *localdata;  /* The local array for each process */
  float *resultarr;  /* The resulting array */

  /* Usual startup tasks */
  MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    resultarr = malloc(sizeof(float) * size);
  }

  /* Get values needed for chunk sizes */
  quotient = size / num_proc;
  rem = size % num_proc;

  sendcounts = malloc(sizeof(int) * num_proc);
  displs = malloc(sizeof(int) * num_proc);

  for (proc = 0; proc < num_proc; proc++) {
    if (proc == num_proc-1 && rem != 0) {
      sendcounts[proc] = quotient + rem;
      displs[proc] = proc * quotient;
      continue;
    }
    sendcounts[proc] = quotient;
    displs[proc] = proc * sendcounts[proc];
  }

  localdata = malloc(sizeof(float) * sendcounts[rank]);
  MPI_Scatterv(arr, sendcounts, displs, MPI_FLOAT, localdata, sendcounts[rank], MPI_FLOAT, 0, MPI_COMM_WORLD);

  for (int i = 0; i < sendcounts[rank]; i++) {
    localdata[i] = func(localdata[i]);
  }

  MPI_Gatherv(localdata, sendcounts[rank], MPI_FLOAT, resultarr, sendcounts, displs, MPI_FLOAT, 0, MPI_COMM_WORLD);

  // Clean up
  free(localdata);
  free(sendcounts);
  free(displs);
  return resultarr;
}

/*
* This function will take again an array, its size, initial value and a
* function pointer again. The function pointer should point to a function
* that takes two floats and returns a float. İnitial values is the value
* that starts the accumulator.
*/
float MPI_Fold_Func(float *arr, int size, float initial_value, float (*func)(float, float)) {
  int rank; /* Rank of the process */
  int proc; /* The worker processes */
  int num_proc; /* Total number of processes */
  int quotient; /* Size of subarray: size/num_proc */
  int rem; /* Number of larger subarrays: size % num_proc */
  int *sendcounts; /* Array specifying the number of elements to send to each processor */
  int *recvcounts; /*  Array containing the number of elements that are received from each process */
  int *displs; /* Specifies the displacements */
  float *localdata;  /* The local array for each process */
  float *resultarr;  /* The resulting array */
  float localresult;  /* The local result */
  float result;  /* The result */

  /* Usual startup tasks */
  MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    resultarr = malloc(sizeof(float) * num_proc);
  }

  /* Get values needed for chunk sizes */
  quotient = size / num_proc;
  rem = size % num_proc;

  sendcounts = malloc(sizeof(int) * num_proc);
  recvcounts = malloc(sizeof(int) * num_proc);
  displs = malloc(sizeof(int) * num_proc);

  for (proc = 0; proc < num_proc; proc++) {
    if (proc == num_proc-1 && rem != 0) {
      sendcounts[proc] = quotient + rem;
      displs[proc] = proc * quotient;
      recvcounts[proc] = 1;
      continue;
    }
    recvcounts[proc] = 1;
    sendcounts[proc] = quotient;
    displs[proc] = proc * sendcounts[proc];
  }

  localdata = malloc(sizeof(float) * sendcounts[rank]);
  MPI_Scatterv(arr, sendcounts, displs, MPI_FLOAT, localdata, sendcounts[rank], MPI_FLOAT, 0, MPI_COMM_WORLD);

  localresult = initial_value;
  for (int i = 0; i < sendcounts[rank]; i++) {
    localresult = func(localresult, localdata[i]);
  }

  MPI_Gather(&localresult, 1, MPI_FLOAT, resultarr, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    result = initial_value;
    for(int i = 0; i < num_proc; i++) {
      result = func(result, resultarr[i]);
    }
    free(resultarr);
  }

  // Clean up
  free(localdata);
  free(sendcounts);
  free(recvcounts);
  free(displs);
  return result;
}

/*
* This is similar to Map version but the function is now a predicate which
* takes a float and returns a boolean.
*/
float *MPI_Filter_Func(float *arr, int size, bool (*pred)(float)) {
  int rank; /* Rank of the process */
  int proc; /* The worker processes */
  int num_proc; /* Total number of processes */
  int quotient; /* Size of subarray: size/num_proc */
  int rem; /* Number of larger subarrays: size % num_proc */
  int *sendcounts; /* Array specifying the number of elements to send to each processor */
  int *displs; /* Specifies the displacements */
  float *localdata;  /* The local array for each process */
  float *resultarr;  /* The resulting array */

  /* Usual startup tasks */
  MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    resultarr = malloc(sizeof(float) * size);
  }

  /* Get values needed for chunk sizes */
  quotient = size / num_proc;
  rem = size % num_proc;

  sendcounts = malloc(sizeof(int) * num_proc);
  displs = malloc(sizeof(int) * num_proc);

  for (proc = 0; proc < num_proc; proc++) {
    if (proc == num_proc-1 && rem != 0) {
      sendcounts[proc] = quotient + rem;
      displs[proc] = proc * quotient;
      continue;
    }
    sendcounts[proc] = quotient;
    displs[proc] = proc * sendcounts[proc];
  }

  localdata = malloc(sizeof(float) * sendcounts[rank]);
  MPI_Scatterv(arr, sendcounts, displs, MPI_FLOAT, localdata, sendcounts[rank], MPI_FLOAT, 0, MPI_COMM_WORLD);

  for (int i = 0; i < sendcounts[rank]; i++) {
    localdata[i] = pred(localdata[i]);
  }

  MPI_Gatherv(localdata, sendcounts[rank], MPI_FLOAT, resultarr, sendcounts, displs, MPI_FLOAT, 0, MPI_COMM_WORLD);

  // Clean up
  free(localdata);
  free(sendcounts);
  free(displs);
  return resultarr;
}
