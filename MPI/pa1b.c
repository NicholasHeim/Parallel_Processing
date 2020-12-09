/* NOTE: Only works for positive single digit integer values */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

int MAX_BUFFER = 100;

int main(void)
{
   int rank, m, n;
   size_t len = 0;
   FILE *file;
   char *temp;

   file = fopen("mv-data.txt", "r");
   getline(&temp, &len, file);
   m = temp[0] - 48;
   getline(&temp, &len, file);
   n = temp[0] - 48;

   MPI_Init(NULL, NULL);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   if(rank == 0)
   {
      int mxn[m][n], nx1[n], final[m];

      for (size_t i = 0; i < m; i += 1)
      {
         getline(&temp, &len, file);
         for (size_t j = 0; j < n; j += 1)
            mxn[i][j] = temp[2 * j] - 48;
      }

      for (size_t i = 0; i < n; i += 1)
      {
         getline(&temp, &len, file);
         nx1[i] = temp[0] - 48;
      }

      // Send over half the data to process
      for (size_t i = (m/2); i < m; i += 1)
         for (size_t j = 0; j < n; j += 1)
            MPI_Send(&mxn[i][j], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);

      for (size_t i = 0; i < n; i += 1)
         MPI_Send(&nx1[i], 1, MPI_INT, 1, 0, MPI_COMM_WORLD);         
      
      for (size_t i = 0; i < (m/2); i += 1)
      {
         final[i] = 0;
         for (size_t j = 0; j < n; j += 1)
            final[i] += nx1[j] * mxn[i][j];
      }

      for (size_t i = m/2; i < m; i += 1)
         MPI_Recv(&final[i], 1, MPI_INT, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (size_t i = 0; i < m; i += 1)
         printf("%d ", final[i]);
      printf("\n");
   }
   else // Rank == 1
   {
      int nx1[n], mxn[m][n], final[n];

      for (size_t i = m/2; i < m; i += 1)
         for (size_t j = 0; j < n; j += 1)
            MPI_Recv(&mxn[i][j], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (size_t i = 0; i < n; i += 1)
         MPI_Recv(&nx1[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      
      for (size_t i = (m/2); i < m; i += 1)
      {
         final[i] = 0;
         for (size_t j = 0; j < n; j += 1)
            final[i] += nx1[j] * mxn[i][j];
      }
      
      for (size_t i = (m/2); i < m; i++)
         MPI_Send(&final[i], 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
   }

   MPI_Finalize();
   return 0;
}