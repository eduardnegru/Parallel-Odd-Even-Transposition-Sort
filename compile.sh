#!/bin/bash

rm openmp pthread
gcc openmp.c -fopenmp -o openmp
gcc pthreads.c -lpthread -o pthread