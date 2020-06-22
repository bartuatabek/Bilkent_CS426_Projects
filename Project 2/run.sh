#!/bin/bash

make clean -f Makefile
make -f Makefile

echo "Running main-serial..."
./main-serial "1000"
echo "-------------------------------------"
./main-serial "10000"
echo "-------------------------------------"
./main-serial "100000"
echo "-------------------------------------"
./main-serial "1000000"
echo "-------------------------------------"
