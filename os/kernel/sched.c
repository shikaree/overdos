/* ---------------------------------------------------------------- */
/* - sched.c: Scheduling functions.				  - */
/* ---------------------------------------------------------------- */

/* Description:

   The scheduler keeps track of running processes.
   It has two queues, one Highest-Priority-First and one 
   Round-Robin-queue.
      
   The scheduler will run all processes in the RR-queue
   removing them from the queue as they are executed and when
   the RR-queue is empty it is then filled with the process
   having the highest priority in the HPF-queue. If there are
   several processes with highest priority in the HPF-queue, all
   these processes are copied.                                     
   
   schedule_init(): Will set up the scheduler for the first time,
   should only be called once within the kernel.
   
   schedule(): Will mark a new process as executing. Use this before
   doing context switch. Then read the global variable 
   pcb_executing to get the pin.
   
   hpf_insert(): Will insert a new process pin into the scheduler.
   Use this when returning from blocked mode or when dispaching
   a completely new process.
   
   hpf_delete(): Will remove a process pin from the HPF-queue.
   Use this when destroying a process or when you just want to
   remove a process from scheduling. This will not, however,
   touch the process itself in any way.
      								    */

#define _PCB_EXECUTING /* No extern for this file. */
#include <sched.h>


void kprint(char *text);

struct queue hpf_queue;
struct queue rr_queue;

/* This variable is global and contains the pin for the executing
   process.							  
   Use it well. 						  */
pin pcb_executing=0;

extern pcb pcbs[];

/* Static functions. Cleans up sched.h nicely. */
static pin hpf_extract_min(void);
static void hpf2rr(void);




/* ---------------------------------------------------------------- */
/* - schedule_init()						  - */
/* - Initializes the queues.                                      - */
/* ---------------------------------------------------------------- */
void schedule_init (void)
{
 int i;
 
 for (i=0;i<MAX_PROCESSES;i++) {
  rr_queue.pins[i]=-1;
  hpf_queue.pins[i]=PIN_INVALID;
 }
 
 hpf_queue.size=0;
}




/* ---------------------------------------------------------------- */
/* - schedule()							  - */
/* - Switches execution to next process in round-robin 		  - */
/* - queue. If it is empty, it creates a new round-robin queue    - */
/* - from the HPF-queue.                                          - */
/* ---------------------------------------------------------------- */
void schedule(void) 
{

 
 /* Is the round-robin queue empty? - Fill it. */
 if (rr_queue.size<=0) {
  hpf2rr();
 }

 /* Is it still empty? */
 if (rr_queue.size<=0) {
  /* Do idleloop. */
  idleloop();
  /* This will NEVER happen!. */
  return; 
 }

 pcb_idleloop=0;

 /* Mark the last one in the rr_queue executing and remove it. */
 pcb_executing=rr_queue.pins[rr_queue.size-1];
 rr_queue.size--;
 
 return;
}




/* ---------------------------------------------------------------- */
/* - hpf_delete()						  - */
/* - Removes a pin from the HPF queue. It will not remove the pin - */
/* - from the round-robin queue, it is unnecessary as it will     - */
/* - be automatically removed by schedule().                      - */
/* ---------------------------------------------------------------- */
void hpf_delete (pin p) 
{
 int i,j;
 
 for (i=0;i<hpf_queue.size;i++) {
  if (hpf_queue.pins[i]==p) {
   for (j=i;j<hpf_queue.size;j++) {
    hpf_queue.pins[j]=hpf_queue.pins[j+1];
   }
   hpf_queue.size--;
   hpf_queue.pins[hpf_queue.size]=PIN_INVALID;
   return; 
  }
 } 

 return;
}



/* ---------------------------------------------------------------- */
/* - hpf_insert()						  - */
/* - Inserts a pin into the HPF-queue. The pin may aldready be    - */
/* - there. Use this function to put a process into the scheduler.- */
/* ---------------------------------------------------------------- */
void hpf_insert (pin p) 
{
 hpf_queue.pins[hpf_queue.size]=p;
 hpf_queue.size++;
 return;
}




/* ---------------------------------------------------------------- */
/* - STATIC FUNCTIONS						  - */
/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
/* - hpf_extract_min()						  - */
/* - Extracts the pin with highest priority from the HPF-queue.   - */
/* ---------------------------------------------------------------- */
static pin hpf_extract_min(void)
{
 int max_priority=1000000,i;
 pin pin;
 
 if (hpf_queue.size<1) {
  return -1;
 }
   
 for (i=0;i<hpf_queue.size;i++) {
  if (pcbs[hpf_queue.pins[i]].priority<max_priority) {
   max_priority=pcbs[hpf_queue.pins[i]].priority;
   pin=hpf_queue.pins[i];
  }
 }
  
 hpf_delete(pin);
   
 return pin;
}




/* ---------------------------------------------------------------- */
/* - hpf2rr()							  - */
/* - Extracts all highest priority values from the heap and puts  - */
/* - them into the round-robin queue.				  - */
/* ---------------------------------------------------------------- */
static void hpf2rr (void) {
 int top,priority,i;
 
 /* Get the highest priority pin from the heap. */
 top=hpf_extract_min();
 
 /* No such element? */ 
 if (top==-1 || top==PIN_INVALID) {
  rr_queue.size=0;
  return;
 }
 
 rr_queue.pins[0]=top;
 rr_queue.size=1;
 priority=pcbs[top].priority;
 
 /* Extract all equal priority pins. */
 do {
  top=hpf_extract_min();
  if (top==-1 || top==PIN_INVALID || pcbs[top].priority!=priority) {
   /* Put it back if valid and break. */
   if (top!=-1 && top!=PIN_INVALID) {
    hpf_insert(top);
    break;
   }
  }
  else {
   /* Add it to the rr_queue. */
   rr_queue.pins[rr_queue.size]=top;
   rr_queue.size++;
  }
 } while (top!=-1 && top!=PIN_INVALID);
 
 /* Put all of the pins in the rr_queue back into the hpf (keeping
    copies of course).                                             */
 for(i=0;i<rr_queue.size;i++) {
  hpf_insert(rr_queue.pins[i]);
 }
}



