#include <sched.h>
#include <mbox.h>
#include <stacks.h>
#include <bitarrayops.h>
#include <ansi-mod.h>
#include <ansi.h>
#include <fs.h>
#include <os.h>
#include <setclock.h>


pcb pcbs[MAX_PROCESSES];
int timer_uptime_timer=-1;
int timer_uptime_seconds=0;
int timer_uptime_minutes=0;
int timer_uptime_hours=0;
int timer_uptime_days=0;

extern timer_interval;
extern timer_counter;
extern pcb_idleloop;

extern mailbox mbox[];

void kprint(char *text);

/* ---------------------------------------------------------------- */
/* - pcb_init()							  - */
/* - Initializes all pcbs.					  - */
/* ---------------------------------------------------------------- */
void pcb_init(){
  int i,j;
  
  pcb_executing=-1;
  
  for(i=0;i<MAX_PROCESSES;i++){
    pcbs[i].active = FALSE;
    for(j=0;j<MAX_PROCESSES;j++)
      resetbit(pcbs[i].children,j); 
    for(j=0;j<MAX_MBOX;j++)
      resetbit(pcbs[i].mailbox_used,j);
  }
}



/* ---------------------------------------------------------------- */
/* - proc_create()						  - */
/* - Creates a new process using name and priority.		  - */
/* - Returns:							  - */
/* -	     pin - PIN of new process.				  - */
/* -	      -1 - If the process couldn't be created.		  - */
/* ---------------------------------------------------------------- */
pin proc_create(char *name, prior pryo) {
  pin a = 0;
  int i, pcb_executing_holder;
  void *addr;
  unsigned char namn[12];
    
  addr = fs_lookup(name);  
  if(!addr) return -1; 

  /* Find next free PCB. */
  while(pcbs[a++].active) {
   if(a==MAX_PROCESSES) {
    return -1;
   }
  }
  a--;
  
  /* Set up the pcb. */
  pcbs[a].active = TRUE;
  pcbs[a].priority = pryo;
  strcpy(pcbs[a].name,name);
  pcbs[a].parent=pcb_executing;
  pcbs[a].status = STATUS_RUNNING;
  pcbs[a].stack = (void *)((int)getstackstart() + STACK_SIZE*(a+1));
  pcbs[a].reg[25]= (word)pcbs[a].stack; /* $sp */
  pcbs[a].reg[27]= (word)&exit_me;	/* Exit function in $ra. */
  pcbs[a].sleep=0;
  pcbs[a].cpu_timer=0;
  pcbs[a].cpu_usage=0;
  pcbs[a].focusswitch=(int *)0;
  
  /* Clear all mailboxes. */
  for(i=0;i<(MAX_MBOX>>3)+1;i++) {
    pcbs[a].mailbox_used[i] = 0;
  }
 
  /* Set epc. */
  pcbs[a].code = addr; 
  
  /* Clear all children. */
  for(i=0;i<(MAX_PROCESSES>>3)+1;i++) pcbs[a].children[i]=0;    
      
  /* Create a stdout mailbox. */    
  i = itoa(namn,a);
  strcat(namn,"stdout");
  pcb_executing_holder=pcb_executing;
  pcb_executing=a;
  pcbs[a].stdout=mbox_create(MAX_MSG,namn); 
  pcb_executing=pcb_executing_holder;
  if (pcbs[a].stdout==-1) {
   pcbs[a].active=FALSE;
   return -1;
  }
  
  /* Create a stdin mailbox. */
  i = itoa(namn,a);
  strcat(namn,"stdin");
  pcb_executing_holder=pcb_executing;
  pcb_executing=a;
  pcbs[a].stdin = mbox_create(MAX_MSG,namn);
  pcb_executing=pcb_executing_holder;
  if (pcbs[a].stdin==-1) {
   mbox_destroy(pcbs[a].stdout);
   pcbs[a].active=FALSE;
   return -1;
  }

  /* Mark them as used. */  
  setbit(pcbs[a].mailbox_used,pcbs[a].stdout);
  setbit(pcbs[a].mailbox_used,pcbs[a].stdin);

  if (pcb_executing!=-1) {
   setbit(pcbs[pcb_executing].children,a);
  }

  /* Go into scheduler. */
  hpf_insert(a);
  if (pcb_executing==-1) {
   pcb_focus=a;
  } else {
   proc_focus_give(a);
  } 
  return a; 
}



