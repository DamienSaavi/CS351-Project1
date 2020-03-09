

#include <sys/shm.h>

#include <sys/msg.h>

#include <stdio.h>

#include <stdlib.h>

#include <unistd.h>

#include "msg.h"    /* For the message struct */

#include <iostream>



/* The size of the shared memory chunk */

#define SHARED_MEMORY_CHUNK_SIZE 1000



/* The ids for the shared memory segment and the message queue */

int shmid, msqid;



/* The pointer to the shared memory */

void* sharedMemPtr;



/**

 * Sets up the shared memory segment and message queue

 * @param shmid - the id of the allocated shared memory

 * @param msqid - the id of the shared memory

 */



void init(int& shmid, int& msqid, void*& sharedMemPtr)

{

	key_t key = ftok("keyfile.txt", 'a'); //Generates a unique key from the pathname and project identifier

	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0666 | IPC_CREAT); //Returns an identifier for the shared memory segment

	sharedMemPtr = shmat(shmid, (void*) 0, 0); //Attaches to shared memory



	msqid = msgget(key, 0666 | IPC_CREAT); //Returns an identifier for the message queue

}



/**

 * Performs the cleanup functions

 * @param sharedMemPtr - the pointer to the shared memory

 * @param shmid - the id of the shared memory segment

 * @param msqid - the id of the message queue

 */



void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)

{

	shmdt(sharedMemPtr); //Detaches from shared memory

	shmctl(shmid, IPC_RMID, NULL);

	msgctl(msqid, IPC_RMID, NULL);

}



/**

 * The main send function

 * @param fileName - the name of the file

 */

void send(const char* fileName)

{

	/* Open the file for reading */

	FILE* fp = fopen(fileName, "r");





	/* A buffer to store message we will send to the receiver. */

	message sndMsg;



	/* A buffer to store message received from the receiver. */

	message rcvMsg;



	/* Was the file open? */

	if(!fp)

	{

		perror("fopen");

		exit(-1);

	}



	/* Read the whole file */

	while(!feof(fp))

	{

		/* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and store them in shared memory.

 		 * fread will return how many bytes it has actually read (since the last chunk may be less

 		 * than SHARED_MEMORY_CHUNK_SIZE).

 		 */

		if((sndMsg.size = fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp)) < 0)

		{

			perror("fread");

			exit(-1);

		}





		//Pass message to message queue of type SENDER_DATA_TYPE.

		sndMsg.mtype = SENDER_DATA_TYPE;

		msgsnd(msqid, &sndMsg, sizeof(sndMsg), 0);

	  //Recieve message from queue. Process is blocked until a message of the desired type is place in the queue or the queue is removed from the system.

		msgrcv(msqid, &rcvMsg, sizeof(message), RECV_DONE_TYPE, 0);

	}





	//Sends message with message type of SENDER_DATA_TYPE and size 0 to let reciever know that we are finished.

	msgsnd(msqid, &sndMsg, 0, 0);





	/* Close the file */

	fclose(fp);



}





int main(int argc, char** argv)

{



	/* Check the command line arguments */

	if(argc < 2)

	{

		fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);

		exit(-1);

	}



	/* Connect to shared memory and the message queue */

	init(shmid, msqid, sharedMemPtr);



	/* Send the file */

	send(argv[1]);



	/* Cleanup */

	cleanUp(shmid, msqid, sharedMemPtr);



	return 0;

}


