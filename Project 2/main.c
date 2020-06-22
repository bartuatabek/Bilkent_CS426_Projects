//*****************************
// Bartu Atabek
//  CS 426: Parallel Computing
// Project II
// Parallel program to estimate Pi
// using Monte Carlo Simulation
//*****************************
#include <mpi.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "helper.c"

/* Square function takes the square of the given number. */
float square(float number) {
  return (number * number);
}

/* Alternative method for generating points and returns square sums. */
float randSquareCoord(float number) {
  float x_coordinate = (float)drand48();
  float y_coordinate = (float)drand48();
  return square(x_coordinate) + square(y_coordinate);
}

/* Sum function sums the two numbers and returns the result. */
float sum(float number1, float number2) {
  return number1 + number2;
}

/* Checks whether the given value is inside the unit circle or not. */
bool isHit(float number) {
  if (number <= 1) {
    return true;
  }
  return false;
}

int main(int argc, char *argv[]) {
  int rank; /* Rank of the process */
  int proc; /* The worker processes */
  int num_proc; /* Total number of processes */
  int noOfExperiments; /* Total number of experiments */
  float hits;	/* The number of hits inside the circle */
  float pi; /* holds approx value of pi */
  float *x_coordinates; /* x values for the random coordinate */
  float *y_coordinates; /* y values for the random coordinate */
  float *z_values;
  double timeStart, timeEnd;
  time_t t;

  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (argc <= 1) {
    fprintf(stderr, "Usage: mpirun -np n %s the number of experiments.\n", argv[0]);
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  noOfExperiments = atoi(argv[1]);

  /* Generate the random points */
  srand48(((unsigned)time(&t)));
  x_coordinates = malloc(noOfExperiments * sizeof(float));
  y_coordinates = malloc(noOfExperiments * sizeof(float));
  z_values = malloc(noOfExperiments * sizeof(float));

  for (int i = 0; i < noOfExperiments; i++) {
    x_coordinates[i] = (float)drand48();
    y_coordinates[i] = (float)drand48();
  }

  timeStart = MPI_Wtime();
  float *square_x_coordinates = MPI_Map_Func(x_coordinates, noOfExperiments, square);
  float *square_y_coordinates = MPI_Map_Func(y_coordinates, noOfExperiments, square);

  MPI_Barrier(MPI_COMM_WORLD);
  for (int i = 0; i < noOfExperiments; i++) {
    z_values[i] = square_x_coordinates[i] + square_y_coordinates[i];
  }

  /* Alternative Implementation */
  // timeStart = MPI_Wtime();
  // float *coordinates = malloc(noOfExperiments * sizeof(float));
  // z_values = MPI_Map_Func(coordinates, noOfExperiments, randSquareCoord);

  float *hit_results = MPI_Filter_Func(z_values, noOfExperiments, isHit);
  MPI_Barrier(MPI_COMM_WORLD);
  hits = MPI_Fold_Func(hit_results, noOfExperiments, 0, sum);
  timeEnd = MPI_Wtime();

  if (rank == 0) {
    pi = (hits / (float)noOfExperiments) * 4.0;
    printf("The estimated value of π: %f with %d experiments.\n", pi, noOfExperiments);
    printf("Running time: %f seconds.\n", timeEnd - timeStart);
  }

  // Clean up
  free(x_coordinates);
  free(y_coordinates);
  free(z_values);
  MPI_Finalize();
  return 0;
}
