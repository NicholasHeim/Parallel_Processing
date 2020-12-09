#include <stdio.h>
#include <string.h>
#include <mpi.h>

int MAX_BUFFER = 100;

int main(void)
{
   int size, rank;
   char html[MAX_BUFFER];

   MPI_Init(NULL, NULL);
   MPI_Comm_size(MPI_COMM_WORLD, &size);
   MPI_Comm_rank(MPI_COMM_WORLD, &rank);

   if(rank == 0)
   {
      char *temp;
      size_t len = 0;
      FILE *file;
      
      file = fopen("hello.html", "r");
      while(getline(&temp, &len, file) != EOF)
         strcat(html, temp);
      fclose(file);
      
      for (int i = 1; i < 3; i += 1)
         MPI_Send(html, MAX_BUFFER, MPI_CHAR, i, 0, MPI_COMM_WORLD);
   }
   else
   {
      printf("Process %d of %d\n", rank, size);
      MPI_Recv(html, MAX_BUFFER, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      printf("%s", html);
   }
   
   free(temp);
   MPI_Finalize();
   return 0;
}