/* ---------------------------------------------------------------- */
/* - proc_set_priority()					  - */
/* - Changes the priority of a process.				  - */
/* - Returns:							  - */
/* -	      TRUE - If priority was changed.			  - */
/* -	     FALSE - If an error occured.			  - */
/* ---------------------------------------------------------------- */
bool proc_set_priority(pin nr, prior pryo) {
  if(pryo>256) return FALSE;
  if(pcbs[nr].active==FALSE) return FALSE;
  if(nr==-1){ 
    hpf_delete(pcb_executing);
    pcbs[pcb_executing].priority = pryo;
    hpf_insert(pcb_executing);
  }
  else {
    hpf_delete(nr);
    pcbs[nr].priority = pryo;
    hpf_insert(nr);
 
  }
  return TRUE;
}



/* ---------------------------------------------------------------- */
/* - proc_kill()						  - */
/* - Kills a process using its PIN				  - */
/* - Returns:							  - */
/* -	      TRUE - If process was killed.			  - */
/* -	     FALSE - If an error occured.			  - */
/* ---------------------------------------------------------------- */
bool proc_kill(pin nr){
  pin a;
  pin i, pcb_executing_holder;
  
  a=nr;
  if(nr == -1) a = pcb_executing;
  if(pcbs[a].active==FALSE) return FALSE;
  hpf_delete(a);
  pcbs[a].active = FALSE;
  for(i=0;i<=MAX_PROCESSES;i++){
    if(getbit(pcbs[a].children,i) == 1) {
      proc_kill(i);
     } 
  }

  pcb_executing_holder=pcb_executing;
  pcb_executing=a;
  for(i=0;i<=MAX_MBOX;i++){
    if(getbit(pcbs[a].mailbox_used,i) == 1) {
      mbox_destroy(i);
      resetbit(pcbs[a].mailbox_used,i);
    }  
  }
  pcb_executing=pcb_executing_holder;

  
  
  /* Remove us from parent. */
  if (pcbs[a].parent!=-1) {
   resetbit(pcbs[pcbs[a].parent].children,a);
  }
  
  /* Do we have focus? */
  if (a==pcb_focus) {
   /* If we exited give focus to parent orelse just take the next one. */
   if (a==pcb_executing) {
    pcb_focus=pcbs[a].parent;
    if (pcbs[pcb_focus].focusswitch) {
     *pcbs[pcb_focus].focusswitch=1;
    }
   } else {
    switch_focus();
   }
  }
  
  return TRUE;
}



/* ---------------------------------------------------------------- */
/* - proc_get_priority()					  - */
/* - Returns the priority of a process.				  - */
/* - Returns:							  - */
/* -	     priority - If call was succesfull.			  - */
/* -	    	   -1 - If an error occured.			  - */
/* ---------------------------------------------------------------- */
prior proc_get_priority(pin nr){
  prior p = -1;  
  if(nr==p) return pcbs[pcb_executing].priority;
  if(pcbs[nr].active==FALSE) return p;
  return pcbs[nr].priority;
}



/* ---------------------------------------------------------------- */
/* - proc_get_name()						  - */
/* - Looks up the name of a process with a specific PIN.	  - */
/* - Returns:							  - */
/* -	     name and TRUE - If call was successfull.		  - */
/* -	    	     FALSE - If an error occured.		  - */
/* ---------------------------------------------------------------- */
bool proc_get_name(char* name, pin nr){
  if(pcbs[nr].active == FALSE){ 
    return FALSE;
  }
  if(nr == -1){
    strcpy(name,pcbs[pcb_executing].name);
    return TRUE;    
  }

  strcpy(name,pcbs[nr].name);
  return TRUE;
}

int proc_get_parent(int p)
{
   if(p==-1)p=pcb_executing;
   if(pcbs[p].active)
      return pcbs[p].parent;
   return -1;
}


void switch_focus(){
  pin p, old_p;
  char itoabuf[12];
  
  if (pcbs[pcb_focus].focusswitch) {
   *pcbs[pcb_focus].focusswitch=2;
  } 

  p = (pcb_focus +1)%MAX_PROCESSES;
  old_p=p;
  while(pcbs[p].active==FALSE) {
   p = (p+1)%MAX_PROCESSES;
   if (p==old_p) {
    pcb_focus=-1;
    return;
   }    
  }
  pcb_focus=p;
  
  if (pcbs[pcb_focus].focusswitch) {
   *pcbs[pcb_focus].focusswitch=1;
  }
  
  kprint("\x1b[37m\x1b[40m\x1b[0m\x1b[2J\x1b[0;0H"); 
  kprint("[focus #");
  itoa(itoabuf,pcb_focus);
  kprint(itoabuf);
  kprint(" (");
  kprint(pcbs[pcb_focus].name);
  kprint(")]\n\x0d");
  
}



