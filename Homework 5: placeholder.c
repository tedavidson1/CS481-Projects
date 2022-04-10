/*
   Name: Thomas Davidson
   Email: tedavidson1@crimson.ua.edu
   Course Section: CS 481-001
   Homework #5
   **Instructions to compile the program: mpicc -Wall -o myallgather1 allgather_driver.c myallgather1.c
   **Instructions to run the program: srun --mpi=pmi2 ./myallgather1
*/
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>

int allgather(void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf, int
recvcount, MPI_Datatype recvtype, MPI_Comm comm){

/*writing out the steps
1. Every process will send their data to the root, including the root
2. Root will receive from every process (use a for loop to iterate ranks)
Need a wait after this. Reference his scatter code
3. The root must now send the receive buffer to every other process
Every other process meanwhile will be receiving
*/

int rank, size, i, offset, root;
MPI_Status *status;
MPI_Request *request;
MPI_Aint lb, sizeofsendtype, sizeofrecvtype;

MPI_Comm_rank (comm, &rank);
MPI_Comm_size (comm, &size);

status = malloc (sizeof (MPI_Status) * (size + 1));
request = malloc (sizeof (MPI_Request) * (size + 1));

//Step 1: Every process sends their buffer to the root
root = 0; //root is process 0
MPI_Type_get_extent (sendtype, &lb, &sizeofsendtype); //get size of data type being sent
MPI_Isend(sendbuf, sizeofsendtype * sendcount, MPI_CHAR, root, 0, comm, &request[0]);
//now that everyone has sent data, the root has to receive it, then send it

//Step 2: Root must receive the data
if (rank == root){ //if we are the root
  for (i = 0; i < size; i++){//for every process, receive the data
    MPI_Type_get_extent (recvtype, &lb, &sizeofrecvtype); //getting the size of receiving
    offset = sizeofrecvtype * recvcount * i; //offset for data. Use to know where to put it in array
    char *bufptr = recvbuf + offset; //receive the data from each process, starting at offset
    MPI_Irecv(bufptr, sizeofrecvtype * recvcount, MPI_CHAR, i, 0, comm, &request[i + 1]);
  }

  MPI_Waitall (size + 1, request, status); //wait for all to be received

  //receive now has the assembled data. Broadcast it to every process.
  //Step 3: Root will send all the data to every process
  for (i = 0; i < size; i++){
    MPI_Type_get_extent (recvtype, &lb, &sizeofrecvtype); //get type sizing
    char *bufptr = recvbuf; //we are sending the entire thing, so the recvbuffer is the send buffer
    MPI_Isend (bufptr, sizeofrecvtype * recvcount * size, MPI_CHAR, i, 0, comm, &request[i]);
  }
  //wait to guarentee that all have sent
  MPI_Type_get_extent (recvtype, &lb, &sizeofrecvtype); //find type sizing again
  MPI_Irecv (recvbuf, sizeofrecvtype * recvcount * size, MPI_CHAR, root, 0, comm, &request[i]);
  //receive the data from the root (sent to self)
  MPI_Waitall (size + 1, request, status); //wait for all to be sent, +1 since we receive from self
}

//Step 4: All non-root processes wait to receive the full dataset
else{ //every other process is receiving the sent data
  MPI_Wait(&request[0], MPI_STATUS_IGNORE);
  MPI_Type_get_extent (recvtype, &lb, &sizeofrecvtype); //get type
  MPI_Irecv(recvbuf, sizeofrecvtype * recvcount * size, MPI_CHAR, root, 0, comm, &request[0]);
  //receive in recv buf, we expect the number of items specified by the function call
  MPI_Wait (&request[0], MPI_STATUS_IGNORE); //wait to receive before proceeding
  }
  //everyone is done, free arrays to prevent memory leaks

free (request);
free (status);
return 0;
}//end of function
//mpicc -Wall -o myallgather1 allgather_driver.c myallgather1.c
//run_script_mpi myscript.sh
