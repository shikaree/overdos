/* ---------------------------------------------------------------- */
/* - sched.h: Scheduling declarations.				  - */
/* ---------------------------------------------------------------- */
#ifndef _SCHED_H
#define _SCHED_H
#include <pcb.h>

/* General queue. */
typedef struct queue {
 pin pins[MAX_PROCESSES+1];
 int size;
} queue;

/* Use hpf_insert() to insert a NEW process into the sheduler! 
   Schedule() changes active (executing) process.              */

void schedule_init (void);
void shedule(void);
void hpf_insert (pin p);
void hpf_delete (pin p);
void idleloop(void);

extern pin pcb_executing;
int pcb_idleloop;
 

/* Should this really be here? */
#define PIN_INVALID 0xFFFFFF
 
#endif
