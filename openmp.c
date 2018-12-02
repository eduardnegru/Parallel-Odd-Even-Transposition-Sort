#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<string.h>
#define MAX_VALUE 1000

void odd_even_sort(int* a, int n, int threadNumber)
{
    int phase, i, temp;
 
    #pragma omp parallel num_threads(threadNumber) default(none) shared(a,n) private(i,temp,phase)
    {
        for(phase = 0; phase < n; phase++)
        {
            if(phase % 2==0) //even phase
            {
                #pragma omp for
                    for(i = 1; i < n; i += 2)
                    {
                        if(a[i-1] > a[i])
                        {
                            temp = a[i];
                            a[i] = a[i-1];
                            a[i-1] = temp;
                        }
                    }
            }
            else //odd phase
            {
                #pragma omp for
                    for(i = 1; i < n-1; i += 2)
                    {
                        if(a[i] > a[i+1])
                        {
                            temp = a[i];
                            a[i] = a[i+1];
                            a[i+1] = temp;                        
                        }
                    }
            }            
        }
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

void generate_array(int* a, int size)
{
    int i = 0;
	
	srand(time(NULL));

	for(i = 0; i < size; i++)
    {
		a[i] = rand() % MAX_VALUE;
    }
}

void dump_array(int* a, int size)
{
    int i;

	for(i = 0; i < size; i++)
    {
		printf("%d\n", a[i]);
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
    int threads = atoi(argv[2]);
	int *a = (int*)calloc(size, sizeof(int));
	int *initial = (int*)calloc(size, sizeof(int));

    generate_array(a, size);
    memcpy(initial, a, size * sizeof(int));

	odd_even_sort(a, size, threads);

    self_test(initial, a, size);
}