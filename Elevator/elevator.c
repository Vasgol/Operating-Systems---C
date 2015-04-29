/* File:  "elevator.c" 
Golematis Vasileios 1115 2005 00026 */

/********************* Functions used by parent process to control elevator ********************/

#include "Elevator_Lib.h"



/* Elevator Initialization */
void InitElevator()
{  
   int i;
   /* Marking elevator empty of users at start */
   for(i=0;i<N;i++)
   elevator->user[i].id=EMPTY;  

   elevator->curfloor=0;       /* Elevator starting from base (floor 0) */
   elevator->direction=UP;     /* Elevator first going UP */
   elevator->totalfloors=0;    /* Elevator counter of total floors set to 0 */
   elevator->destfloor=0;      /* Elevator highest-lowest floor (extreme request) set to 0 */
}

/* This function is used by the parent process to change elevator's current floor */
void MoveElevator()
{
       if(elevator->direction==UP)
       elevator->curfloor++;
       else if(elevator->direction==DOWN)
       elevator->curfloor--;
}
  
/* This function changes elevator's direction */  
void ChangeDirection()
{
     if(elevator->direction==UP)
     elevator->direction=DOWN;
     else
     elevator->direction=UP;
}
