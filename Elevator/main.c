/* File:  "main.c" 
Golematis Vasileios 1115 2005 00026 */

#include <stdio.h>
#include <stdlib.h>
#include "Elevator_Lib.h"
#include "IPC_Lib.h"



/* MAIN function, Creation of semaphore and shared memory , Simulation of elevator */ 
int main(int argc, char *argv[])
{
  
  int i,sp=0,lastfloor=0;
  int j,status,simpoints,flag=0;
  int num,*pid;
  
  
  /* Semaphore and Shared Memory Segment are set free by signal handler, if these signals are sent */
  signal(SIGINT,signal_handler);
  signal(SIGQUIT,signal_handler);
  signal(SIGUSR1,signal_handler);
  
  /* Ensuring that user gives the right parameters */
  if(argc!=5)
  {
   printf("Usage: > %s <N> <F> <X> <S>\n",argv[0]);
   printf("N: User Processes\n");
   printf("F: Total Floors\n");
   printf("X: Simulation points\n");
   printf("S: Total simulation points\n");
   return EXIT_FAILURE;
  } 
  
  N=atoi(argv[1]);   /* Number of User-Processes (Children) that use the elevator */
  F=atoi(argv[2]);   /* Number of total floors */
  X=atoi(argv[3]);   /* Number of simulation points when door of lift is open and when user waits in the floor to assign new destination */
  S=atoi(argv[4]);   /* Number of total simulation points. Parent ends the lift simulation */

  if(N<0 || F<0 || X<0 || S<0)
  {
     printf("Arguments must have positive values\n");
     return EXIT_FAILURE;
  }
  else if(S<=X)
  {
	 printf("Parameter S (maximum simulation points) must have a bigger value than X (some simulation points)\n");
	 return EXIT_FAILURE;
  }

  /* Allocating memory for an array to hold children ids. Used from parent in the end to terminate the simulation */
  if((pid=malloc(N*sizeof(int)))==NULL)
  {
     fprintf(stderr,"No memory Available\n");
     return EXIT_FAILURE;
  }
 
  /* Creating shared memory segment (the elevator). Size of the shared memory depended from number of users (N). */
  shmid=shmget(SHMKEY,(5*sizeof(int)+N*sizeof(User)),IPC_CREAT | PERMS);  
  if (shmid == -1) 
  {
	perror("Shared memory creation");
	return EXIT_FAILURE;
  }
 
  /* Creating a semaphore (array with one single semaphore that will handle lift's door access) */
  semid = semget(SEMKEY,1,IPC_CREAT | PERMS);
  if (semid == -1) 
  {
	perror("Semaphore creation ");
        /* Deleting shared memory if creation of semaphore fails and terminate program. */
	shmctl(shmid,IPC_RMID,(struct shmid_ds *)NULL);
	return EXIT_FAILURE;
  } 
  
  /* Initializing the semaphore - Ensuring that semaphore and shared memory will be deleted if function fails. Initialization of semaphore-> DOWN , parent has the control (door of lift is closed)
in the beggining. */
  if (Sem_Init(semid, 0) == -1) 
  {
    Free_Resources(shmid,semid);
    return EXIT_FAILURE;
  }
  
  
  /* Attaching shared memory segment to Parent Process */
  elevator=(Elevator*)shmat(shmid,0,0);	
  /* Pointer for the users shows in the right space inside the shared memory */
  elevator->user=(User*)(elevator+2*sizeof(int));
  /* Checking if attach of shared memory succeded. Else free resources (semaphore and shared memory) and terminate. */	  
  if(elevator==NULL)
  {
	perror("Shared memory attach ");
	Free_Resources(shmid,semid);
	return EXIT_FAILURE;
  }

  
  /* Init Elevator (Critical Section) */
  InitElevator();

  
  /* Creating the N Child-user Processes */
  printf("\n====================Users of Elevator===========================\n");
  for(i=0;i<N;i++)
  {
    /* Child Process creation */
    if((num=fork())==0)
    {
      /* Child becomes user of elevator */	
      Create_User();
      /* Breaking from loop to start request of critical section (elevator) */
      break;
    }
    /* Fork failure - Freeing resources and terminate */
    else if(num==-1)
    {
	   perror("fork");
	   Free_Resources(shmid,semid);
	   return EXIT_FAILURE;
    }
    /* Parent holding every child id */
    else
    pid[i]=num;
   
    
  }
  if(num)
  printf("=================================================================\n\n");
  
 
  
  // User
  if(!num)
  { 
      /* The user function. User request for critical section (elevator) */
      User_function();
  }
  // Parent Process
  else
  {    
       /* Elevator Simulation (Parent terminates it when all simulation points are over) */
       while(1)
       {
		   
		   printf("\nElevator is in floor %d\n\n",elevator->curfloor);
		   fflush(stdout);
	       
	       /* Parent Ups semaphore to allow users to get control of the lift */	
	       Sem_Up(semid);     

	       /* Elevator Door Open (semaphore up) for X simulation points */
	       for(j=0;j<X;j++)
	       sp++;    /* Consuming simulation points of the total simulation */
	      
	       /* Closing Door (semaphore down). Parent gets the exclusive control of the lift */
	       Sem_Down(semid);

	       
	       /* Informing total floor counter of elevator */
	       elevator->totalfloors++;
	      
	       /* Changing current floor. Moving elevator up or down depending from its direction */
	       MoveElevator();
	       
	       /* If elevator reached highest or lowest floor, marking "IDLE". That means that user "presses the button" to assign 
		  elevator's direction (as in a real elevator) */
	       if(elevator->curfloor==elevator->destfloor)
	       elevator->direction=IDLE;

	       /* Sending signal to user processes that reached their destination to leave the elevator. */
	       for(j=0;j<N;j++)
	       {
			 /* Send signal to users inside elevator that reached their ending floor */
			 if(elevator->user[j].id!=EMPTY && elevator->user[j].floor_end==elevator->curfloor)
			 {  
				 printf("#%d: signaled to continue depart\n",elevator->user[j].id);
				 fflush(stdout);
				 /* Signal from parent to user-child to wake up */
				 if(kill(elevator->user[j].id,SIGCONT)==-1)
				 printf("Parent failed to send signal SIGCONT to user %d\n",elevator->user[j].id);
		 
				 /* User just left the elevator */
				 elevator->user[j].id= EMPTY; 
			     
			 }
           }
         
       


	       /* If maximum simulation points are consumed, the simulation is terminated by parent.  */
	       if(sp>=S)
	       {
		      printf("\n\n============================================================================\n");
			  fflush(stdout);
			   /* Simulation ended. Printing total floor that elevator passed */
			  printf("\t\tEnd of Simulation!\n");
	  		  fflush(stdout);
			  printf("Elevator total floors -> %d\n",elevator->totalfloors);
			  fflush(stdout);

			  /* Parent sending signal to all children to terminate, waiting them to terminate */
			  for(i=0;i<N;i++)
			  {
			    kill(pid[i],SIGCONT);
			   /* Every child that terminates (SIGUSR1) prints its average time of getting into lift. (Signal_handler) */
			   if(kill(pid[i],SIGUSR1)==-1)
			   printf("Failed to terminate child %d\n",pid[i]);
			   /* Parent waits for every child to terminate first. */
			   if(waitpid(pid[i],&status,0)<0)
			   perror("Wait!");
			   
			  }
			  
			  printf("============================================================================\n\n"); 
			  fflush(stdout);
		  
			  /* Clear resources and exit parent to terminate the simulation (the whole program) */
			  free(pid);
			  Free_Resources(shmid,semid);
			  exit(EXIT_SUCCESS);
		  }
     }
      
  }



}



/* Signal Handler */
void signal_handler(int sig)
{

   /* Handling of signals ->  SIGINT: Ctrl-C  SIGQUIT: Ctrl-\  
      In case program terminated by user with these signals, semaphore and shared memory are cleared from the system
      before exiting */
   if(sig==SIGINT || sig==SIGQUIT)
   {
	/* Remove the shared memory segment and semaphore from the system */
  	Free_Resources(shmid,semid);
	exit(EXIT_FAILURE);
   }
   /* Signal sent by parent to terminate every child in the end of the simulation. Every child prints the average time
      to get into elevator */
   else if(sig==SIGUSR1)
   {
    
    printf("#%d with average time of getting into lift %.5Le microseconds\n",user.id,usertime.avtime/(long double)usertime.timecnt);
    fflush(stdout);
    exit(EXIT_SUCCESS);
   }
   
     
}

  
