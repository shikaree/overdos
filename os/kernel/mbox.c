#include <sched.h>
#include <ansi-mod.h>
#include <ansi.h>
#include <pcb.h>
#include <bitarrayops.h>

#include <mbox.h>
#include <uart.h>

#define assert(x) kprint(x);kprint("\x0d\x0a")

void kprint(char *);
void ledd( char *);

int counter=0;

mailbox mbox[MAX_MBOX];


/* ---------------------------------------------------------------- */
/* - proc_block()                                                 - */
/* - Removes a process from the scheduler and marks it blocked.   - */
/* ---------------------------------------------------------------- */
void proc_block(pin p, int mbox_nr) {
 int rear;
 
  if (pcbs[p].status==STATUS_BLOCKED) {
  return;
 }
 
 pcbs[p].status=STATUS_BLOCKED;
 hpf_delete(p);

 /* Add to end if this isn't the first process in the FIFO. */
 if (mbox[mbox_nr].blocked_count) {
  mbox[mbox_nr].blocked_rear=(mbox[mbox_nr].blocked_rear+1) % MAX_BLOCKED;
 }

 rear=mbox[mbox_nr].blocked_rear;
 mbox[mbox_nr].blocked[rear]=p;
 mbox[mbox_nr].blocked_count++;  
 
 return;
}



/* ---------------------------------------------------------------- */
/* - proc_unblock()                                               - */
/* - Returns a blocked process to the scheduler and marks it      - */
/* - running.                                                     - */
/* ---------------------------------------------------------------- */
void proc_unblock(int mbox_nr) {
 int front;
 
 if (!mbox[mbox_nr].blocked_count) {
  return;
 }

 /* Remove the front one. */
 front = mbox[mbox_nr].blocked_front;
 
 if (mbox[mbox_nr].blocked_count!=1) {
  mbox[mbox_nr].blocked_front = (mbox[mbox_nr].blocked_front+1) % MAX_BLOCKED;
 } 
 mbox[mbox_nr].blocked_count--;
    		
 pcbs[mbox[mbox_nr].blocked[front]].status=STATUS_RUNNING;
 
 hpf_insert(mbox[mbox_nr].blocked[front]);

 return;
}




/* ---------------------------------------------------------------- */
/* - mbox_translate()                                             - */
/* - Translates a mbox number to stdin/stdout if necessary. 	  - */
/* - -1 becomes stdin (of executing process).                     - */
/* - -2 becomes stdout (of executing process).                    - */
/* - Anything else just returns the same value.                   - */
/* ---------------------------------------------------------------- */
int mbox_translate(int mbox_nr, pin p) {
 
 
 switch(mbox_nr) {
  case -1: return pcbs[p].stdout;
  case -2: return pcbs[p].stdin;
  default: return mbox_nr;
 }
}




/* ---------------------------------------------------------------- */
/* - mbox_init()						  - */
/* - Initiates all mailboxes.					  - */
/* ---------------------------------------------------------------- */
void mbox_init(void) {
 int i;
 
 for (i=0;i<MAX_MBOX;i++) {
  mbox[i].active=0;
  mbox[i].number=i; 
  mbox[i].size=-1;
  mbox[i].msg_front=0;
  mbox[i].msg_rear=0;
  mbox[i].msg_count=0;
  mbox[i].counter=0;
  mbox[i].blocked_front=0;
  mbox[i].blocked_rear=0;
  mbox[i].blocked_count=0;
 }

 return;
}



/* ---------------------------------------------------------------- */
/* - mbox_name2nr()                                               - */
/* - Converts a string to a mailbox number			  - */
/* - Returns: Number of mailbox or -1 if not found.		  - */
/* ---------------------------------------------------------------- */
int mbox_name2nr(char *name) {
 int i;
 
 /* Search thru the list. */
 for (i=0;i<=MAX_MBOX;i++) {
  if (mbox[i].active) {
   if (strcmp(name,mbox[i].name)==0) {
    return i;
   }
  }
 }
 
 return -1;
}



