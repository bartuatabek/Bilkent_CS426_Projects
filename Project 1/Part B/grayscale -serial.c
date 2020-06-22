#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct RGB {
  unsigned char R;
  unsigned char G;
  unsigned char B;
  unsigned char GRAYSCALE;
} RGB;

/* Converts the given RGB pixels into grayscale */
void rgb2gray(RGB **image, int size) {
  for (int i = 0; i < size; i++) {
    (*image)[i].GRAYSCALE = (((*image)[i].R) * 0.21) + (((*image)[i].G) * 0.71) + (((*image)[i].B) * 0.07);
  }
}

int main(int argc, char* argv[]) {
  RGB *image; /* The image stored as an array */
  int num_rows;

  if (argc <= 1) {
    fprintf(stderr, "Usage: %s filepath\n", argv[0]);
    return 1;
  }

  FILE *imgFile = fopen(argv[1], "r");

  if (imgFile == NULL) {
    fprintf(stderr, "%s: failed to open file %s\n", argv[0], argv[1]);
    return 1;
  }

  FILE *outputFile = fopen("grayscale -serial-output.txt", "w");
  if (outputFile == NULL) {
    printf("Error opening file!\n");
    return 1;
  }

  char pixel[25];
  int count = 0;

  char *input = NULL;
  size_t len = 0;
  ssize_t read;

  // Get the number of rows
  getline(&input, &len, imgFile);
  num_rows = atoi(input);

  /* Uncomment this line in order to view as image. */
  // fprintf(outputFile, "P2\n%d %d\n255\n", num_rows, num_rows);
  image = malloc(num_rows * num_rows * sizeof(RGB));

  while ((read = getline(&input, &len, imgFile)) != -1) {
    strcpy(pixel, input);
    image[count].R = atoi(strtok(pixel, ","));
    image[count].G = atoi(strtok(NULL, ","));
    image[count].B = atoi(strtok(NULL, ","));
    count++;
  }

  fclose(imgFile);

  clock_t start = clock();
  rgb2gray(&image, num_rows*num_rows);

  for (int i = 0; i < num_rows * num_rows; i++) {
    fprintf(outputFile, "%d\n", image[i].GRAYSCALE);
  }

  clock_t stop = clock();
  double elapsed = (double)(stop - start) / CLOCKS_PER_SEC;
  printf("Running time = %f seconds.\n", elapsed);

  fclose(outputFile);
  free(image);
  return 0;
}
