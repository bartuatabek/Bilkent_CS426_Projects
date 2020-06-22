all: sparse_mat_vect_mult

sparse_mat_vect_mult: sparse_mat_vect_mult.c
	gcc -fopenmp -o exe sparse_mat_vect_mult.c

clean:
	rm -f *.o exe
