//*****************************
// Bartu Atabek 21602229
// CS 426: Parallel Computing
// Project IV
//*****************************

// #define imin(a,b) (a<b?a:b)
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// CUDA runtime
#include <cuda_runtime.h>

/* Array generator code which generates a random integer array with N elements. */
int* arrayGenerator(int N) {
	if (N <= 0) {
		return NULL;
	}

	int* vector = (int*) malloc(N * sizeof(int));
	for (int i = 0; i < N; i++) {
		vector[i] = rand() % 100;
	}
	return vector;
}

/* Reads the first line of the input file and returns it as N. */
int getArraySize(char* filename) {
	FILE* file;
	int numOfArraySize = 0;

	if ((file = fopen(filename, "r")) == NULL) {
		fprintf(stderr, "Error: failed to open file %s\n", filename);
		return numOfArraySize;
	}

	fscanf(file, "%d", &numOfArraySize);
	fclose(file);
	return numOfArraySize;
}

/* Reads the inputted file and returns input vectors. */
void readFile(char* filename, int arrSize, int** vector1, int** vector2) {
	int a = 0;
	FILE* file = fopen(filename, "r");
	fscanf(file, "%d", &a);

	int x = 0;
	int i = 0;
	int j = 0;
	while (!feof(file)) {
		fscanf(file, "%d", &x);
		if (i < arrSize) {
			(*vector1)[i] = x;
		}
		if (i >= arrSize && i < 2 * arrSize) {
			(*vector2)[j] = x;
			j++;
		}
		i++;
	}
	fclose(file);
}

/* Finds the angle between two vectors. */
double findAngle(int N, int* vector1, int* vector2) {
	float nominator = 0;
	double length1 = 0;
	double length2 = 0;
	double denominator = 0;
	double result = 0;
	double value = 180.0 / M_PI;

	for (int i = 0; i < N; i++) {
		nominator += vector1[i] * vector2[i];
	}

	for (int i = 0; i < N; i++) {
		length1 += pow(vector1[i], 2);
		length2 += pow(vector2[i], 2);
	}
	length1 = sqrt(length1);
	length2 = sqrt(length2);
	denominator = length1 * length2;

	result = nominator / denominator;
	result = atan(result) * value;
	return result;
}

__global__ void compute(int N, int threadsPerBlock, int *d_vector1, int *d_vector2, float *d_vector3) {
	extern __shared__ float temp[];
	int index = threadIdx.x + blockIdx.x * blockDim.x;

	// Calculate the nominator using dot product of two vectors
	temp[threadIdx.x] = d_vector1[index] * d_vector2[index];

	// Synchronize threads
	__syncthreads();

	// Accumulate the results
	if (0 == threadIdx.x) {
		int sum = 0;
		for (int i = 0; i < threadsPerBlock; i++)
			sum += temp[i];
		atomicAdd(d_vector3, sum);
		__syncthreads();
	}
	__syncthreads();

	// Calculate the denominator using ||v1|| * ||v2||
	// Calculate the sqrt of first vector
	temp[threadIdx.x] = powf(d_vector1[index], 2);

	// Synchronize threads
	__syncthreads();

	// Accumulate the results
	if (0 == threadIdx.x) {
		int sum = 0;
		for (int i = 0; i < threadsPerBlock; i++)
			sum += temp[i];
		atomicAdd(d_vector3 + 1, sum);
		__syncthreads();
	}
	__syncthreads();

	// Calculate the sqrt of second vector
	temp[threadIdx.x] = powf(d_vector2[index], 2);

	// Synchronize threads
	__syncthreads();

	// Accumulate the results
	if (0 == threadIdx.x) {
		int sum = 0;
		for (int i = 0; i < threadsPerBlock; i++)
			sum += temp[i];
		atomicAdd(d_vector3 + 2, sum);
		__syncthreads();
	}
	__syncthreads();
}

