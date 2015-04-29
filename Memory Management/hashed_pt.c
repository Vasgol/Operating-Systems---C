#include "ask2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Creating hashed page table */
int hash_create(int size)
{

	int i;

	/* Allocating hashed page table heads for every bucket */
	if((hashtable.nodes=malloc(size*sizeof(HashNode*)))==NULL)
	return -1;
	
	/* All buckets are empty when creating hash table */
	for(i=0;i<size;i++)
	hashtable.nodes[i]=NULL;

	/* Assigning hashtable's size */
	hashtable.hashsize=size;
	/* Initializing frame counter. Hashed page table capacity must not exceed memory's frames */ 
	hashtable.fcounter=0;

	return 0;
}

/* Hash function which adds the ascii codes of pagenum string */
int hash_function(char* pagenum)
{
	int i;
    int hash=0;
	int num;

	/* Taking the sum of ascii codes of every pagenum string */
	for(i=0;i<5;i++)
	hash+=pagenum[i];

	/* Returning the position of the hash table where this pagenum is located using mod */
	return hash%hashtable.hashsize;
}

/* Searching for a page of a process in hash table and update its dirty parameter 
   Getting the page if found in hashed page table to be used by LFU or WS algorithm */
int hash_search(Page *page)
{
	HashNode *node;
    int hash;

	/* Getting hash value from hash function */
	hash=hash_function((*page).pagenum);

	/* Getting node of hashtable */
	node=hashtable.nodes[hash];

	while(node!=NULL)
	{
		/* Found page in hashed page table */
		if(!strcmp((*page).pagenum,node->page.pagenum) && node->page.pid==(*page).pid)
		{
			node->page.dirty|=(*page).dirty; /* Updating dirty value */
			*page=node->page; // Getting page from Hashed Page Table 
			return 0;
		}
		
		node=node->next;
	}

	/* Page is not in memory (page fault) */
	return PF;
}

/* Inserting a page of a process (trace file) into hashed page table (because of page fault) */
int hash_insert(Page page)
{

	HashNode *node;
	int hash;

	/* Getting hash value from hash function */
	hash=hash_function(page.pagenum);

	/* Allocating memory for a new page */
	if((node=malloc(sizeof(HashNode)))==NULL)
	return -1;

	/* Inserting node in the beggining of the list */
	node->page.dirty=page.dirty;
	node->page.pid=page.pid;
	strcpy(node->page.pagenum,page.pagenum);
	node->next=hashtable.nodes[hash];
	hashtable.nodes[hash]=node;

	hashtable.fcounter++;

	return 0;
}

/* Removing a page (victimization) from hashed page table (used for replacement because of a page fault) */
int hash_remove(Page page)
{

	HashNode *node,*prevnode=NULL;
	int hash;

	/* Getting hash value from hash function */
	hash=hash_function(page.pagenum);

	/* Getting node of hashtable */
	node=hashtable.nodes[hash];

	while(node!=NULL)
	{
		if(!strcmp(page.pagenum,node->page.pagenum) && node->page.pid==page.pid)
		{
			if(prevnode!=NULL)
			prevnode->next=node->next;
			else
			hashtable.nodes[hash]=node->next;

			free(node);
			
			hashtable.fcounter--;

			
			return 0;
		}

		prevnode=node;
		node=node->next;
	}

	/* Failed to remove the victim page from memory */
	return -1;
}

/* Destroys the hashed page table */
void hash_destroy()
{
	int i;
	HashNode *node , *oldnode;

	/* Free nodes of hashtable */
	for(i=0;i<hashtable.hashsize;i++)
	{
		node=hashtable.nodes[i];

		while(node!=NULL)
		{
			oldnode=node;
			node=node->next;
			free(oldnode);
		}
	}

	/* Free buckets of hashtable */
	free(hashtable.nodes);
}




