#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include "msg.h"    /* For the message struct */
using namespace std;

/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void *sharedMemPtr;

/* The name of the received file */
const char recvFileName[] = "recvfile";

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory 
 * @param msqid - the id of the shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */

void init(int& shmid, int& msqid, void*& sharedMemPtr)
{
	fstream file;
	file.open("keyfile.txt", ios::out);
	
	if(!file)
		return;

	file << "Hello world";
	file.close();

	key_t key = ftok("keyfile.txt", 'a');
	shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, 0666|IPC_CREAT);
	sharedMemPtr = (char*) shmat(shmid,(void*)0,0);
	msqid = msgget(key, 0666|IPC_CREAT);	
}
 

/**
 * The main loop
 */
void mainLoop()
{
	/* The size of the mesage */
	int msgSize = 0;
	
	/* Open the file for writing */
	FILE* fp = fopen(recvFileName, "w");
		
	/* Error checks */
	if(!fp)
	{
		perror("fopen");	
		exit(-1);
	}

	message msg_rcv;
    message msg_snd;

	msgSize = msgrcv(msqid, &msg_rcv, sizeof(msg_rcv), SENDER_DATA_TYPE, 0);
	/* Keep receiving until the sender set the size to 0, indicating that
 	 * there is no more data to send
 	 */	

	while(msgSize != 0)
	{	
		/* If the sender is not telling us that we are done, then get to work */
		if(msgSize != 0)
		{
			/* Save the shared memory to file */
			if(fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < 0)
			{
				perror("fwrite");
			}

			msg_snd.mtype = RECV_DONE_TYPE;
			msg_snd.size = 0;
			msgsnd(msqid, &msg_snd, sizeof(msg_snd), 0); 
		}
		/* We are done */
		else
		{
			fclose(fp); /* Close the file */
		}

		msgSize = msgrcv(msqid, &msg_rcv, sizeof(msg_rcv), SENDER_DATA_TYPE, 0);
	}
}



/**
 * Perfoms the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */

void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr)
{
	/* Detach from shared memory */
	shmdt(sharedMemPtr);
	/* Deallocate the shared memory chunk */
	shmctl(shmid, IPC_RMID, NULL);
	/* Deallocate the message queue */
	msgctl(msqid, IPC_RMID, NULL);
}

/**
 * Handles the exit signal
 * @param signal - the signal type
 */

void ctrlCSignal(int signal)
{
	/* Free system V resources */
	cleanUp(shmid, msqid, sharedMemPtr);
	
	/*exit program*/
	exit(0);
}

int main(int argc, char** argv)
{
	
	signal(SIGINT, ctrlCSignal); 
				
	/* Initialize */
	init(shmid, msqid, sharedMemPtr);
	
	/* Go to the main loop */
	mainLoop();

	/* Detach from shared memory segment, and deallocate shared memory and message queue (i.e. call cleanup) */
	cleanUp(shmid, msqid, sharedMemPtr);
	
	return 0;
}
