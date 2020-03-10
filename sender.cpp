

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

	if(key == -1) {

		perror("Failure to genarate file key.");

		exit(-1);

	}



	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0666 | IPC_CREAT); //Returns an identifier for the shared memory segment

	if(shmid == -1) {

		perror("Failure to return identifier to shared memory.");

		exit(-1);

	}



	sharedMemPtr = shmat(shmid, (void*) 0, 0); //Attaches to shared memory

	if(shmid == -1) {

		perror("Failure to attach to shared memory.");

		exit(-1);

	}



	msqid = msgget(key, 0666 | IPC_CREAT); //Returns an identifier for the message queue

	if(shmid == -1) {

		perror("Failure to return identifier to message queue.");

		exit(-1);

	}

}



/**

 * Performs the cleanup functions

 * @param sharedMemPtr - the pointer to the shared memory

 * @param shmid - the id of the shared memory segment

 * @param msqid - the id of the message queue

 */



void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)

{

	//Detaches from shared memory

	if(shmdt(sharedMemPtr) == -1) {

		perror("Failure to detached from shared memory");

		exit(-1);

	}

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

		if(msgsnd(msqid, &sndMsg, sizeof(sndMsg), 0) == -1) {

			perror("Failed to send message to message queue.");

			exit(-1);

		}



	  //Recieve message from queue. Process is blocked until a message of the desired type is place in the queue or the queue is removed from the system.

		if(msgrcv(msqid, &rcvMsg, sizeof(rcvMsg), RECV_DONE_TYPE, 0) == -1) {

			perror("Failed to recieve message from reciever.");

			exit(-1);

		}







	}





	//Sends message with message type of SENDER_DATA_TYPE and size 0 to let reciever know that we are finished.

	if(msgsnd(msqid, &sndMsg, 0, 0) == -1) {

		perror("Failed to send message to message queue.");

		exit(-1);

	}





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


