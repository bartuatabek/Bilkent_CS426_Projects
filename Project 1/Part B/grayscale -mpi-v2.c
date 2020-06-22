/* Bartu Atabek
 * 21602229
 * CS 426 Project #1
 */
#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct RGB {
  unsigned char R;
  unsigned char G;
  unsigned char B;
  unsigned char GRAYSCALE;
  int index;
} RGB;

/* Slices the given image into chunks */
void get_slice(RGB *image, RGB **slice, int imageSize, int sliceSize, int i, int j) {
  int spliceColumnSize = sqrt(sliceSize);
  int arrayColumnSize = sqrt(imageSize);

  for (int k = 0; k < spliceColumnSize; k++) {
    for (int l = 0; l < spliceColumnSize; l++) {
      (*slice)[l + (k * (spliceColumnSize))] = image[(i+k)*arrayColumnSize+j+l];
    }
  }
}

/* Converts the given RGB pixels into grayscale */
void rgb2gray(RGB **image, int size) {
  int chunk_size = sqrt(size);
  for (int i = 0; i < chunk_size; i++) {
    for (int j = 0; j < chunk_size; j++) {
      (*image)[i*(chunk_size)+j].GRAYSCALE = ((*image)[i*(chunk_size)+j].R * 0.21) + ((*image)[i*(chunk_size)+j].G * 0.71) + ((*image)[i*(chunk_size)+j].B * 0.07);
    }
  }
}

int main(int argc, char *argv[]) {
  int rank; /* Rank of the process */
  int proc; /* The worker processes */
  int num_proc; /* Total number of processes */
  int num_rows; /* Number of rows in the square image */
  int sub_len; /* Length of my subarray */
  RGB *image; /* The image stored as an array */
  MPI_Status status; /* status for receive */

  double timeStart, timeEnd;

  /* Usual startup tasks */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_proc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  const int nitems = 5;
  int blocklengths[5] = {1, 1, 1, 1, 1};
  MPI_Datatype types[5] = {MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_INT};
  MPI_Datatype mpi_pixel_type;
  MPI_Aint offsets[5];

  offsets[0] = offsetof(RGB, R);
  offsets[1] = offsetof(RGB, G);
  offsets[2] = offsetof(RGB, B);
  offsets[3] = offsetof(RGB, GRAYSCALE);
  offsets[4] = offsetof(RGB, index);

  MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_pixel_type);
  MPI_Type_commit(&mpi_pixel_type);

  /* Code for Process 0 */
  if (rank == 0) {
    if (argc <= 1) {
      fprintf(stderr, "Usage: %s filepath\n", argv[0]);
      return 1;
    }

    FILE *imgFile = fopen(argv[1], "r");
    FILE *outputFile = fopen("grayscale -mpi-v2-output.txt", "w");

    if (imgFile == NULL || outputFile == NULL) {
      fprintf(stderr, "%s: failed to open file %s\n", argv[0], argv[1]);
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
      image[count].index = count;
      count++;
    }

    fclose(imgFile);
    timeStart = MPI_Wtime();

    for (int i = 0; i < num_proc / 2; i++) {
      for (int j = 0; j < num_proc / 2; j++) {
        proc = j+i*(num_proc/2);
        sub_len = (num_rows * num_rows) / num_proc;
        RGB *chunk = malloc(sub_len * sizeof(RGB));
        int sliceColumnSize = sqrt(sub_len);
        get_slice(image, &chunk, (num_rows * num_rows), sub_len, i*(sliceColumnSize), j*(sliceColumnSize));

        if (proc == 0) {
          rgb2gray(&chunk, sub_len);
          int size = sqrt(sub_len);
          for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
              image[chunk[i*(size)+j].index] = chunk[i*(size)+j];
            }
          }
          continue;
        }

        MPI_Send(&sub_len, 1, MPI_INT, proc, 0, MPI_COMM_WORLD);
        MPI_Send(chunk, sub_len, mpi_pixel_type, proc, 0, MPI_COMM_WORLD);
        free(chunk);
      }
    }

    /* Get back the chunks from the workers */
    for (proc = 1; proc < num_proc; proc++) {
      MPI_Recv(&sub_len, 1, MPI_INT, proc, 0, MPI_COMM_WORLD, &status);
      RGB *chunk = malloc(sub_len * sizeof(RGB));
      MPI_Recv(chunk, sub_len, mpi_pixel_type, proc, 0, MPI_COMM_WORLD, &status);

      int size = sqrt(sub_len);
      for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
          image[chunk[i*(size)+j].index] = chunk[i*(size)+j];
        }
      }

      free(chunk);
    }

    for (int i = 0; i < num_rows * num_rows; i++) {
      fprintf(outputFile, "%d\n", image[i].GRAYSCALE);
    }

    fclose(outputFile);
    timeEnd = MPI_Wtime();
    printf("Running time = %f seconds.\n", (timeEnd - timeStart));
  } else {
    MPI_Recv(&sub_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    image = malloc(sub_len * sizeof(RGB));
    MPI_Recv(image, sub_len, mpi_pixel_type, 0, 0, MPI_COMM_WORLD, &status);
    rgb2gray(&image, sub_len);
    /* Send back the chunk to master */
    MPI_Send(&sub_len, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Send(image, sub_len, mpi_pixel_type, 0, 0, MPI_COMM_WORLD);
  }

  // Clean up
  free(image);
  MPI_Type_free(&mpi_pixel_type);
  MPI_Finalize();
  return 0;
}
