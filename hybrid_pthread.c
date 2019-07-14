#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"
#include <time.h>
#include <string.h>
#include<pthread.h>

typedef struct
{
    int* array;
    int size;
    int nrThreads;
    int threadID;
    pthread_barrier_t* barrier;
}arguments;

void *sort_array(void* a)
{
    arguments *args = (arguments*)a;
    int* array = args->array;
    int size = args->size;
    int threadID = args->threadID;
    int nrThreads = args->nrThreads;
    int i, phase;
       
    for(phase = 0; phase < size; phase++)
    {
        if(phase % 2 == 0) //even phase
        {
            int localSize = size;

            if(size % 2 != 0)
                localSize--;

            int step = localSize / nrThreads;
            
            if(step % 2 != 0)
            {
                step += 1;
            }

            int low = threadID * step;
            int high = (threadID + 1) * step;            

            if(threadID == nrThreads - 1)
            {
                high = localSize;
            }
            
            for(i = low; i < high; i += 2)
            {
                if(array[i] > array[i+1])
                {
                    int temp;
                    temp = array[i];
                    array[i] = array[i+1];
                    array[i+1] = temp;
                }
            }
        }
        else //odd phase
        {
            int localSize = size - 1;
            
            int step = localSize / nrThreads;
            
            if(step % 2 != 0)
            {
                step += 1;
            }

            int low = threadID * step + 1;
            int high = (threadID + 1) * step + 1;

            if(threadID == nrThreads - 1)
            {
                high = localSize;
            }

            for(i = low; i < high; i += 2)
            {
                if(array[i] > array[i+1])
                {
                    int temp;
                    temp = array[i];
                    array[i] = array[i+1];
                    array[i+1] = temp;                        
                }
            }
        }
        pthread_barrier_wait(args->barrier);
    }
}

void sort_pthread(int* a, int n, int nrThreads)
{
    int phase, i, temp;

    pthread_t *threads = (pthread_t*)calloc(nrThreads, sizeof(pthread_t));
    pthread_barrier_t barrier;

    pthread_barrier_init(&barrier, NULL, nrThreads);

    for(i = 0; i < nrThreads; i++)
    {
        arguments* args = (arguments*)malloc(sizeof(arguments));
        
        if(args == NULL)
        {
            printf("Cannot allocate memory\n");
            exit(-1);
        }

        args->array = a;
        args->size = n;
        args->nrThreads = nrThreads;
        args->threadID = i;
        args->barrier = &barrier;
        int ret = pthread_create(&threads[i], NULL, sort_array, (void*)args);
    }

    for(i = 0; i < nrThreads; i++)
    {
        int ret = pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier);
}

void init_data(int *array, int length)
{    
    srand(time(NULL));

    for (int i = 0; i < length; i++)
    {
        array[i] = rand() % 1000;
    }
}

void dump_array(int* array, int size)
{
    int i;

	for(i = 0; i < size; i++)
    {
		printf("%d ", array[i]);
    }

    printf("\n");
}

int compare_integers(const void* pInt1, const void* pInt2)
{
    int a = *((int*)pInt1);
    int b = *((int*)pInt2);

    return (a < b) ? -1 : (a == b) ? 0 : 1;
}

int find_neighbour(int rank, int phase, int nrProcesses)
{
    int neighbour;

    if(phase % 2 == 0)
    {
        // find even neighbour
        neighbour = rank % 2 == 0 ? ++rank : --rank;
    }
    else
    {
        // find odd neighbour
        neighbour = rank % 2 == 0 ? --rank : ++rank;
    }

    return (neighbour > nrProcesses - 1 || neighbour <= 0) ? -1 : neighbour; 
}

void merge_low(int *a, int *b, int lengthA, int lengthB)
{
   int i = 0, j = 0, k = 0;
   int tempLength = lengthA < lengthB ? lengthA : lengthB;
   int* temp = (int*)calloc(tempLength, sizeof(int));

   while (k < tempLength)
   {
      if (a[i] <= b[j])
      {
         temp[k++] = a[i++];
      }
      else
      {
         temp[k++] = b[j++];
      }
   }

   memcpy(a, temp, tempLength * sizeof(int));
}

void merge_high(int *a, int *b, int lengthA, int lengthB)
{
    int tempLength = lengthA > lengthB ? lengthA : lengthB;
    int i = lengthA - 1, j = lengthB - 1, k = tempLength - 1;    
    int* temp = (int*)calloc(tempLength, sizeof(int));

    while (k >= 0)
    {
        if (a[i] >= b[j])
        {
            temp[k--] = a[i--];
        }
        else
        {
            temp[k--] = b[j--];
        }
    }

    memcpy(a, temp, tempLength * sizeof(int));
}

