/* File:  "IPC.c" 
Golematis Vasileios 1115 2005 00026 */

/************************ Inter-Process Communication Functions ************************/

#include <stdio.h>
#include "IPC_Lib.h"


/* Semaphore Down operation, using semop */
int Sem_Down(int sem_id)
{
	struct sembuf sem_d;
	sem_d.sem_num = 0;
	sem_d.sem_op = -1;
	sem_d.sem_flg = 0;
	if (semop(sem_id, &sem_d, 1) == -1) 
        {
		perror("Semaphore down operation failed");
		return -1;
	}
	return 0;
}

/* Semaphore Up operation, using semop */
int Sem_Up(int sem_id)
{
	struct sembuf sem_d;
	sem_d.sem_num = 0;
	sem_d.sem_op =  1;
	sem_d.sem_flg = 0;
	if (semop(sem_id,&sem_d,1) == -1) 
        {
		perror("Semaphore up operation failed ");
		return -1;
	}
	return 0;
}

/* Semaphore Init */
int Sem_Init(int sem_id, int val)
{
	union semun arg;
	arg.val = val;
	if (semctl(semid,0,SETVAL,arg) == -1)
        {
		perror("Semaphore setting value ");
		return -1;
	}
	return 0;
}

/* Function that removes semaphore and shared memory from the system */
void Free_Resources(int shm_id, int sem_id)
{
	/* Delete the shared memory segment */
	shmctl(shm_id,IPC_RMID,NULL);
	/* Delete the semaphore */
	semctl(sem_id,0,IPC_RMID,0);
}

