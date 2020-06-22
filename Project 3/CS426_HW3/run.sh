#!/bin/bash

make clean -f Makefile
make -f Makefile

# ./exe no_of_threads no_of_repetitions 0 “filename”
echo "This is a sample test script."
echo "Since it's not indicated what should be its contents it runs the
program on the given 3 matrices with sample argument values."


echo "Running Sparse Matrix - Vector Multiplication"
echo "-------------------------------------"
echo "Test-file: fidapm08.mtx"
./exe "32" "10" "0" "fidapm08.mtx"

echo "-------------------------------------"
echo "Test-file: fidapm11.mtx"
./exe "32" "10" "0" "fidapm11.mtx"

echo "-------------------------------------"
echo "Test-file: cavity02.mtx"
./exe "32" "10" "0" "cavity02.mtx"
