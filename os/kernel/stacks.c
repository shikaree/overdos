#include <pcb.h>

bool stack_used[MAX_PROCESSES];

/* ---------------------------------------------------------------- */
/* - stackinit()                                                  - */
/* - Initializes stackarray.                                      - */
/* ---------------------------------------------------------------- */
void stackinit(void) {
  int i;
 for (i=0;i<MAX_PROCESSES;i++) {
   stack_used[i]=FALSE;
 } 
}


/* ---------------------------------------------------------------- */
/* - getstackstart()						  - */
/* - Keeps an array of stacks.					  - */
/* - Returns:							  - */
/* -	      Pointer to start of stack array.     		  - */
/* ---------------------------------------------------------------- */
void *getstackstart(void) {
 static char stack_stack[(MAX_PROCESSES+1)*STACK_SIZE];
 char *ptr;
 ptr = stack_stack;
 
 while ((int)ptr&7) ptr++; 
 return (void *)ptr;
}


/* ---------------------------------------------------------------- */
/* - getfreestack()						  - */
/* - Finds next free stack.					  - */
/* - Returns:							  - */
/* -	      stack number - If there was a free stack.           - */
/* - 	 		-1 - If no free stack was found.	  - */
/* ---------------------------------------------------------------- */
int getfreestack(void) {
 int i;
 for (i=0;i<MAX_PROCESSES;i++) {
   if (!stack_used[i]) {
    stack_used[i]=1;
    return i;
   }
 } 
 return -1;
}
