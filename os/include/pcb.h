#ifndef _PCB_H
#define _PCB_H
#include <mbox.h>

#define TRUE 1
#define FALSE 0

#define STATUS_RUNNING 0
#define STATUS_BLOCKED 128

typedef signed int prior;
typedef int word;
typedef int pin;
typedef unsigned char bool;


pin pcb_focus;


/*-------------------------------PCB----------------------------
  The struct pcb contains:
  pin - Process Idefication Number
  name - The process name, at most MAX_PROCESS_NAME_LENGHT chars
         long
  priority - For scheduling
  reg[] - An array containg the processor's registers values, not
          inluding the kernel registers
  active - TRUE or FALSE, wether this pcb is "representing" a program
           right now or not.
  status - ready, blocked or executing
  stack - a pointer to the stack the process is to use
  code - a pointer to the code for the program...
  stack_nr - the number of the stack (the stack's index in the array of 
             stacks)
  mailbox_used - a "bit array", a 1 on position n in the array means 
                 that the process uses mailbox n
  children - a "bit array", a 1 on position n means that process with pin
             n is this process's child
  stdin - the number of the mailbox used by a process for input
  stdout - dito output.
  ---------------------------------------------------------------*/

typedef struct pcb {
  word reg[29];
  void *code;
  unsigned int status;
  int *focusswitch;
  int sleep;
  pin pin;
  pin parent;
  unsigned  char name[10];
  unsigned  char priority;
  unsigned int cpu_timer;
  unsigned int cpu_usage;
  bool active;
  void *stack;
  unsigned char stack_nr;
  unsigned char mailbox_used[(MAX_MBOX>>3)+1]; 
  unsigned char children[(MAX_PROCESSES>>3)+1];
  int stdin;
  int stdout;
} pcb;

/* This is the array of PCB's, called pcbs[], a PCB's position in this array
   will also be that process' pin */

pcb pcbs[MAX_PROCESSES];

pin proc_create(char*, prior);

bool proc_kill(pin);

bool proc_set_priority(pin, prior);

prior proc_get_priority(pin);

bool proc_get_name(char*, pin);

int proc_get_parent(int p);

void pcb_init();

void switch_focus();

int proc_focus_give(int p);

void exit_me();

void process_timer (void);

void process_sleep(int t);

void timer_uptime (int *days, int *hours, int *minutes);

int proc_get_status(int p);

#endif






