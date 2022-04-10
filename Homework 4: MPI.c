/*
   Name: Thomas Davidson
   Email: tedavidson1@crimson.ua.edu
   Course Section: CS 481-001
   Homework #4
   **Instructions to compile the program: mpicc -g -Wall -O2 -o hw4 hw4.c
   **Instructions to run the program: ./hw4 (size) (generations) (path for file)
   Example: ./a.exe 5000 1000 /scratch/$USER
*/
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

double gettime(void) {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}

int **allocarray(int P, int Q) {
  int i;
  int *p, **a;

  p = (int *)malloc(P*Q*sizeof(int));
  a = (int **)malloc(P*sizeof(int*));

  if (p == NULL || a == NULL)
    printf("Error allocating memory\n");

  /* for row major storage */
  for (i = 0; i < P; i++)
    a[i] = &p[i*Q];

  return a;
} //end of allocarray

int **initarray(int **a, int size, int num) {
  //filling the arrays with zeros.
  //flag determines if we do random cells too
  for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
      // a[i][j] = drand48()*value;
      a[i][j] = num; //just filling with whatever num is

  return a;
}//end of init

//modified function for non-square matrices
int **initpartarray(int **a, int rows, int cols, int num) {
  //filling the arrays with zeros.
  //flag determines if we do random cells too
  for (int i = 0; i < rows; i++)
    for (int j = 0; j < cols; j++)
      // a[i][j] = drand48()*value;
      a[i][j] = num; //just filling with whatever num is

  return a;
}//end of init

void printarray(int **a, int size) {
  int i,j;

  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++)
      printf("%d ", a[i][j]);
    printf("\n");
  }
  printf("\n");
}//end of print

void printpartarray(int **a, int rows, int cols, int rank) {
  int i,j;
printf("Rank: %d\n", rank);
  for (i = 0; i < rows; i++) {
    for (j = 0; j < cols; j++)
      printf("%d ", a[i][j]);
    printf("\n");
  }
  printf("\n");
}//end of print

int compute(int **a, int **b, int N, int M) {
  int match = 1; //starts saying that both boards match. Is set to 0 if a single cell changes
  //parallelize by rows, not columns
  for (int i = 1; i < N + 1; i++){ //rows
    for (int j = 1; j < M + 1; j++){ //columns
      int count = 0; //variable to count living cells with
      count = a[i-1][j-1] + a[i-1][j] + a[i-1][j+1] + a[i][j-1] + a[i][j+1] + a[i+1][j-1] + a[i+1][j] + a[i+1][j+1];
      if (count == 3 && a[i][j] == 0) b[i][j] = 1; //dead now alive //edited to possibly be faster
      else if ((count == 2 || count == 3) && a[i][j] == 1) b[i][j] = 1; //lives //edited this line to possibly be faster
      else b[i][j] = 0;
      if (match == 1 && a[i][j] != b[i][j]) match = 0; //short circuit eval for identical
    }//end of cell checking
  }//end of cell checking
  return match;
}//end of compute

void writefile(int **a, int N, FILE *fptr) {
  int i, j;
  for (i = 0; i < N+2; i++) {
    for (j = 0; j< N+2; j++)
      fprintf(fptr, "%d ", a[i][j]);
    fprintf(fptr, "\n");
  }
}//end of writefile

int **randomize(int **a, int size){//populate with randoms
  for (int i = 1; i < size - 1; i++) {
    srand48(54321|i);
    for (int j = 1; j < size - 1; j++)
      if (drand48() < 0.5) a[i][j] = 1;
      else a[i][j] = 0;
  }

  return a;
}//end of randomize

