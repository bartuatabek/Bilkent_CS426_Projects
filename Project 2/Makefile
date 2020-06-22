all: main-serial main-parallel

main-serial: serial.c
	gcc -o main-serial serial.c

main-parallel: main.c
	mpicc -o main-parallel main.c

clean:
	rm -f *.o main-serial
	rm -f *.o main-parallel
