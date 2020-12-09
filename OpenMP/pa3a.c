#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>

int ARR_SIZE = 4096000;

int main(int argc, char* argv[])
{
   int m = strtol(argv[2], NULL, 10),
       n = strtol(argv[3], NULL, 10), 
       end, i;
   long double start, *values = malloc(sizeof(long double) * ARR_SIZE);
   for (i = 0; i < ARR_SIZE; ++i) values[i] = 1;
   srand((unsigned)time(NULL));

   start = omp_get_wtime();
   omp_set_num_threads(strtol(argv[1], NULL, 10));
   #pragma omp parallel num_threads(strtol(argv[1], NULL, 10)) private(end, i)
   {
      end = ARR_SIZE/strtol(argv[1], NULL, 10) * (omp_get_thread_num() + 1);
      for (i = ARR_SIZE/strtol(argv[1], NULL, 10) * omp_get_thread_num(); i < end; ++i)
         for(size_t j = rand() % (n - m + 1) + m; j > 1; values[i] *= j--);
   }

   printf("%Lf\n", omp_get_wtime() - start);
   free(values);
   return 0;
}