int main(int argc, char *argv[]){

  //general variables needed, as well as ones for each process
  int rank, size, length, maxGens, myN, remain, myCount, myIdentical, trueIdentical, myNumber, i;
  int **original = NULL, **myBoard, **myTemp, **ptr, *sendbuf = NULL;
  int *counts = NULL, *displs = NULL;
  double starttime, endtime; //for timing
  char filename[BUFSIZ];
  FILE *fptr;

//begin MPI stuff
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  length = atoi(argv[1]); //boards wanted are squared, so we will only use one value
  maxGens = atoi(argv[2]); //will be an argument in our for loop for running
  myN = length / size; //how many pieces for each process. rows / processes
  remain = length % size; //check for leftovers. They need to be distributed.

  if (rank == 0){
    starttime = gettime();
    sprintf(filename,"%s/output.%d.%d.%d", argv[3], length, maxGens, size);
    if ((fptr = fopen(filename, "w")) == NULL) {
     printf("Error opening file %s for writing\n", argv[3]);
     perror("fopen");
     exit(-1);
    }
  //now that file things are handled, the main board needs to be made
 original = allocarray(length + 2, length + 2);
 original = initarray(original, length + 2, 0);
 original = randomize(original, length + 2);
 //printarray(original, length + 2);
 //with the main board made, it now needs to be divided among the processes

 counts = malloc(sizeof(int)*size); //arrays for finding the size and displacement
 displs = malloc(sizeof(int)*size);
//displacement calculating code provided
 for (int i = 0; i < size; i++){
   counts[i] = myN + ((i < remain)?1:0); //assigns how many pieces each process should get
   counts[i] = counts[i] * (length + 2); //multiply by the number of values per row + ghost cells
   //printf("%d ", counts[i]);
 }

 displs[0] = 0; //naturally, master process starts at 0 offset
 for (int i = 1; i < size; i++){ //for each processes index in the array
   displs[i] = displs[i-1] + counts[i-1];
   //printf("%d ", displs[i]);
}
sendbuf = &original[1][0];
}//end of rank 0 setup


myCount = myN + ((rank < remain)?1:0); //total number of rows each will get
myNumber = myCount * (length + 2);
myBoard = allocarray(myCount + 2, length + 2); //the board will be allocated to the size found and have 2 extra spots for ghost cells
myBoard = initpartarray(myBoard, myCount + 2, length + 2, 0); //myCount is how many rows I have + 2 ghost rows. length is how many columns
myTemp = allocarray(myCount + 2, length + 2); //board for temp states
myTemp = initpartarray(myTemp, myCount + 2, length + 2, 0);
MPI_Scatterv(sendbuf, counts, displs, MPI_INT, &myBoard[1][0], myNumber, MPI_INT, 0, MPI_COMM_WORLD);

MPI_Barrier(MPI_COMM_WORLD);
//Every process now has their board. Everything up until here is correct
int next = rank + 1;
int prev = rank - 1;
if (rank == 0) prev = MPI_PROC_NULL;
if (rank == (size - 1)) next = MPI_PROC_NULL;
 //starttime = gettime(); //begin timing right before we start the actual looping
 for (i = 0; i < maxGens; i++){
  //top: was 1 0
  MPI_Sendrecv(&myBoard[1][0], length + 2, MPI_INT, prev, 1, &myBoard[0][0], length + 2, MPI_INT, prev, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
    //bottom was my count 0
  MPI_Sendrecv(&myBoard[myCount][0], length + 2, MPI_INT, next, 1, &myBoard[myCount + 1][0], length + 2, MPI_INT, next, 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
//  printpartarray(myBoard, myCount + 2, length + 2, rank);
                /* WORKS UP TO HERE */
      myIdentical = compute(myBoard, myTemp, myCount, length); //myCount is the number of rows, length is columns
      //all computations are done
      MPI_Barrier(MPI_COMM_WORLD); //we must wait for all processes to have finished
      //perform a reduce operation to see if things are identical or not
      MPI_Allreduce(&myIdentical, &trueIdentical, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

      if (trueIdentical == size) break;
         ptr = myBoard;
         myBoard = myTemp;
         myTemp = ptr;
  }//end of generations

  if (rank == 0) endtime = gettime();
  MPI_Gatherv(&myBoard[1][0], myNumber, MPI_INT, sendbuf, counts, displs, MPI_INT, 0, MPI_COMM_WORLD);
  if (rank == 0){ //stuff for process 0 to do before end
  printf("Time taken for size %d = %lf seconds\n", length, endtime-starttime);
  if (trueIdentical == size) printf("Identical\n");
  printf("Writing output to file: %s\n", filename);
  writefile(original, length, fptr);
}

MPI_Finalize();
return 0;
}
//mpicc -g -Wall -O2 -o hw4 hw4.c
//run_script_mpi myscript.sh

