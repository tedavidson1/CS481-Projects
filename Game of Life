/*
   Name: Thomas Davidson
   Email: tedavidson1@crimson.ua.edu
   Course Section: CS 481-001
   Homework #: 1
   **Instructions to compile the program: g++ -Wall hw1.cpp -O2
   **Instructions to run the program: ./a.exe (size) (generations)
   Example: ./a.exe 5000 1000
*/

/*
Project Checklist:
[X] Establish command line arguments and usage of parameters
[X] create the 2d array (non-dynamic) with ghost cells
[X] randomly fill the usable space of the 2d array with living or dead cells
[X] write the algorithm to check each cell and determine future state in next iteration
[X] Replace with dynamic 2d arrays
[X] implement the time keeping algorithm for testing
[X] Testing on indicated sizes and reports
Phase 2:
[] Impleent an update list
[] could use structs and make the update system check 2 fields in a struct. Would return space complexity to N^2
  a struct could even reduce complexity back to N
*/
#include <iostream>
#include <string>
#include <math.h>
#include <sys/time.h>
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
  for (int i = 1; i < size - 1; i++)
    for (int j = 1; j < size - 1; j++)
    a[i][j] = rand() % 2; //random value between 0 and 1

  return a;
}//end of randomize

void printarray(int **a, int size) {
  int i,j;

  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++)
      printf("%d ", a[i][j]);
    printf("\n");
  }
  cout << endl;
}//end of print

int compute(int **a, int **b, int size){
  //needs to only check cells that are set to 1 in the updated matrix
  //use a conditional for the changed check
  //update the updated list after putting next gen in B
  //if a cell changes, it and all of it's neighbors must be set to 1 in updated
  //this will require 2 more arrays, one update to run through and another to add to, then they swap
  //returns an int for if boards are identical
  int match = 1; //starts saying that both boards match. Is set to 0 if a single cell changes
  for (int i = 1; i < size + 1; i++){ //rows
    for (int j = 1; j < size + 1; j++){ //columns
      //if (updateOne[i][j] == 1){ //if a value of 1 is in the updated list, we have to check it
      int count = 0; //variable to count living cells with
      //improvement: removed a loop and logical condition, as well as an int var. Saved 8 condition
      //checks and 1 cell check
      count = a[i-1][j-1] + a[i-1][j] + a[i-1][j+1] + a[i][j-1] + a[i][j+1] + a[i+1][j-1] + a[i+1][j] + a[i+1][j+1];
      if (a[i][j] == 0 && count == 3) b[i][j] = 1; //dead now alive
      else if (a[i][j] == 1 && (count == 2 || count == 3)) b[i][j] = 1; //lives
      else b[i][j] = 0;

      //if ()

      if (match == 1 && a[i][j] != b[i][j]) match = 0; //short circuit eval for identical
      //}
    }//end of cell checking
  }//end of cell checking
  return match;
}//end of compute

void swap (int **a, int **b, int **c){//swaps two 2d arrays
  c = a;
  a = b;
  b = a;
}//end of swap


int main(int argc, char const *argv[]){
 double starttime, endtime; //for timing
 starttime = gettime();
 int size = atoi(argv[1]); //boards wanted are squared, so we will only use one value
 int maxGens = atoi(argv[2]); //will be an argument in our for loop for running
int **genone = NULL, **gentwo = NULL; //all arrays, N^4

genone = allocarray(size + 2, size + 2); //+2 for ghost cells
gentwo = allocarray(size + 2, size + 2);
//updateOne = allocarray(size + 2, size +2); //used for tracking cells that changed
//updateTwo = allocarray(size + 2, size +2); //used for tracking cells that changed

genone = initarray(genone, size + 2, 0); //fills both with zeros
gentwo = initarray(gentwo, size + 2, 0);
//updateOne = initarray(updated, size + 2, 1); //fills with ones, since all cells must be checked first run
//updateTwo = initarray(updated, size + 2, 0); //fills with zeroes
genone = randomize(genone, size + 2); //adds random cells

//printarray(genone, size + 2);

  //board is now populated. Run it through the generations
  int identical = 0; //flag for checking if board is unchanged
  for (int i = 0; i < maxGens / 2; i++){// will run as many times as needed for max gens
    //cout << "Next 2" << endl;
     identical = compute(genone, gentwo, size);//first array is current board, second is next board
     if (identical == 1) break; //very simple check for if a generation repeated
    //printarray(gentwo, size + 2);
    identical = compute (gentwo, genone, size);
    if (identical == 1) break;
    //printarray(genone, size + 2);
}
endtime = gettime();
printf("Time taken for size %d = %lf seconds\n", size, endtime-starttime);
if (identical == 1) cout << " Identical";
   return 0;
 }//end of main
