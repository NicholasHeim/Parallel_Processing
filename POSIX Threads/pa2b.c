#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h> // Requires linking with -lpthread
#include <semaphore.h>

/* Continue from PartA, now you need to add the result from each thread to a global variable: sum.  
   Next add the code to protect the critical section of sum using 
      (1) nothing,         n
      (2) busy-waiting,    b
      (3) mutex,           m
      (4) semaphore,       s
      (5) write lock.      w
   Add code to allow users to select which protection method to use when starting your program using argv. 
   Run your program with m=n=1, record in performance2.txt the following: 
      sum, run-time, speedup, efficiency, scalability (strongly, weakly, or not at all) for each of the four protection methods.
 */

/* Global Variables */
int ARR_SIZE = 10000000;
int TCOUNT = 1;
// For busy-waiting
long double BUSY_CURRENT = 0;
// For mutex
pthread_mutex_t MUTEX;
// For semaphore
sem_t SEMAPHORE;
// For read/write lock
pthread_rwlock_t RWLOCK;

long double SUM = 0;

void *noLimit(void *);
void *busy(void *);
void *mute(void *);
void *sema(void *);
void *lock(void *);
long double factorial(int);

int main(int argc, char *argv[])
{
   int n = 1, m = 1, i, j;
   struct timespec begin, end;
   double elapsed;
   long double **values = NULL;
   pthread_t *threads;
   char mode = 'n';
   void *(*function)(void*);
   srand((unsigned)time(NULL));
   
   if(argv[1])
      TCOUNT = atoi(argv[1]);
   threads = malloc(sizeof(pthread_t) * TCOUNT);

   if(argv[2])
      mode = argv[2][0];

/*    printf("Enter two numbers between 1 and 1745 where m < n\n");
   scanf("%d %d", &m, &n); */

   // Begin timing at allocation of the array of values
   clock_gettime(CLOCK_MONOTONIC, &begin);

   /* Allocate space for the 2D array of values
      It is declared this way in order to avoid copying the values to a smaller
      array later on in order to pass the values (non-globally) to each thread.
      There is no lock on writing because none of the threads are writing
      to an overlapping block, each is going to a unique subset of the original
      values array. 
   */
   values = malloc(sizeof(long double) * TCOUNT);
   if(mode == 'n' || mode == 'm' || mode == 's' || mode == 'w')
      for (size_t i = 0; i < TCOUNT; i += 1)
         values[i] = (long double *)malloc(sizeof(long double) * (ARR_SIZE / TCOUNT));
   else if(mode == 'b')
   {
      for (size_t i = 0; i < TCOUNT; i += 1) // One more in size for a flag variable
      {
         values[i] = (long double *)malloc(sizeof(long double) * (ARR_SIZE / TCOUNT + 1));
         values[i][ARR_SIZE/TCOUNT] = i; // Used to check turn
      }
   }

   switch (mode)
   {
   case 'n':
      function = noLimit;
      break;
   case 'b':
      function = busy;
      break;
   case 'm':
      function = mute;
      pthread_mutex_init(&MUTEX, NULL);
      break;
   case 's':
      function = sema;
      sem_init(&SEMAPHORE, 0, 1);
      break;
   case 'w':
      function = lock;
      pthread_rwlock_init(&RWLOCK, NULL);
      break;
   default:
      printf("An invalid mode character was entered.\n");
      return 1;
   }

   // Calculate random values within the given range for the array
   for (i = 0; i < TCOUNT; i += 1)
   {   
      for (j = 0; j < ARR_SIZE/TCOUNT; j += 1)
         values[i][j] = (long double)((rand() % (n - m + 1)) + m);
   }

   // Create threads and pass in the function and values
   for (i = 0; i < TCOUNT; i += 1)
      pthread_create(&threads[i], NULL, function, (void*)values[i]);

   // Wait for threads to finish processing and then free the values
   for (i = 0; i < TCOUNT; i += 1)
      pthread_join(threads[i], NULL);
   free(threads);
   for (i = 0; i < TCOUNT; i += 1)
      free(values[i]);
   free(values);
   switch (mode)
   {
   case 'm':
      pthread_mutex_destroy(&MUTEX);
      break;
   case 's':
      sem_destroy(&SEMAPHORE);
      break;
   case 'w':
      pthread_rwlock_destroy(&RWLOCK);
      break;
   }
   
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

void *noLimit(void *params)
{
   int i;
   long double *nums = (long double *) params;

   for (i = 0; i < ARR_SIZE/TCOUNT; i+=1)
   {
      nums[i] = factorial(nums[i]);
      SUM += nums[i];
   }
}

void *busy(void *params)
{
   int i;
   long double *nums = (long double *) params;
   // Each thread waits for its turn
   while (BUSY_CURRENT != nums[ARR_SIZE/TCOUNT]);
   for (i = 0; i < ARR_SIZE/TCOUNT; i+=1)
   {
      nums[i] = factorial(nums[i]);
      SUM += nums[i];
   }
   BUSY_CURRENT += 1;
}

void *mute(void *params)
{
   int i;
   long double *nums = (long double *) params;
   
   for (i = 0; i < ARR_SIZE/TCOUNT; i+=1)
   {
      nums[i] = factorial(nums[i]);
      pthread_mutex_lock(&MUTEX);
      SUM += nums[i];
      pthread_mutex_unlock(&MUTEX);
   }
}

void *sema(void *params)
{
   int i;
   long double *nums = (long double *) params;
   
   for (i = 0; i < ARR_SIZE/TCOUNT; i+=1)
   {
      nums[i] = factorial(nums[i]);
      sem_wait(&SEMAPHORE);
      SUM += nums[i];
      sem_post(&SEMAPHORE);
   }
}

void *lock(void *params)
{
   int i;
   long double *nums = (long double *) params;
   
   for (i = 0; i < ARR_SIZE/TCOUNT; i+=1)
   {
      nums[i] = factorial(nums[i]);
      pthread_rwlock_wrlock(&RWLOCK);
      SUM += nums[i];
      pthread_rwlock_unlock(&RWLOCK);
   }
}
