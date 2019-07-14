all:
	openmp pthread mpi hybrid_openmp hybrid_pthread

openmp:
	gcc openmp.c -fopenmp -o openmp

pthread:
	gcc pthread.c -pthread -o pthread

mpi:
	mpicc mpi.c -o mpi

hybrid_openmp:
	mpicc mpi.c -fopenmp -o homp

hybrid_pthread:
	mpicc mpi.c -pthread -o hpth