int main(int argc, char **argv) {
	// Info
	int N;
	int threadsPerBlock;
	int blocksPerGrid;

	// Results
	double CPU_result, GPU_result;

	// Time measure properties for CPU
	clock_t start, end;
	double time_for_arr_gen, time_for_cpu_func, time_for_host_to_device, time_for_device_to_host, time_for_kernel_exe;

	// Input/ouput vectors of the host and kernel
	int *vector1, *vector2, *d_vector1, *d_vector2;
	float *output, *d_output;

	if (argc == 3) {
		// To measure time for GPU
		cudaEvent_t start_gpu, stop_gpu;
		cudaEventCreate(&start_gpu);
		cudaEventCreate(&stop_gpu);

		N = atoi(argv[1]);
		threadsPerBlock = atoi(argv[2]);
		blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;

		// Initialize time t for pseudo random numbers
		time_t t;

		// Initialize input vectors and output
		start = clock();
		srand((unsigned) time(&t));
		vector1 = arrayGenerator(N);
		vector2 = arrayGenerator(N);
		output = (float*) malloc(3 * sizeof(float));
		output[0] = 0; output[1] = 0; output[2] = 0;

		// Memory allocation for device members
		cudaMalloc((void**)&d_vector1, N * sizeof(int));
		cudaMalloc((void**)&d_vector2, N * sizeof(int));
		cudaMalloc((void**)&d_output, 3 * sizeof(float));
		end = clock();
		time_for_arr_gen = ((double)(end - start)) / CLOCKS_PER_SEC;

		// Host to device transfer
		start = clock();
		cudaMemcpy(d_vector1, vector1, N * sizeof(int), cudaMemcpyHostToDevice);
		cudaMemcpy(d_vector2, vector2, N * sizeof(int), cudaMemcpyHostToDevice);
		end = clock();
		time_for_host_to_device = ((double)(end - start)) / CLOCKS_PER_SEC;

		// Run host function
		start = clock();
		CPU_result = findAngle(N, vector1, vector2);
		end = clock();
		time_for_cpu_func = ((double)(end - start)) / CLOCKS_PER_SEC;

		// Run kernel function
		start = clock();
		compute<<< blocksPerGrid, threadsPerBlock, (threadsPerBlock * sizeof(float)) >>>(N, threadsPerBlock, d_vector1, d_vector2, d_output);

		cudaDeviceSynchronize();
		end = clock();
		time_for_kernel_exe = ((double)(end - start)) / CLOCKS_PER_SEC;

		// Device to host transfer
		start = clock();
		cudaMemcpy(output, d_output, 3 * sizeof(float), cudaMemcpyDeviceToHost);
		cudaDeviceSynchronize();
		end = clock();
		time_for_device_to_host = ((double)(end - start)) / CLOCKS_PER_SEC;

		output[1] = sqrt(output[1]);
		output[2] = sqrt(output[2]);
		float nominator = output[0];
		float denominator = output[1] * output[2];
		GPU_result = nominator / denominator;
		double value = 180.0 / M_PI;
		GPU_result = atan(GPU_result) * value;
	}
	else if (argc == 4) {
		// To measure time for GPU
		cudaEvent_t start_gpu, stop_gpu;
		cudaEventCreate(&start_gpu);
		cudaEventCreate(&stop_gpu);

		// Read filename & get size of input
		char *filename = argv[3];
		int numOfArraySize = getArraySize(filename);
		N = numOfArraySize;
		threadsPerBlock = atoi(argv[2]);
		blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;

		// Initialize input vectors and output
		start = clock();
		vector1 = (int*) malloc(N * sizeof(int));
		vector2 = (int*) malloc(N * sizeof(int));
		output = (float*) malloc(3 * sizeof(float));
		readFile(filename, numOfArraySize, &vector1, &vector2);
		output[0] = 0; output[1] = 0; output[2] = 0;

		// Memory allocation for device members
		cudaMalloc((void**)&d_vector1, N * sizeof(int));
		cudaMalloc((void**)&d_vector2, N * sizeof(int));
		cudaMalloc((void**)&d_output, 3 * sizeof(float));
		end = clock();
		time_for_arr_gen = ((double)(end - start)) / CLOCKS_PER_SEC;

		// Host to device transfer
		start = clock();
		cudaMemcpy(d_vector1, vector1, N * sizeof(int), cudaMemcpyHostToDevice);
		cudaMemcpy(d_vector2, vector2, N * sizeof(int), cudaMemcpyHostToDevice);
		end = clock();
		time_for_host_to_device = ((double)(end - start)) / CLOCKS_PER_SEC;

		// Run host function
		start = clock();
		CPU_result = findAngle(N, vector1, vector2);
		end = clock();
		time_for_cpu_func = ((double)(end - start)) / CLOCKS_PER_SEC;

		// Run kernel function
		start = clock();
		compute<<< blocksPerGrid, threadsPerBlock, (threadsPerBlock * sizeof(float)) >>>(N, threadsPerBlock, d_vector1, d_vector2, d_output);

		cudaDeviceSynchronize();
		end = clock();
		time_for_kernel_exe = ((double)(end - start)) / CLOCKS_PER_SEC;

		// Device to host transfer
		start = clock();
		cudaMemcpy(output, d_output, 3 * sizeof(float), cudaMemcpyDeviceToHost);
		cudaDeviceSynchronize();
		end = clock();
		time_for_device_to_host = ((double)(end - start)) / CLOCKS_PER_SEC;

		output[1] = sqrt(output[1]);
		output[2] = sqrt(output[2]);
		float nominator = output[0];
		float denominator = output[1] * output[2];
		GPU_result = nominator / denominator;
		double value = 180.0 / M_PI;
		GPU_result = atan(GPU_result) * value;
	}
	else {
		fprintf(stderr, "Usage: ./a N threadsPerBlock (optional) filename.txt.\n");
		return -1;
	}

	// Display results
	printf("Info \n");
	printf("--------------------\n");
	printf("Number of Elements: %d \n", N);
	printf("Number of threads per block: %d \n", threadsPerBlock);
	printf("Number of blocks will be created: %d \n", (N + threadsPerBlock - 1) / threadsPerBlock);

	printf("Time \n");
	printf("--------------------\n");
	printf("Time for the array generation: %f ms \n", time_for_arr_gen);
	printf("Time for the CPU function: %f ms \n", time_for_cpu_func);
	printf("Time for the Host to Device transfer: %f ms \n", time_for_host_to_device / 1000);
	printf("Time for the kernel execution: %f ms \n", time_for_kernel_exe / 1000);
	printf("Time for the Device to Host transfer: %f ms \n", time_for_device_to_host / 1000);
	printf("Total execution time for GPU: %f ms \n", (time_for_host_to_device + time_for_kernel_exe) / 1000);

	printf("Results \n");
	printf("--------------------\n");
	printf("CPU result: %.3f \n", CPU_result);
	printf("GPU result: %.3f \n", GPU_result);

	free(vector1);
	free(vector2);
	free(output);
	cudaFree(d_vector1);
	cudaFree(d_vector2);
	cudaFree(d_output);
	return 0;
}
