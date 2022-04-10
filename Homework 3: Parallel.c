/*
   Name: Thomas Davidson
   Email: tedavidson1@crimson.ua.edu
   Course Section: CS 481-001
   Homework #: 3
   **Instructions to compile the program: (g++ or icc) -Wall (-fopenmp or -qopenmp) -O2 hw3.cpp 
   **Instructions to run the program: ./a.exe (size) (generations) (threads) (path for file)
   Example: ./a.exe 5000 1000 2 /scratch/$USER
*/
/*
Project Checklist:
Phase 3:
[X] Write to file
[X] Have seeded randomization
[X] Declare threads only once
[X] Parallelize the compute function
[X] TEST!
[X] Update the Github page with code and test receipts
*/
#include <iostream>
#include <string>
#include <math.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
using namespace std;

double gettime(void) {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}//end of time

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

int **randomize(int **a, int size){//populate with randoms
  for (int i = 1; i < size - 1; i++) {
    srand48(54321|i);
    for (int j = 1; j < size - 1; j++)
      if (drand48() < 0.5) a[i][j] = 1;
      else a[i][j] = 0;
  }

  return a;
}//end of randomize

void printarray(int **a, int size) {
  int i,j;

  for (i = 1; i < size - 1; i++) {
    for (j = 1; j < size - 1; j++)
      printf("%d ", a[i][j]);
    printf("\n");
  }
  cout << endl;
}//end of print

void writefile(int **a, int N, FILE *fptr) {
  int i, j;
  for (i = 0; i < N+2; i++) {
    for (j = 0; j< N+2; j++)
      fprintf(fptr, "%d ", a[i][j]);
    fprintf(fptr, "\n");
  }
}//end of writefile

int compute(int **a, int **b, int size){
  int match = 1; //starts saying that both boards match. Is set to 0 if a single cell changes
  int limit = size + 1;
  //parallelize by rows, not columns
  # pragma omp for
  for (int i = 1; i < limit; i++){ //rows
    for (int j = 1; j < limit; j++){ //columns
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


int main(int argc, char const *argv[]){
 double starttime, endtime; //for timing
 starttime = gettime();
 int size = atoi(argv[1]); //boards wanted are squared, so we will only use one value
 int maxGens = atoi(argv[2]); //will be an argument in our for loop for running
 int nthreads = atoi(argv[3]); //number of threads that can be used for the run
 char filename[BUFSIZ];
 FILE *fptr;

 sprintf(filename,"%s/output.%d.%d.%d", argv[4], size, maxGens,nthreads);
 if ((fptr = fopen(filename, "w")) == NULL) {
    printf("Error opening file %s for writing\n", argv[3]);
    perror("fopen");
    exit(-1);
 }

int **gen = NULL, **temp = NULL, **ptr; //all arrays, N^4

gen = allocarray(size + 2, size + 2); //+2 for ghost cells
temp = allocarray(size + 2, size + 2);

gen = initarray(gen, size + 2, 0); //fills both with zeros
temp = initarray(temp, size + 2, 0);

gen = randomize(gen, size + 2); //adds random cells
//printarray(gen, size+2); //default array

int trueIdentical = 1; //flag for checking if board is unchanged

  #pragma omp parallel num_threads(nthreads)
  {
    int myIdentical; //private var for a thread to use
    for (int i = 0; i < maxGens; i++){// will run as many times as needed for max gens
     myIdentical = compute(gen, temp, size);
     //every thread must now wait for the others to finish so that a decision can be made
     #pragma omp critical
     trueIdentical = myIdentical && trueIdentical;
     //let wait for every thread to apply their work
     #pragma omp barrier
     if (trueIdentical == 1) break; //every thread applied their work. A true answer exists. End if true
     //otherwise, let master swap the pointers. Wait for him.
     #pragma omp barrier
      #pragma omp master
      { //section that will be run only by a single thread. This is to prevent major issues
        ptr = gen;
        gen = temp;
        temp = ptr;
        trueIdentical = 1; //reset it //REMOVING THIS MAKES IT WORK
      }
  //barrier so that they wait for the master thread
 #pragma omp barrier
  }//end of iterating maxGens
}//end of parallel region

//back to a sequential program
endtime = gettime();
//printarray(gen, size+2); //default array

printf("Time taken for size %d = %lf seconds\n", size, endtime-starttime);

printf("Writing output to file: %s\n", filename);
writefile(gen, size, fptr);
fclose(fptr);

   return 0;
 }//end of main
