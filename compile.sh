#!/bin/bash
	
	gcc openmp.c -fopenmp -o openmp
	gcc pthreads.c -pthread -o pthread
	mpicc mpi.c -o mpi
	mpicc mpi.c -fopenmp -o homp
	mpicc mpi.c -pthread -o hpth
