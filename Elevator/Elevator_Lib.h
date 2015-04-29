/* File:  "Elevator_Lib.h" 
Golematis Vasileios 1115 2005 00026 */

/**************************** Elevator and signal handler ***************************************/

#include <signal.h>
#include "User_Lib.h"


/* Elevator's direction definitions */
#define UP   1    /* Elevator direction UP */
#define DOWN 0    /* Elevator direction DOWN */
#define IDLE 2    /* Elevator waits for a request to go up or down when it reaches highest or
                      lowest floor */
#define EMPTY -1  /* Elevator marked empty position. Can hold all N users */


/* Elevator's shared memory struct  */
typedef struct{
		int curfloor;	 /* Current floor */
        int direction;   /* Direction of Elevator */
	    User *user;		 /* Users inside elevator */
		int destfloor;	 /* Destination floor (Highest or lowest floor) */
        int totalfloors; /* Total Floors that elevator passed */
               
              }Elevator;

/* Pointer that shared memory (Elevator) is attached. */
Elevator *elevator;

/* Parameters given from User from Command Prompt */
int N,F,X,S;

/* Elevator Functions */
void InitElevator();
void ChangeDirection();
void MoveElevator();


/* Signal Handler */
void signal_handler(int sig);