/* ---------------------------------------------------------------- */
/* - mbox_create()                                                - */
/* - Creates and initiates a new mailbox by name. 		  - */
/* - Returns: Number of new mailbox (or already existing with     - */
/* -	      that name or -1 if no free mailboxes or error.	  - */
/* ---------------------------------------------------------------- */
int mbox_create(int size, char *name) {
 int mbox_nr, i;

 if (size > MAX_MSG || size<0) {
  return -1;
 }

 /* Check that the mbox doesn't already exist. */
 mbox_nr = mbox_name2nr(name);
 
 /* Do we already own this? */
 if (mbox_nr!=-1 && getbit(pcbs[pcb_executing].mailbox_used,mbox_nr)) {
  return mbox_nr;
 }
 
 if (mbox_nr!=-1) {
  mbox[mbox_nr].counter++;
  return mbox_nr;
 }
 
 /* Find first free mbox. */
 mbox_nr=-1;
 for (i=0;i<MAX_MBOX;i++) {
  if (!mbox[i].active) {
   mbox_nr=i;
   mbox[mbox_nr].counter++;
   break;
  } 
 }

 /* No free mboxes? */
 if (mbox_nr==-1) {
  return -1;
 } 
 
 /* Fill in data. Mbox_create is responsible of resetting the data. */
 if (strlen(name)>15) {
  strncpy(mbox[mbox_nr].name,name,15);
  mbox[mbox_nr].name[15]=0;
 } else {
  strcpy(mbox[mbox_nr].name,name);
 }
 mbox[mbox_nr].active=1;
 mbox[mbox_nr].number=mbox_nr;
 mbox[mbox_nr].size=size;
 mbox[mbox_nr].msg_front=0;
 mbox[mbox_nr].msg_rear=0;
 mbox[mbox_nr].msg_count=0;
 mbox[mbox_nr].counter=1; /* We are being used by 1 process. */
 mbox[mbox_nr].blocked_front=0;
 mbox[mbox_nr].blocked_rear=0;
 mbox[mbox_nr].blocked_count=0;
 
 return mbox_nr;
}



/* ---------------------------------------------------------------- */
/* - mbox_destroy()                                               - */
/* - Destroys a mailbox and unblocks all processes using it.      - */
/* - Returns: -1 if mailbox wasn't active or unspecified if       - */
/* -	      successfull.					  - */
/* ---------------------------------------------------------------- */
int mbox_destroy(int mbox_nr) {

 if (mbox_nr<0 || mbox_nr>=MAX_MBOX) {
  return -1;
 }

 /* Don't we own this? */
 if (!getbit(pcbs[pcb_executing].mailbox_used,mbox_nr)) {
  return -1;
 }
 
 /* Is this one active? */
 if (!mbox[mbox_nr].active) {
  return -1;
 } 

 if (mbox[mbox_nr].counter>1) {
   mbox[mbox_nr].counter--;
   return mbox_nr;
 }
 
 /* Unblock all processes using this mailbox. */
 if (mbox[mbox_nr].blocked_count) {
  while (mbox[mbox_nr].blocked_count) {
    proc_unblock(mbox_nr); 
  }
 }

 mbox[mbox_nr].active=0;
 return mbox_nr;
}



