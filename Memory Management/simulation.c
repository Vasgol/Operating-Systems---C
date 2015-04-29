#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ask2.h"


/* Getting reference from trace file */
int get_reference(int refindex, char *buf,FILE *fp)
{
   if((fseek(fp,TRACE_SIZE*refindex,SEEK_SET))!=0)
   {
		fprintf(stderr,"Cannot read file!\n");
		return -1;
   }
		    
   if(fread(buf,TRACE_SIZE,1,fp)!=1)
   {
	    fprintf(stderr,"Cannot read file\n");
	    return -1;
   }

   return 0;
}

/* Getting pagenumber and dirty parameter from a reference */
Page get_page(char *buf)
{
	Page page;
    char point; /* R or W */
    
	
	/* Reference is ||pagenum|offset|| where offset is 12 bits because |page|=2^12bytes=4Kb and pagenum=20bits=5 Hex digits */
	memcpy(page.pagenum,buf,5);    // Getting pagenum string 
	memcpy(page.pagenum+5,"\0",1); // Terminating pagenum string
	memcpy((void*)&point,buf+9,1); // Getting dirty parameter 
	/* If page is marked as W and is removed from memory it must be written back to hard disk (dirty) */
	if(point=='R')
	page.dirty=0;
	else
	page.dirty=1;

	return page;
}
