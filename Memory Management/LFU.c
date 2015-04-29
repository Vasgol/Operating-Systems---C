/********************* LFU with aging page victimization algorithm ************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ask2.h"

/* Initialization of LFU table. Table holds pagenumbers and used counters */
void LFU_Init(int size)
{
	int i;

	/* Allocating memory for LFU table */
	if((lfu=malloc(size*sizeof(LFU)))==NULL)
	{
		fprintf(stderr,"No memory available\n");
		exit(EXIT_FAILURE);
	}

	/* Initializing pages' used counters to -1 (empty LFU table) */
	for(i=0;i<size;i++)
		lfu[i].usedcnt=-1;

}

/* Page insertion in LFU table */
int LFU_Insert(Page page, int size)
{
	int i;

	/* Inserting page and updating used counter */
	for(i=0;i<size;i++)
	{
		if(lfu[i].usedcnt==-1)
		{
			strcpy(lfu[i].page.pagenum,page.pagenum);   // LFU page entry 
			lfu[i].page.dirty=page.dirty;
			lfu[i].page.pid=page.pid;
			lfu[i].usedcnt=1;   // Page used for first time 
			lfu[i].time=clock;  // Time inserted in memory 
			
			return 0;
		}
	}
	return -1;
}

/* Updating used counter of page */
int LFU_Update(Page page, int size)
{
	int i;

	/* Inserting page and updating used counter */
	for(i=0;i<size;i++)
	{
		if(lfu[i].usedcnt!=-1 && !strcmp(lfu[i].page.pagenum,page.pagenum) && lfu[i].page.pid == page.pid)
		{
			lfu[i].usedcnt++;   // Updating used counter of the page
			lfu[i].page.dirty=page.dirty; // Updating dirty parameter
			return 0;
		}
	}

	return -1;
}

/* Victimization with LFU. Searching for the least frequently used page in LFU table. Returning the
   the victim page in order to remove from hashed page table. */
Page LFU_victim(int maximum, int size)
{
	int i,least;
    int maxclock=clock;
	
	/* Searching into LFU table for the page with smallest used counter (Least Frequently Used page) */
	for(i=0;i<size;i++)
	{
		/* Finding the least frequently used page */	
		if(lfu[i].usedcnt!=-1 && lfu[i].usedcnt<maximum)
		{	
			maximum=lfu[i].usedcnt;		
			least=i;
			
		}
	}

	/* In case there are more than one page with a least used counter, victimize the oldest that came into
	    memory */
	for(i=0;i<size;i++)
	{
		/* Finding the oldest page of the least frequently used (better for the locality ) */	
		if(lfu[i].usedcnt==lfu[least].usedcnt && lfu[i].time<maxclock)
		{	
			maxclock=lfu[i].time;		
			least=i;	
		}
	}


	/* Freeing LFU table entry of least frequently used page and returning it */
	lfu[least].usedcnt=-1;

	return lfu[least].page;
}

/* After getting 5*q references from trace files, all page used counters are divided by 2 in LFU table */
void LFU_aging(int size)
{
   int i;

   for(i=0;i<size;i++)
	   if(lfu[i].usedcnt!=-1)
			lfu[i].usedcnt/=2;  /* Dividing page used counter by 2 (aging) */
}