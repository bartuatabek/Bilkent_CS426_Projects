/* Bartu Atabek - 21602229
* CS 426 Project #1
* A serial program that takes as input an array of integers,
* stored in 'input.txt' with one integer per line, and prints out
* the min of the elements in the file
*/
#include <time.h>
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

int main(int argc, char* argv[]) {
  int array_len; /* Length of the main array */
  int *search_array; /* The array to search */
  int global_min; /* Minimum for the main array */

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
  clock_t start = clock();
  global_min = find_min(search_array, array_len);
  printf("Min: %d\n", global_min);
  clock_t stop = clock();
  double elapsed = (double)(stop - start) / CLOCKS_PER_SEC;
  printf("Running time = %f seconds.\n", elapsed);

  free(search_array);
  return 0;
}
