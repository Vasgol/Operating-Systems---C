#include <stdio.h>

/* Trace Files */
#define GCC			"gcc.trace"
#define BZIP		"bzip.trace"

#define PM	    0   /* Page in Memory */
#define PF	   -1   /* Page Fault */

#define TRACE_SIZE  11			/* |Page|=4K=2^12bytes -> every trace 12 bits*/
#define MAX_REFS	2000002		/* Defining maximum references from both files */

int clock;

/* Statistics Counters */
int readcnt , writecnt, pfcnt; 

/* Page */
typedef struct{
	char pagenum[6];
	int	   pid;
	int dirty;
}Page;

/* Nodes of hash table */
struct Node{
    Page   page;
	struct Node *next;
};

typedef struct Node HashNode;

/* HashTable definition */
typedef struct{
	 int hashsize;
	 HashNode** nodes;
	 int fcounter;
}HashTable;

HashTable hashtable;


/* LFU algorithm struct */
typedef struct{
	 Page page;  // Page to be victimized by lfu
	 int usedcnt;	// Frequently used counter
	 int time;
}LFU;

/* LFU table definition (holding every page with its used counter) */
LFU *lfu;

/* WS algorithm struct */
typedef struct{
	  Page page;  // Page in Working Set of a process
	  int time;   // Time of a page in working set
	  int backup; // Used to restore page removed from a working set of a process
}WS;

WS *window;
WS *backup;


/* Reading Trace Files and getting pages */
int get_reference(int refindex, char *buf, FILE *fp);
Page get_page(char* buf);

/* Hash functions */
int hash_create(int size);
int hash_function(char* pagenum);
int hash_search(Page *page);
int hash_insert(Page page);
int hash_remove(Page page);
void hash_destroy();

/* LFU functions */
void LFU_Init(int size);
int  LFU_Insert(Page page, int size);
int  LFU_Update(Page page, int size);
Page LFU_victim(int maximum, int size);
void LFU_aging(int size);

/* WS functions */
void WS_Init(int size);
int WS_Search(int size, int ws_size, Page page, int time);
int WS_Insert(int size, int ws_size, Page page, int time);
void WS_Shifting(int size, int pid);
int WS_Backup(int size, int ws_size, Page page, int time);
void WS_Restore(int ws_size);
void WS_Print(int size, int pid);
void WS_Backup_print(int ws_size);


