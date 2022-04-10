/*
   Name: Thomas Davidson
   Email: tedavidson1@crimson.ua.edu
   Course Section: CS 481-001
   Homework #5
   **Instructions to compile the program: mpicc -Wall -o myallgather2 allgather_driver.c myallgather2.c
   **Instructions to run the program: srun --mpi=pmi2 ./myallgather2
*/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>

int allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int
recvcount, MPI_Datatype recvtype, MPI_Comm comm){

int rank, size, i, tracker, target, sendoffset, recvoffset, mask;
MPI_Aint lb, sizeofsendtype, sizeofrecvtype;

MPI_Comm_rank (comm, &rank);
MPI_Comm_size (comm, &size);

MPI_Type_get_extent (sendtype, &lb, &sizeofsendtype);
MPI_Type_get_extent (recvtype, &lb, &sizeofrecvtype);


tracker = rank; //variable to track the bit shifting
sendoffset = rank; //for where we send data from
recvoffset = rank; //for where we receive data to
mask = size - 1; //gives all ones for the mask. Used for finding recvoffset

//first, put own data into the recv buffer so that things are workable
/*
sendbuf is the data it has, so no changes
sendcount * sizeofsendtype is used since that is how we calculate how much data was sent
sends to self
recvbuf is what will hold all data, so put in specific index by using rank * how many items we have
*/
MPI_Sendrecv(sendbuf, sendcount * sizeofsendtype, MPI_CHAR, rank, 0,
  recvbuf + (rank * recvcount * sizeofrecvtype) , recvcount * sizeofrecvtype, MPI_CHAR, rank, 0, comm, MPI_STATUSES_IGNORE);

//now that own data is in the recv buffer, we can start sending and receiving parts of the buffer
for (i = 1; i < size; i = i << 1){ //shift left is same as times 2
  if (tracker & 1) target = rank - i; //odd case, ends with a 1
  else target = rank + i; //even case, no match with ending 1

//now find offsets. These are needed to send the proper part and receive proper part
recvoffset = target & mask; //keep only the ones


int sendval = (sendoffset * sizeofsendtype * sendcount);
int recvval = (recvoffset * sizeofrecvtype * recvcount);
/*
recvbuf is where our data is and where it will be sent from. We use the calculated sendoffset to know where it goes.
we are sending the size of the data type multiplied by the power of 2 and the starting amount
target is the calculated target process done by messing with bits
recvbuf needs to be receiving at a specific area found from the offset found. Pretty identical to sending
Size is identical to that we are sending
*/
//printf("Rank: %d is about to perform a sendrecv for %d\n", rank, i);
  MPI_Sendrecv(recvbuf + (sendval), sendcount * i * sizeofsendtype, MPI_CHAR, target, 0,
    recvbuf + (recvval), recvcount * i * sizeofrecvtype, MPI_CHAR, target, 0, comm, MPI_STATUSES_IGNORE);

  tracker = tracker >> 1; //divides it by two
  mask = mask - i; //make least signifigant bit a 0
  if (recvoffset < sendoffset) sendoffset = recvoffset; //get our next location to send from
}

return 0;
}//end of function

//mpicc -Wall -o myallgather2 allgather_driver.c myallgather2.c
//run_script_mpi myscript.sh
