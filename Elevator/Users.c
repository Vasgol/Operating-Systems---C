/* File:  "Users.c" 
Golematis Vasileios 1115 2005 00026 */

/****************************** Users' Functions ******************************************/
#include "Elevator_Lib.h"
#include "IPC_Lib.h"
#include <stdio.h>
#include <time.h>
#include <signal.h>

/* This function fills user struct's data for the child process in order to
   enter lift simulation */
void Create_User()
{

  /* Assigning unique time seed for each child-user */
  srand(time(NULL)+getpid());

  /* User process id */
  user.id=getpid();
  user.counter=0;
  /* Assigning starting floor and ending floor of user process. Ensuring that they are different */
  do{
  		user.floor_start=rand() % (F+1) + 0;  /* User enters elevator in this random floor (0 to F) */
  		user.floor_end  =rand() % (F+1) + 0;  /* User requests to go in this random floor  (0 to F) */
   }while(user.floor_start == user.floor_end);

  /* Assigning current time */
  usertime.timecnt=0;      /* Time counter holding how many times a user got into elevator. */
  gettimeofday(&tv,NULL);  /* Getting time of day */
  usertime.curtime=tv.tv_sec+(tv.tv_usec/(long double)MILLION); /* Computing in microseconds */

  
  printf("#%d: Starting floor is %d and ending floor is %d\n",user.id,user.floor_start,user.floor_end);
  fflush(stdout);

}

/* User base function. User tries to enter lift, using semaphore operations and shared 
   memory (Elevator) */
void User_function()
{
    int i=0,direction;

  /* Users busy waiting, trying to get control of the lift */
  while(1)
  {  
   
	/* Getting direction that user requests to go */
	if(user.floor_start > user.floor_end)
    direction=DOWN;
    else
    direction=UP;

    /* User downs semaphore trying to get control of the elevator */   
    Sem_Down(semid);

	/* If Elevator's curfloor and direction agrees with user's starting floor and direction, the user moves in. */
	if(user.floor_start==elevator->curfloor && direction==elevator->direction) 
	{
	   GetinLift();  // User gets into lift
	   SetDestFloor(user.floor_end,direction);  // User sets elevator's destination floor 
   
       Sem_Up(semid);     // Semaphore Up after user got into elevator 

  	   WaitinLift();     /* User pauses and waits for parent signal. Then after waiting for X simulation points,
		                      request for new ending floor */
		 
		 
	}
	 
    /* If Elevator's curfloor and highest or lowest floor aggrees with user's starting floor ,
    the user sets elevator's direction. (Elevator is idle) */
	else if(user.floor_start==elevator->curfloor && user.floor_start==elevator->destfloor) 
	{
		 
 	    GetinLift();  // User gets into lift
	    elevator->direction=direction; // Sets elevator's direction ("presses the button")
	    SetDestFloor(user.floor_end,direction);  // And sets elevator's new destination floor 
	    Sem_Up(semid);      // Semaphore Up after user got into elevator 
		 
	    WaitinLift();     /* User pauses and waits for parent signal. Then after waiting for X simulation points,
		                      request for new ending floor */
		
  	}
	/* If Elevator's current floor isn't in user's starting floor - User Busy waiting */
	else
	{
	    /* If elevator is idle (because reached highest or lowest floor) */
	    if(elevator->direction == IDLE)
	    {
	      /* If user's floor start is higher change direction to UP else Down */
	      if(user.floor_start > elevator->destfloor)
	      elevator->direction = UP;
	      else
	      elevator->direction = DOWN;
	    }

	    /* Assigning elevator's destination floor */
	    SetDestFloor(user.floor_start,elevator->direction);
	    
	    /* User Ups semaphore */
	    Sem_Up(semid);
	     
	}
   
   }
    
    
}

/* This function gets User into the Elevator */
void GetinLift()
{
   int i;
   
   for(i=0;i<N;i++)
   {
      /* Getting an empty position in elevator */
      if(elevator->user[i].id==EMPTY)
      {
             /* Computing time difference to get into elevator */ 
	     gettimeofday(&tv,NULL);  // Getting current time
         usertime.curtime=tv.tv_sec+(tv.tv_usec/(long double)MILLION) - usertime.curtime; // Computing time difference
	     usertime.timecnt++;  // Times of getting into elevator +1
         usertime.avtime+=usertime.curtime; // Holding the sum of time differences in order to compute average time before
											// child terminates

	     printf("#%d: Entered elevator in floor %d after %.5Le microsecs\n",getpid(),elevator->curfloor,usertime.curtime);
	     fflush(stdout);   
         /* Getting User into elevator . (Elevator now holds user's id inside it) */
         elevator->user[i].id=user.id;
             
	   
         /* Elevator also holds user's request to leave it when he reaches its ending floor. (Parent wakes him up with 
            signal SIGCONT) */
         elevator->user[i].floor_end=user.floor_end;
         
		 /* End function of getting user into elevator */
	     return;
	  }
   }
}

/* This function pauses User into Elevator and when it reaches destination, it
   waits X steps and then assigns new destination floor */
void WaitinLift()
{
   int i;
   /* User pauses */
   raise(SIGSTOP);
   
   /* Remove comments to check which floor user exited the elevator */
  // printf("#%d: Exited elevator in floor %d\n",user.id,user.floor_end);
  // fflush(stdout);
             
		
   // Waiting X simulation points after reaching destination 
   for(i=0;i<X;i++)
   user.counter++;
		
                
   // Generating new floor destination
   user.floor_start=user.floor_end;   // User waited X points in his ending floor
   do{
	 user.floor_end = rand() % (F+1) + 0;  // User requests now for a new floor (0 to F) to go using the elevator
	       
    }while(user.floor_start == user.floor_end);
   	  
   /* Remove comments to check which floor user requests to go to */
  //  printf("#%d: Asking for new destination floor %d\n",getpid(),user.floor_end);
  //  fflush(stdout);

   /* Getting current time in microseconds. User tries again after that to use the elevator. Holding
      this time to compute the time difference. */
   gettimeofday(&tv,NULL);
   usertime.curtime=tv.tv_sec+(tv.tv_usec/(long double)MILLION);

   

}

/* This function used by Users to set Elevator's highest or lowest floor to go (extreme request) */
void SetDestFloor(int floor_request, int direction)
{
    
    /* Elevator goes Up or Down. 
       -> Up: checking if there is a user in a highest floor and assigns new destination.
       -> Down: checking if there is a user in a lowest floor and assigns new destination.
              
    */ 
    if((floor_request>elevator->destfloor && elevator->direction==UP)||
       (floor_request<elevator->destfloor && elevator->direction==DOWN))
		elevator->destfloor=floor_request;
 
    
}
