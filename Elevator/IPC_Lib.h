/* File:  "IPC_Lib.h" 
Golematis Vasileios 1115 2005 00026 */

/******************* Inter-Process Communication (IPC) Library *************************/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>



/* Shared memory key , semaphore key and Permissions definitions */
#define SHMKEY (key_t)4321
#define SEMKEY (key_t)9876
#define PERMS   0666


union semun{
         	int val;
		struct semid_ds *buff;
                unsigned short *array;
	   };

/* Shared memory id , Semaphore id (Values acquired by shmget and semget functions) */
int shmid, semid;

/* Semaphore Functions */
int Sem_Down(int sem_id);
int Sem_Up(int sem_id);
int Sem_Init(int semid, int value);

/* Removes semaphore and shared memory from the system */
void Free_Resources(int shm_id, int sem_id);