/* ---------------------------------------------------------------- */
/* - proc_focus_give()						  - */
/* - Gives focus to a specific process. Only the process 	  - */
/* - currently having focus may do this.			  - */
/* - Returns:							  - */
/* -	     1 (TRUE)  - If successfull.			  - */
/* -	     0 (FALSE) - If an error occured.			  - */
/* ---------------------------------------------------------------- */
int proc_focus_give(int p)
{
   if((pcb_executing!=pcb_focus)||!pcbs[p].active) return 0;
   
   if (pcbs[pcb_focus].focusswitch) {
      *pcbs[pcb_focus].focusswitch=2;
   }
   if (pcbs[pcb_focus=p].focusswitch) {
      *pcbs[pcb_focus].focusswitch=1;
   }
   return 1;
}



/* ---------------------------------------------------------------- */
/* - proc_get_status()						  - */
/* - Syscall-function. Returns status of a specific PIN.	  - */
/* - Returns:							  - */
/* -	        0 - Running					  - */
/* -	      128 - Blocked					  - */
/* -	       -1 - Illegal process.				  - */
/* ---------------------------------------------------------------- */
int proc_get_status(int p)
{

 if (p==-1) {
  p = pcb_executing;
 }

 if (p<0 || p>=MAX_PROCESSES) {
  return -1;
 }
 
 if (!pcbs[p].active) {
  return -1;
 }
 
 return pcbs[p].status;
}



/* ---------------------------------------------------------------- */
/* - exit_me()							  - */
/* - Function that every process returns to at exit.		  - */
/* - $ra is set to point to this function initially.		  - */
/* ---------------------------------------------------------------- */
void exit_me() 
{
 kill_process(-1);  
 for(;;);
 return;
}



/* ---------------------------------------------------------------- */
/* - process_timer()						  - */
/* - Function to update various timers.				  - */
/* ---------------------------------------------------------------- */
void process_timer (void) {
 static unsigned int microsecond=0;
 static int ticks;
 
 int i, c;

 /* Update uptimer. */
 c=readclock_sec();
 if (timer_uptime_timer == -1) {
  timer_uptime_timer = c;
 }
 
 if (timer_uptime_timer != c) {
  timer_uptime_timer=c;
  timer_uptime_seconds++;
  if (timer_uptime_seconds>=60) {
   timer_uptime_seconds=0;
   timer_uptime_minutes++;
   if  (timer_uptime_minutes>=60) {
    timer_uptime_minutes=0;
    timer_uptime_hours++;
    if (timer_uptime_hours >=24) {
     timer_uptime_hours=0;
     timer_uptime_days++;
    }
   }
  } 
 }


  
 /* Calculate CPU-usage for processes. */ 
 microsecond+=timer_interval;
 
 if (microsecond>=2000000) {
  ticks=microsecond/timer_interval;  
 }

   
 for(i=0;i<MAX_PROCESSES;i++) {
  if (pcbs[i].active) {
   /* Update CPU every 2 seconds. */
   if (microsecond>=2000000) {
    pcbs[i].cpu_usage=((1000000*pcbs[i].cpu_timer)/(ticks));
    pcbs[i].cpu_timer=0;
   } 
   /* Check for sleep every tick. */
   if (pcbs[i].sleep) {
    pcbs[i].sleep--;
    /* Unblock if time elapsed. */
    if (!pcbs[i].sleep) {
     pcbs[i].status=STATUS_RUNNING;
     hpf_insert(i);
    }
   }
  }  		
 } 

 if (microsecond>=2000000) {
  microsecond=0;
 }
  
 if (pcbs[pcb_executing].status==STATUS_RUNNING) {
  pcbs[pcb_executing].cpu_timer++;
 } 
 
 return;
}



/* ---------------------------------------------------------------- */
/* - timer_uptime()						  - */
/* - Syscall-function.						  - */
/* - Returns:							  - */
/* -	     Fills in uptime-information into the int-buffers.	  - */
/* ---------------------------------------------------------------- */
void timer_uptime (int *days, int *hours, int *minutes) 
{

 *days = timer_uptime_days;
 *hours = timer_uptime_hours;
 *minutes = timer_uptime_minutes;

 return;
}



/* ---------------------------------------------------------------- */
/* - process_sleep()						  - */
/* - Syscall-function. Delays the current executing process       - */
/* - (indicated by pcb_executing) a number of timer ticks.	  - */
/* ---------------------------------------------------------------- */
void process_sleep(int t) {
 
 if (t<=0) {
  return;
 }
 
 pcbs[pcb_executing].sleep=t;
 pcbs[pcb_executing].status=STATUS_BLOCKED;
 hpf_delete(pcb_executing);
 return;
}
          