void odd_even_sort(int* localArray, int sliceLength, int rank, int nrProcesses, int nrThreads, int globalLength)
{
    int phase;
    
    sort_pthread(localArray, sliceLength, nrThreads);

    for(phase = 0; phase < nrProcesses; phase++)
    {
        MPI_Status status;
        int neighbour = find_neighbour(rank, phase, nrProcesses);
        int neighbourLength = globalLength / (nrProcesses - 1); // master process does not receive an array slice

        if(neighbour == nrProcesses - 1)
        {
            neighbourLength = globalLength - (neighbour - 1) * neighbourLength;
        }

        int* neighbourArray = (int*)calloc(neighbourLength, sizeof(int));

        if(neighbour != -1)
        {
            if(phase % 2 == 0)
            {        
                MPI_Send(localArray, sliceLength, MPI_INT, neighbour, 0, MPI_COMM_WORLD);
                MPI_Recv(neighbourArray, neighbourLength, MPI_INT, neighbour, 0, MPI_COMM_WORLD, &status);
                
                if(rank % 2 != 0)
                    merge_high(localArray, neighbourArray, sliceLength, neighbourLength);
                else
                    merge_low(localArray, neighbourArray, sliceLength, neighbourLength);     
            }
            else
            {
                MPI_Send(localArray, sliceLength, MPI_INT, neighbour, 0, MPI_COMM_WORLD);
                MPI_Recv(neighbourArray, neighbourLength, MPI_INT, neighbour, 0, MPI_COMM_WORLD, &status);

                if(rank % 2 != 0)
                    merge_low(localArray, neighbourArray, sliceLength, neighbourLength);
                else
                    merge_high(localArray, neighbourArray, sliceLength, neighbourLength);
            }
        }
    }

}

/**
    Append array b to array a.
*/

void join_arrays(int* a, int* b, int bLength, int currentOffset)
{
    int i;
    
    for(i = 0; i < bLength; i++)
    {
        a[currentOffset + i] = b[i];
    }
}

void self_test(int* initial, int* sorted, int length)
{
    int i;

    qsort(initial, length, sizeof(int), compare_integers);

    for(i = 0; i < length; i++)
    {
        if(initial[i] != sorted[i])
        {
            printf("[Error] Array is not sorted\n");
            return;
        }
    }
    printf("[OK] Array is sorted\n");
}


int main(int argc, char *argv[])
{
    int arrayLength, nrProcesses, nrThreads, rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nrProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    arrayLength = atoi(argv[1]);
    nrThreads = atoi(argv[2]);

    if(nrProcesses <= 1)
    {
        printf("Please use more than 1 process\n");
        MPI_Finalize();
        exit(0);
    }

    if(rank == 0)
    {
        // Master process

        int i, currentOffset = 0;
        int *initialArray = (int*)calloc(arrayLength, sizeof(int));
        int *sortedArray = (int*)calloc(arrayLength, sizeof(int));
        
        init_data(initialArray, arrayLength);
        
        for(i = 1; i < nrProcesses; i++)
        {
            int sliceLength = arrayLength / (nrProcesses - 1); // master process does not receive an array slice
 
            if(i == nrProcesses - 1)
            {
                sliceLength = arrayLength - (i - 1) * sliceLength;
            }

            int *localArray = (int*)calloc(sliceLength, sizeof(int));
            int *receivedArray = (int*)calloc(sliceLength, sizeof(int));

            memcpy(localArray, initialArray + currentOffset, sliceLength * sizeof(int));
            currentOffset += sliceLength;

            MPI_Send(localArray, sliceLength, MPI_INT, i, 0, MPI_COMM_WORLD);            
        }

        currentOffset = 0;

        for(i = 1; i < nrProcesses; i++)
        {
            int sliceLength = arrayLength / (nrProcesses - 1); // master process does not receive an array slice
            MPI_Status status;

            if(i == nrProcesses - 1)
            {
                sliceLength = arrayLength - (i - 1) * sliceLength;
            }

            int *receivedArray = (int*)calloc(sliceLength, sizeof(int));
            MPI_Recv(receivedArray, sliceLength , MPI_INT, i, 0, MPI_COMM_WORLD, &status);
            join_arrays(sortedArray, receivedArray, sliceLength, currentOffset);
            currentOffset += sliceLength;
        }

        self_test(initialArray, sortedArray, arrayLength);
    }
    else
    {
        // Worker Process
        int i, sliceLength = arrayLength / (nrProcesses - 1); // master process does not receive an array slice
        MPI_Status status;

        if(rank == nrProcesses - 1)
        {
            sliceLength = arrayLength - (rank - 1) * sliceLength;
        }

        int *receivedArray = (int*)calloc(sliceLength, sizeof(int));
        MPI_Recv(receivedArray, sliceLength , MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        

        odd_even_sort(receivedArray, sliceLength, rank, nrProcesses, nrThreads, arrayLength);

        MPI_Send(receivedArray, sliceLength, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    exit(0);
}
