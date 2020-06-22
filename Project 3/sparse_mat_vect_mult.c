//*****************************
// Bartu Atabek 21602229
// CS 426: Parallel Computing
// Project III
//*****************************

#include <omp.h>
#include <stdio.h>
#include <stdlib.h>

void read_matrix(int **r_ptr, int** c_ind, float** v, char* filename, int* r_count, int* v_count) {
	FILE* file;
	if ((file = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Error: failed to open file %s\n", filename);
		return;
	}

	int row_count;
	int column_count;
	int values_count;
	fscanf(file, "%d %d %d\n", &row_count, &column_count, &values_count);
	*r_count = row_count;
	*v_count = values_count;

	int* row_ptr = malloc(row_count * sizeof(int));
	int* col_ind = malloc(values_count * sizeof(int));
	for (int i = 0; i < values_count; i++) {
		col_ind[i] = -1;
	}

	int row;
	int column;
	float value;
	float* values = malloc(values_count * sizeof(float));
	/* Fill the row_ptr data */
	while(1) {
		int noOfItems = fscanf(file, "%d %d %f\n", &row, &column, &value);
		column--;
		row--;
		if (noOfItems == 3) {
			// If its a non-zero element increase index number
			row_ptr[row]++;
		} else if (noOfItems == EOF) {
			// If end of file break loop
			break;
		} else {
			// If either row | column | value is empty in data
			printf("No match.\n");
		}
	}
	rewind(file);

	int index = 0;
	int val = 0;
	for (int i = 0; i < row_count; i++) {
		val = row_ptr[i];
		row_ptr[i] = index;
		index += val;
	}

	fscanf(file, "%d %d %d\n", &row_count, &column_count, &values_count);
	int i = 0;
	/* Fill the col_ind & values data */
	while (1) {
		int noOfItems = fscanf(file, "%d %d %f\n", &row, &column, &value);
		column--;
		row--;
		if (noOfItems == 3) {
			while (col_ind[i+row_ptr[row]] != -1) {
				i++;
			}
			col_ind[i+row_ptr[row]] = column;
			values[i+row_ptr[row]] = value;
			i = 0;
		} else if (noOfItems == EOF) {
			// If end of file break loop
			break;
		} else {
			// If either row | column | value is empty in data
			printf("No match.\n");
		}
	}

	fclose(file);
	*r_ptr = row_ptr;
	*c_ind = col_ind;
	*v = values;
}

void printInitialData(int* row_ptr, int* col_ind, float* values, float* x, int r_count, int v_count) {
	fprintf(stdout, "=====Initial Matrix=====\n");
	fprintf(stdout, "[Row] [Column] [Non-zero-Value-At-A[row][column]]\n");

	for (int i = 0; i < r_count; i++) {
		if (i + 1 < r_count) {
			for (int k = row_ptr[i]; k < row_ptr[i+1]; k++) {
				fprintf(stdout, "[%d][%d]: %e\n", i + 1, col_ind[k] + 1, values[k]);
			}
		} else {
			for (int k = row_ptr[i]; k < v_count; k++) {
				fprintf(stdout, "[%d][%d]: %e\n", i + 1, col_ind[k] + 1, values[k]);
			}
		}
	}

	fprintf(stdout, "\n=====Initial Vector=====\n");
	for (int i = 0; i < r_count; i++) {
		fprintf(stdout, "[%d]: %f\n", i, x[i]);
	}
}

void printFinalVector(float* x, int r_count) {
	fprintf(stdout, "\n=====Resulting Vector=====\n");
	for (int i = 0; i < r_count; i++) {
		fprintf(stdout, "[%d]: %f\n", i, x[i]);
	}
}

int main(int argc, char *argv[]) {
	if (argc != 5) {
		fprintf(stderr, "Usage: ./exe no_of_threads no_of_repetitions 1|0 “filename”.\n");
		return -1;
	}

	/* For each row i of the sparse matrix, row_ptr[i] contains an index number
	* associated with the first nonzero element in this row. This index can be
	* used in col_ind and values arrays (see below). If row i does not contain
	* a non-zero element (i.e. it is all zeros), then rowptr[i]==rowptr[i+1].
	*/
	int* row_ptr;

	/* This array contains the column indices of the non-zero elements. If the
	* matrix element at row i & column j is a non-zero element, then
	* col_ind[k]==j for some k such that row_ptr[i] <= k < row_ptr[i+1].
	*/
	int* col_ind;

	/* This array contains the values of non-zero elements. This array is indexed
	* similar to the col_ind array. If the matrix element at row i column j has
	* the non-zero value v, then for some k such that rowptr[i] <= k < rowptr[i+1],
	* you have values[k]==v.
	*/
	float* values;

	/* The number of threads used to compute Matrix-vector product */
	int thread_num = atoi(argv[1]);
	/* The number of repetitions */
	int repetitions = atoi(argv[2]);
	/* An argument to print on stdout */
	int shouldPrint = atoi(argv[3]);
	/* Test-file name */
	char* filename = argv[4];
	/* Row count */
	int r_count;
	/* Values count */
	int v_count;
	/* Timing variables */
	double timeStart, timeEnd;

	read_matrix(&row_ptr, &col_ind, &values, filename, &r_count, &v_count);

	/* Used to store the vector. Initialize x to all 1s before the Matrix-vector
	 * product iterations begin. */
	float* x = malloc(r_count * sizeof(float));
	for (int i = 0; i < r_count; i++) {
		x[i] = 1.0;
	}

	/* Print the initial matrix & vector */
	if (shouldPrint == 1) {
		printInitialData(row_ptr, col_ind, values, x, r_count, v_count);
	}

	omp_set_num_threads(thread_num);
	int k,i; /* Private vars for each thread */
	float temp; /* Private var for each thread */
	float y[r_count]; /* Array storing the results */

	timeStart = omp_get_wtime();
	for (int r = 0; r < repetitions; r++) {
		#pragma omp parallel for private(i, k, temp)
		for (i = 0; i < r_count; i++) {
			temp = 0.0;
			if (i + 1 < r_count) {
				for (k = row_ptr[i]; k < row_ptr[i+1]; k++) {
					#pragma omp atomic
					temp += (float)(values[k] * x[col_ind[k]]);
				}
			} else {
				for (k = row_ptr[i]; k < v_count; k++) {
					#pragma omp atomic
					temp += (float)(values[k] * x[col_ind[k]]);
				}
			}
			y[i] = temp;
		}

		#pragma omp barrier
		for (i = 0; i < r_count; i++) {
			x[i] = y[i];
		}
	}
	timeEnd = omp_get_wtime();


	/* Print the resulting vector after all the
	 * xi+1=Axi iterations have completed.
	 */
	if (shouldPrint == 1) {
		printFinalVector(x, r_count);
	}

	/* Print the timing results */
	printf("Running time: %lf seconds.\n", timeEnd - timeStart);

	// Clean up
	free(row_ptr);
	free(col_ind);
	free(values);
  free(x);
  return 0;
}
