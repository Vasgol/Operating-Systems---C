#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ask2.h"


/* User Parameters */
char alg[5];
int frames,q,ws,max;


int main(int argc, char* argv[])
{
	FILE *fp,*fp1, *fp2;
    char *buf=malloc(TRACE_SIZE*sizeof(char));
	Page page,victim; 
	int refindex=0,ref1=0, ref2=0;
	int i=0,j=0,pn=0, pid , aging=0, bup=0;	
	int age=0;

	if(argv[1]==NULL || strlen(argv[1])>5)
	{
		printf("Invalid first parameter %s\n",argv[1]);
		return EXIT_FAILURE;
	}
	strcpy(alg,argv[1]);
	if(!strcmp(alg,"LFU") && (argc==4 || argc==5))
	{
		clock=0;  /* Initializing clock (used for improving the LFU algorithm) */
		frames=atoi(argv[2]);
		q=atoi(argv[3]);
		if(argc==5)
		max=atoi(argv[4]);
		else
		max=MAX_REFS;
		
		if(max>MAX_REFS)
		{
			printf("Cannot read %d references. (Maximum -> %d)\n",max,MAX_REFS);
			return EXIT_FAILURE;
		}
	}
	else if(!strcmp(alg,"WS") && (argc==6 || argc==5))
	{
		ws=atoi(argv[2]);
		frames=atoi(argv[3]);
		q=atoi(argv[4]);
		if(argc==6)
		max=atoi(argv[5]);
		else
		max=MAX_REFS;

		if(ws>frames)
		{
			printf("Working Set window size must not exceed number of memory frames\n");
			return EXIT_FAILURE;
		}

	}
	else
	{
		printf("Usage: %s <alg> (<ws>) <frames> <q> (<max>)\n",argv[0]);
		printf("alg:  LFU or WS\n");
		printf("ws:   Size of window for WS algorithm if working set algorithm is chosen\n");
		printf("frames: Number of frames of main memory\n");
		printf("q:    Set of reference traces to be read each time from each file\n");
		printf("max: Maximum number of references to be read from the trace files\n");
		return EXIT_FAILURE;
	}

	/* Checking if user gave non positive values */
	for(i=2;i<argc;i++)
	{
		if(atoi(argv[i])<=0)
		{
			printf("Parameters must have positive values\n");
			return EXIT_FAILURE;
		}
	}


	/* Opening Trace Files to get references */
	if((fp1=fopen(BZIP,"rb"))==NULL)
	{
		fprintf(stderr,"Cannot open file %s\n",GCC);
		return EXIT_FAILURE;
	}

	if((fp2=fopen(GCC,"rb"))==NULL)
	{
		fprintf(stderr,"Cannot open file %s\n",BZIP);
		return EXIT_FAILURE;
	}
	
	/* Initializing Statistics Counters */
	pfcnt=0;
	readcnt=0;
	writecnt=0;
	

	/* Creating hashed page table */
	if(hash_create(frames)==-1)
    return EXIT_FAILURE;

	/* Initializing LFU table if selected for page victimization algorithm */
	if(!strcmp(alg,"LFU"))
		LFU_Init(frames);
	/* Initializing WS table holding both processes' Working Sets */
	else
		WS_Init(frames);


	

	fp=fp1;
	pid=0;  //  BZIP -> process 0  , GCC -> process 1
    printf("\nReading from BZIP\n");

	/* Reading max references from both trace files */
	while(!feof(fp1) || !feof(fp2))
	{
			/* Getting reference in a buf */
			if(get_reference(refindex,buf,fp)!=0)
			printf("Failed to get reference from trace file\n");

			/* Getting page (page number and dirty parameter) */
			page=get_page(buf);
			page.pid=pid;

			/* Simulation with LFU */
			if(!strcmp("LFU",alg))
			{
				clock++;

				/* Searching for page in memory if not in memory - page fault load it from hd. */
				if(hash_search(&page)==PF)
				{
					/* Not found in memory -> Page fault */
					pfcnt++;

					/* If not exceeded available frames insertion of page in memory */
					if(hashtable.fcounter<frames)
					{
						/* Page insertion in hashed table */
						if(hash_insert(page)==-1)
						{
							printf("Can't insert this page in hash table\n");
							return EXIT_FAILURE;
						}

						/* Insert page in LFU table and update used counter if LFU algorithm is selected */
						if(!strcmp(alg,"LFU"))
						{
							if(LFU_Insert(page,frames)!=0)
							{
								printf("LFU algorithm failed\n");
								return EXIT_FAILURE;
							}
							aging++;
						}


						/* Page read from hard disk */
						printf("Page was read from hard disk\n");
						
					}
					else
					{
						printf("Page must be replaced\n");
						/* Getting victim page with LFU algorithm if selected */
						if(!strcmp(alg,"LFU"))
						{
							if(max!=-1)
							victim=LFU_victim(max,frames);
						}
							
						/* If page is dirty is written into hard disk before removed from memory */
						if(victim.dirty==1)
						writecnt++;
						

						printf("Removing page %s of process %d...\n",victim.pagenum,victim.pid);
						if(hash_remove(victim)!=0)
						{
							printf("Failed to remove page\n");
							return EXIT_FAILURE;
						}
			
						/* Inserting the page */
						if(hash_insert(page)==-1)
						printf("Can't insert this page in hash table\n");

						/* Insert page in LFU table and update used counter if LFU algorithm is selected */
						if(!strcmp(alg,"LFU"))
						{
							if(LFU_Insert(page,frames)!=0)
							{
								printf("LFU algorithm failed\n");
								return EXIT_FAILURE;
							}
							aging++;
						}
						
					}

					readcnt++;
				}
				else
				{
					printf("Page found in memory \n");

					/* Updating page used counter for LFU algorithm */
					if(!strcmp(alg,"LFU"))
					{
						if(LFU_Update(page,frames)!=0)
						{
							printf("LFU algorithm failed\n");
							return EXIT_FAILURE;
						}
						aging++;
					}
				}
			}
			/* Working Set Algorithm (page fault if page is not in working set of the process) */
			else
			{
				WS_Shifting(frames,pid);

				/* First Checking if page of process is located in its working set */
				if(WS_Search(frames,ws,page,ws)==PF)
				{
					printf("Page %s not found in working set of process %d\n",page.pagenum,pid);
					
					/* Insertion in Ws of the process */
					if(WS_Insert(frames,ws,page,ws)!=0)
					{
						/* If insertion failed getting page in working set by removing and backing up page
						   from the other process working set */
						if(WS_Backup(frames,ws,page,ws)!=0)
						{
							printf("Working Set algorithm failed\n");
							return EXIT_FAILURE;
						}
						bup=1;
					}

				}

				//WS_Print(frames,pid);
				
			}


			j++;
			if(fp==fp1)
			refindex=++ref1;
			else
			refindex=++ref2;
			
			

			/* Switching - Changing trace file after q references */
			if(j==q)
			{
			  j=0;
			  if(fp==fp1)
			  {
			   printf("\nReading from GCC\n");
			   fp=fp2;
			   refindex=ref2;
			   pid=1;
			  }
			  else if(fp==fp2)
			  {
			   printf("\nReading from BZIP\n");
			   fp=fp1;
			   refindex=ref1;
			   pid=0;
			  }
			  
			  /* After reading 5*q references all page counters are divided by 2 (LFU with aging) */ 
			  if(!strcmp("LFU",alg) && aging==5*q)
			  {
				  age++;
				  printf("LFU AGING\n");
				  LFU_aging(frames);
				  aging=0;
			  }
			  /* Working set of the other process must be restored before switching */
			  else if(bup==1)
			  {
				  WS_Backup_print(ws);
				  WS_Restore(ws);
			  }

			}

			/* Checking if reached maximum references of traces in each file and switch to the other
			   in case User gave max references parameter */
			if((!strcmp(alg,"LFU") && argc==5) || (!strcmp(alg,"WS") && argc==6))
			{
				/* If any of the two files reached max references continue with the other till termination */
				if(ref1==max && ref2!=max)
				{
				  refindex=ref2;
				  fp=fp2;
				  pid=1;
				}
				else if(ref2==max && ref1!=max)
				{
				  refindex=ref1;
				  fp=fp1;
				  pid=0;
				}
				else if(ref2==max && ref1==max)
				break;
			}
			else
			{
				/* If any of the two files reached max references continue with the other till termination */
				if(feof(fp1) && !feof(fp2))
				{
					refindex=ref2;
					fp=fp2;
					pid=1;
				}
				else if(feof(fp2) && !feof(fp1))
				{
				  refindex=ref1;
				  fp=fp1;
				  pid=0;
				}
			}


		
		
	}

	printf("\n=======================Statistics===========================\n");
	printf("Number of writes into hard disk %d\n",writecnt);
	printf("Number of reads from hard disk %d\n",readcnt);
	printf("Number of page faults %d\n",pfcnt);
	printf("Number of references read  %d (BZIP(%d) GCC(%d))\n",ref1+ref2,ref1,ref2);
	printf("Number of memory frames %d\n",frames);
	printf("============================================================\n");
	
	free(buf);
	hash_destroy();

	if(!strcmp(alg,"LFU"))
	free(lfu);
	else
	{
		free(window);
		if(bup==1)
			free(backup);
	}
	
	fclose(fp1);
	fclose(fp2);
	getchar();
}