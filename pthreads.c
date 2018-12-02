#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<pthread.h>
#include<syscall.h>
#include<string.h>
#define MAX_VALUE 1000

typedef struct
{
    int* array;
    int size;
    int threadID;
}arguments;

int nrThreads = 0;

pthread_barrier_t barrier;

void dump_array(int* a, int size)
{
    int i;

	for(i = 0; i < size; i++)
    {
		printf("%d ", a[i]);
    }

    printf("\n");
}

void *sort_array(void* a)
{
    arguments *args = (arguments*)a;
    int* array = args->array;
    int size = args->size;
    int threadID = args->threadID;
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
        pthread_barrier_wait(&barrier);
    }
}

int compare_integers(const void* pInt1, const void* pInt2)
{
    int a = *((int*)pInt1);
    int b = *((int*)pInt2);

    return (a < b) ? -1 : (a == b) ? 0 : 1;
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

void odd_even_sort(int* a, int n, int nrThreads)
{
    int phase, i, temp;

    pthread_t *threads = (pthread_t*)calloc(nrThreads, sizeof(pthread_t));

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
        args->threadID = i;
        int ret = pthread_create(&threads[i], NULL, sort_array, (void*)args);
    }

    for(i = 0; i < nrThreads; i++)
    {
        int ret = pthread_join(threads[i], NULL);
    }
}

void generate_array(int* a, int size)
{
    int i = 0;
	
	srand(time(NULL));

	for(i = 0; i < size; i++)
    {
		a[i] = rand() % MAX_VALUE;
    }
}

int main(int argc, char** argv)
{
    if(argc != 3)
    {
        printf("Error: Please provide the size of the array to sort and the number of threads to use\n");
        exit(-1);       
    }
  
    int size = atoi(argv[1]);
    int *a = (int*)calloc(size, sizeof(int));
    int *initial = (int*)calloc(size, sizeof(int));
    
    nrThreads = atoi(argv[2]);

    pthread_barrier_init(&barrier, NULL, nrThreads);
	
    generate_array(a, size);
    memcpy(initial, a, size * sizeof(int));
    
    odd_even_sort(a, size, nrThreads);
    
    self_test(initial, a, size);
    
    pthread_barrier_destroy(&barrier);
}