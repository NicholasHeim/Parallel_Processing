#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include <float.h>

/* Write a program to:
   (9) calculate and add into performance.txt: the speedup and efficiency of all of the above cases (tables 3.6 & 3.7 in the textbook); 
   (10) at the top of performance.txt, tell me what are the values of m and n you used, whether your program is scalable, if yes, then strongly or weakly.
        You can manually enter your result tables into performance.txt or let your program to append to it.
*/

/* Global Variables */
int ARR_SIZE = 5000000;

long double factorial(int);
void genRandoms(long double *, int, int);

int main(void)
{
   int rank, size, n, m;
   double start, end;
   long double *values = NULL;

   MPI_Init(NULL, NULL);
   MPI_Comm_size(MPI_COMM_WORLD, &size);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);
   srand((unsigned)time(NULL));

   if (rank == 0)
   {
      printf("Enter two numbers between 1 and 1750 where m < n\n");
      scanf("%d %d", &m, &n);
      start = MPI_Wtime();
      values = malloc(sizeof(long double) * ARR_SIZE);
      for (size_t i = 0; i < ARR_SIZE; i += 1)
         values[i] = (rand() % (n - m + 1)) + m;
   }

   if (rank != 0)
      values = malloc(sizeof(long double) * (ARR_SIZE / size));
   MPI_Scatter(values, ARR_SIZE / size, MPI_LONG_DOUBLE, values, ARR_SIZE / size, MPI_LONG_DOUBLE, 0, MPI_COMM_WORLD);

   for (int i = 0; i < ARR_SIZE / size; i += 1)
      values[i] = factorial(values[i]);

   MPI_Gather(values, ARR_SIZE / size, MPI_LONG_DOUBLE, values, ARR_SIZE / size, MPI_LONG_DOUBLE, 0, MPI_COMM_WORLD);

   if (rank == 0)
   {
      end = MPI_Wtime();
      printf("Time taken: %f seconds\n", end - start);
   }

   free(values);
   MPI_Finalize();
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
