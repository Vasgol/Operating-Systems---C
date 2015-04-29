/* File:  "User_Lib.h" 
Golematis Vasileios 1115 2005 00026 */

/************************** Users (Children Processes) Library ***********************************/

#include <sys/time.h> /* Including time lib */


#define MILLION 1000000

/* Time struct to compute average time to get into elevator for each User */
typedef struct{

	long double curtime;
	int         timecnt;
    long double avtime;
}Time;

Time usertime;

struct timeval tv;

/* User struct holding every child's process info */
typedef struct{
		int id;					/* User Process id */
		int counter;			/* Counter of the User */
		int floor_start;        /* User starting floor */
        int floor_end;			/* User ending floor */
	      }User;

/* Global user struct used for each child process created */
User user;

/* User Functions */
void Create_User(); 		/* Saves Child info in User struct */  
void User_funct();		/* User base function of the lift simulation */
void GetinLift();		/* Function that gets User into Lift */
void WaitinLift();              /* Function that User calls when gets into lift to pause
                                   and assign new destination after waiting X steps */
void SetDestFloor(int floor_request, int direction);	/* Function that sets highest or lowest
							   floor of elevator by user */