/* ---------------------------------------------------------------- */
/* - send()							  - */
/* - Sends a message to a mailbox or marks the process blocked    - */
/* - if mailbox was full.					  - */
/* - Returns:  0 - Process got blocked.				  - */
/* -	      -1 - Zero size mailbox got blocked.		  - */
/* ---------------------------------------------------------------- */
int send(char *msg,int size,int mbox_nr) {
 int front, rear;


 /* Message too big? Truncate. */
 if (size > MAX_MSG_LEN) {
  size=MAX_MSG_LEN;
 }
 
 /* Translate if necessary. */
 mbox_nr=mbox_translate(mbox_nr, pcb_executing);
 
/* skeletor(mbox[mbox_nr].msg_count);*/
 
 /* Block if necessary. */
 if (mbox[mbox_nr].msg_count>=mbox[mbox_nr].size) {
  /* Special case for 0-size mailbox. */
  if (mbox[mbox_nr].size==0) {
   if (mbox[mbox_nr].msg_count==1) {
    proc_block(pcb_executing, mbox_nr);
    return 0;
   }
  } else {
    proc_block(pcb_executing, mbox_nr);
    return 0;
  }  
 } 

 /* Any processes waiting in blocklist? */
 if (mbox[mbox_nr].blocked_count) {
  /* Ok, unblock one. */
  proc_unblock(mbox_nr);
 }
  
 /* Add the new message to the FIFO. */
 /* I do this in every FIFO becouse front = rear = 0 in the beginning. */
 if (mbox[mbox_nr].msg_count) {
  mbox[mbox_nr].msg_rear= (mbox[mbox_nr].msg_rear+1) % MAX_MSG;
 } 
 rear = mbox[mbox_nr].msg_rear;
 _memcpy((void *)(mbox[mbox_nr].msgs[rear].msg),
    	 (void *)(msg),
         size);
 mbox[mbox_nr].msgs[rear].size=size;        
 mbox[mbox_nr].msg_count++;

 /* If this is an output-request we need to try to send to generate
    interrupts.							    */
 
 if (mbox_nr==pcbs[pcb_focus].stdout) {
  if (!buffer_len) {
   /* These lines are equal to receive() but avoids blocking. */
   front = mbox[mbox_nr].msg_front;
   uart_send(mbox[mbox_nr].msgs[front].msg,
      	     mbox[mbox_nr].msgs[front].size);
   if (mbox[mbox_nr].msg_count!=1) {
    mbox[mbox_nr].msg_front=(mbox[mbox_nr].msg_front+1) % MAX_MSG;
   } 
   mbox[mbox_nr].msg_count--;
  }   
 }   

 /* ALWAYS block if size is 0. */
 if (mbox[mbox_nr].size==0) {
  proc_block(pcb_executing, mbox_nr);
  return -1;
 }

 return size;
}



/* ---------------------------------------------------------------- */
/* - receive()							  - */
/* - Receives a message from a mailbox or marks the process       - */
/* - blocked if mailbox was empty.				  - */
/* - Returns: 0 - Process got blocked.				  - */
/* ---------------------------------------------------------------- */
int receive(char *msg, int mbox_nr) {
 int front, size, shifted_mbox;
  
  
 /* Translate if necessary. */
 mbox_nr=mbox_translate(mbox_nr, pcb_executing);
 
 /* Block if necessary. */
 shifted_mbox=mbox_nr;
 if (shifted_mbox>=65536) {
  shifted_mbox -= 65536;
 }
 
 if (mbox[shifted_mbox].msg_count<1) {
  /* mbox_nr > 65534 is from uart. */
  if (mbox_nr < 65536) { 
   proc_block(pcb_executing, mbox_nr);
  } 
  return 0;
 } 

 /* Filter out uart_int() request. */
 if (mbox_nr >= 65536) {
  mbox_nr = shifted_mbox;
 }

 /* Any processes waiting in blocklist? */
 if (mbox[mbox_nr].blocked_count) {
  /* Ok, unblock one. */
  proc_unblock(mbox_nr);
 }

 /* Copy the message. */
 front = mbox[mbox_nr].msg_front;
 size=mbox[mbox_nr].msgs[front].size;
 _memcpy((void *)msg,
    	 (void *)(mbox[mbox_nr].msgs[front].msg),
         size);

 
 /* Remove the message from the fifo. */
 if ( mbox[mbox_nr].msg_count!=1) {
  mbox[mbox_nr].msg_front= (mbox[mbox_nr].msg_front+1) % MAX_MSG;
 } 
 mbox[mbox_nr].msg_count--;

 return size;
}


   	
/* ---------------------------------------------------------------- */
/* - receive_cond()						  - */
/* - Receives a message from a mailbox if it is available. 	  - */
/* - blocked if mailbox was empty.				  - */
/* - Returns: The character available.				  - */
/* -	      ERR - If the mailbox was empty.			  - */
/* ---------------------------------------------------------------- */
int receive_cond(char *msg, int mbox_nr)
{
   mbox_nr=mbox_translate(mbox_nr,pcb_executing);
   if(mbox[mbox_nr].msg_count) {
    return receive(msg,mbox_nr+65536);
   }
   return ERR;
}
