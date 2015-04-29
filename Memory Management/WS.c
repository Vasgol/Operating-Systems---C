#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ask2.h"

/* Initializing of Working Set table. This holds the working set windows of both processes (BZIP, GCC) */
void WS_Init(int size)
{
	int i;

	/* Allocating Working Set Table which holds both processes' working sets windows */
	if((window=malloc(size*sizeof(WS)))==NULL)
	{
		fprintf(stderr,"No memory available\n");
		exit(EXIT_FAILURE);
	}

	/* Table needed if both processes' working sets cannot be hold both in memory */
	backup=NULL;

	/* Initializing empty working sets */
	for(i=0;i<size;i++)
	{
		window[i].page.pid=-1;
		window[i].time=-1;
		window[i].backup=-1;
	}
}

/* Searching if page already belongs to the working set of a process */
int WS_Search(int size, int ws_size, Page page, int time)
{
	int i;
	int flag=0;

	/* Checking Working set window of the process */
	for(i=0;i<size;i++)
	{
		/* Locating page in working set and update time */
		if(page.pid==window[i].page.pid && !(strcmp(page.pagenum,window[i].page.pagenum)))
		{
			if(hash_search(&page)!=0)
			{
				printf("Cannot Insert page in hash table\n");
			    return -1;
			}

			window[i].time=time;  // New page gets current time (size of working set window)
			window[i].page.dirty=page.dirty; // Updating dirty parameter 
			/* Returning 0 if page located in working set of the process */
			return 0;
		}
	
	}

	/* Returning -1 : Page fault if page not in working set of a process */
	return -1;
}

/* Insertion of a new page in Working Set because of a page fault */
int WS_Insert(int size, int ws_size, Page page, int time)
{
	int i;
	int flag=0;

	/* Locating Working set window of the process */
	for(i=0;i<size;i++)
	{
		/* Inserting new page in working set and updating time */
		if(window[i].page.pid==-1)
		{
			/* Not found in Working Set -> Page fault */
			pfcnt++;

			/* Page insertion in hashed page table (memory) */
			if(hash_insert(page)==-1)
			{
				printf("Can't insert this page in hash table\n");
				return EXIT_FAILURE;
			}
			readcnt++;

			/* Inserting the page in the working set of current process */
			window[i].page.pid=page.pid;       // Assigning process Working set window
			window[i].time=time;  // New page gets current time (size of working set window) 
			strcpy(window[i].page.pagenum,page.pagenum);
			window[i].page.dirty=page.dirty;
			
			/* Returning 0 if page inserted successfully in working set of the process */
			return 0;
		}

	}

	/* Returning -1 if memory is full. Working set of the process must be in memory so a page
	   from the other process working set must be removed */
	return -1;
}

/* Shifting Working Set Window of a process (decreasing all time counters)
   and removing page getting out of the working set */
void WS_Shifting(int size, int pid)
{
	int i;

	/* Shifting working set window of a process */
	for(i=0;i<size;i++)
	{
		/* Decreasing time counters */
		if(window[i].page.pid==pid)
		{
			window[i].time--;

			/* Removing page that stays out of the working set */
			if(window[i].time==0)
			{
				if(window[i].page.dirty==1)
					writecnt++;

				hash_remove(window[i].page);
				window[i].page.pid=-1;
			}
		}
	}
}

/* Prints Current Working Set of a process */
void WS_Print(int size, int pid)
{
	int i;

	printf("Working Set of Process %d: {",pid);
	for(i=0;i<size;i++)
	{
		if(window[i].page.pid==pid)
			printf("%s ",window[i].page.pagenum);

	}
	printf("}\n");
}

/* Backing up page that belongs to a working set of another process */
int WS_Backup(int size, int ws_size, Page page, int time)
{
	int i;
	int j;

	/* Allocating backup window to save pages */
	if(backup==NULL)
	{
		if((backup=malloc(ws_size*sizeof(WS)))==NULL)
		{
			fprintf(stderr,"No memory available\n");
			exit(EXIT_FAILURE);
		}

		/* Initializing it */
		for(j=0;j<ws_size;j++)
		backup[j].page.pid=-1;
	}

	for(i=0;i<size;i++)
	{
		/* Backing up and removing from memory, page from other process */
		if(window[i].page.pid!=page.pid)
		{
			
			/* Backing up */
			for(j=0;j<ws_size;j++)
			{
				if(backup[j].page.pid==-1)
				{
					/* Backing Up working set page of the other process */
					strcpy(backup[j].page.pagenum,window[i].page.pagenum);
					backup[j].page.dirty=window[i].page.dirty;
					backup[j].page.pid=window[i].page.pid;
					backup[j].time=window[i].time;

					/* Holding bring back place */
					backup[j].backup=i;
					break;
				}
			}
			
			/* Searching and getting page from page table */
			hash_search(&window[i].page);
			
			/* If page is dirty, must be written to disk before removed */
			if(window[i].page.dirty==1)
			writecnt++;

			/* Removing page from memory */
			hash_remove(window[i].page);

			/* Inserting page in current process working set */
			window[i].page.pid=-1;
			WS_Insert(size,ws_size,page,time);

			/* After backing up and inserting in working set returning 0 */
			return 0;
		}
	}

	return -1;
}

/* Restoring working set of a process */
void WS_Restore(int ws_size)
{
	int i;
	WS temp;

	for(i=0;i<ws_size;i++)
	{
		if(backup[i].page.pid!=-1)
		{
			/* Saving into temp struct for swaping */
			temp.page.pid=window[backup[i].backup].page.pid;
			strcpy(temp.page.pagenum,window[backup[i].backup].page.pagenum);
			temp.page.dirty=window[backup[i].backup].page.dirty;
			temp.time=window[backup[i].backup].time;

			/* Restoring pages of process that belong to its working set */
			window[backup[i].backup].page.pid=backup[i].page.pid;
			strcpy(window[backup[i].backup].page.pagenum,backup[i].page.pagenum);
			window[backup[i].backup].page.dirty=backup[i].page.dirty;
			window[backup[i].backup].time=backup[i].time;

			/* Bringing back the pages to memory (Working Set of current process must be in memory) */
			hash_insert(window[backup[i].backup].page);
			readcnt++;

			/* Backing up page of the other process working to restore it afterwards */
			backup[i].page.pid=temp.page.pid;
			strcpy(backup[i].page.pagenum,temp.page.pagenum);
			backup[i].page.dirty=temp.page.dirty;
			backup[i].time=temp.time;	

			/* Removing the other process pages from memory */
			if(backup[i].page.dirty==1)
			writecnt++;

			hash_remove(backup[i].page);

		}
	}
}

void WS_Backup_print(int ws_size)
{
	int i;
  
	printf("Backup Window: {");
  
	for(i=0;i<ws_size;i++)
	  if(backup[i].page.pid!=-1)
		  printf("%s ",backup[i].page.pagenum);
  
	printf("}\n");

}

	       
		



