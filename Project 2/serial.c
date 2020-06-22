//*****************************
// Bartu Atabek
//  CS 426: Parallel Computing
// Project II
// Serial program to estimate Pi
// using Monte Carlo Simulation
//*****************************
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

typedef int bool;
enum { false, true };

/* Square function takes the square of the given number. */
float square(float number) {
  return (number * number);
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

int main(int argc, char* argv[]) {
	float z; /* Used to check if x^2+y^2<=1 */
	float pi; /* holds approx value of pi */
	float hits; /* The number of hits inside the circle */
	int noOfExperiments; /* Total number of experiments */
	float *x_coordinates; /* x values for the random coordinate */
  float *y_coordinates; /* y values for the random coordinate */
	time_t t;

	if (argc <= 1) {
		fprintf(stderr, "Usage: %s n (the number of experiments).\n", argv[0]);
		return 1;
	}

	hits = 0;
	noOfExperiments = atoi(argv[1]);

	/* Generate the random points */
	srand48(((unsigned)time(&t)));
	x_coordinates = malloc(noOfExperiments * sizeof(float));
	y_coordinates = malloc(noOfExperiments * sizeof(float));

	for (int i = 0; i < noOfExperiments; i++) {
		x_coordinates[i] = (float)drand48();
		y_coordinates[i] = (float)drand48();
	}

	clock_t start = clock();
	for (int i = 0; i <= noOfExperiments; i++) {
		z = sum(square(x_coordinates[i]), square(y_coordinates[i]));

		if (isHit(z) == true) {
			hits++;
		}
	}

	clock_t stop = clock();
	double elapsed = (double)(stop - start) / CLOCKS_PER_SEC;

	pi = (hits / (float)noOfExperiments) * 4.0;
	printf("The estimated value of π: %f with %d experiments.\n", pi, noOfExperiments);
	printf("Running time: %f seconds.\n", elapsed);

	// Clean up
	free(x_coordinates);
	free(y_coordinates);
	return 0;
}
