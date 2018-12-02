#!/bin/bash

    for i in `seq 1 100`;
	do 
		mpirun -np 4 ./a.out 1000
	done
