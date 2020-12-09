#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h> // Requires linking with -lpthread

/* Global Variables */
int ARR_SIZE = 10000000;
int TCOUNT = 1;

void *threadFunc(void *);
long double factorial(int);

int main(int argc, char *argv[])
{
   int n, m, i, j;
   struct timespec begin, end;
   double elapsed;
   long double **values = NULL;
   pthread_t *threads;
   srand((unsigned)time(NULL));
   
   if(argv[1])
      TCOUNT = atoi(argv[1]);
   threads = malloc(sizeof(pthread_t) * TCOUNT);

   printf("Enter two numbers between 1 and 1750 where m < n\n");
   scanf("%d %d", &m, &n);

   // Begin timing at allocation of the array of values
   clock_gettime(CLOCK_MONOTONIC, &begin);
   
   // Allocate space for the 2D array of values
   // It is declared this way in order to avoid copying the values to a smaller
   // array later on in order to pass the values (non-globally) to each thread.
   // There is no lock on writing because none of the threads are writing
   // to an overlapping block, each is going to a unique subset of the original
   // values array.
   values = malloc(sizeof(long double) * TCOUNT);
   for (size_t i = 0; i < TCOUNT; i += 1)
      values[i] = (long double *)malloc(sizeof(long double) * (ARR_SIZE / TCOUNT));

   // Calculate random values within the given range for the array
   for (i = 0; i < TCOUNT; i += 1)
      for (j = 0; j < ARR_SIZE/TCOUNT; j += 1)
         values[i][j] = (long double)((rand() % (n - m + 1)) + m);

   // Create threads and pass in the function and values
   for (i = 0; i < TCOUNT; i += 1)
      pthread_create(&threads[i], NULL, threadFunc, (void*)values[i]);

   // Wait for threads to finish processing and then free the values
   for (i = 0; i < TCOUNT; i += 1)
      pthread_join(threads[i], NULL);   
   free(values); free(threads);

   // Calculation and output for the time the program takes to execute
   clock_gettime(CLOCK_MONOTONIC, &end);
   elapsed = end.tv_sec - begin.tv_sec;
   elapsed += (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
   printf("%f\n", elapsed);

   return 0;
}

/* Simple iterative factorial function */
long double factorial(int val)
{
   long double temp = 1;
   for (int i = 2; i <= val; i += 1)
      temp *= i;
   return temp;
}

void *threadFunc(void *params)
{
   int i;
   long double *nums = (long double *) params;
   for (i = 0; i < ARR_SIZE/TCOUNT; i+=1)
      nums[i] = factorial(nums[i]);
}
