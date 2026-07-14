#include <pcb.h>
#include <ansi-mod.h>
#include <ansi.h>
#include <os.h>
#include <mbox.h>
#include <bitarrayops.h>

static void top_redraw();
static int top_compar(const void *pcb1, const void *pcb2);


/* PCBINFO takes less space than pcbs. */
typedef struct PCBINFO {
 int pin;
 int parent;
 int status;
 int mailboxes;
 unsigned int cpu_usage;
 int mem;
 char name[10];
 int priority;
} PCBINFO;




/* ---------------------------------------------------------------- */
/* - top()							  - */
/* - TOP - The wonderful and beautiful.				  - */
/* ---------------------------------------------------------------- */
void main_top(){
 int i,j;
 struct PCBINFO pcbinfo[MAX_PROCESSES];
 char *status_strings[]={"RUNNING","BLOCKED","UNKNOWN"};
 char *status;
 int pcbinfos=0;
 int focusswitch=0;
 int idle_cpu, processes, mailboxes, messages,blocked, running;
 int up_days, up_hours, up_minutes, led_counter=3;
 char ledbuf[128];
  
 register_focusswitch(&focusswitch);
  
 top_redraw();
     
 for(;;) {    	
 
  printf("%@",xy(1,6));
  
  if (focusswitch==1) {
   focusswitch=0;
   top_redraw();
  }
  
  /* Copy all PCBs at once. */
  pcbinfos=0;	
  running=0;
  blocked=0;	
  idle_cpu=0;
  processes=0;
  for(i=0;i<MAX_PROCESSES;i++){
   if (pcbs[i].active) {
    processes++;
    pcbinfo[pcbinfos].pin=i;
    pcbinfo[pcbinfos].parent=pcbs[i].parent;
    pcbinfo[pcbinfos].status=pcbs[i].status;
    pcbinfo[pcbinfos].mem = (int)pcbs[i].stack - pcbs[i].reg[25];
    /* More than 65536 is error since max stack size is just that. */
    if (pcbinfo[pcbinfos].mem>99999) {
     pcbinfo[pcbinfos].mem=99999;
    }
    pcbinfo[pcbinfos].cpu_usage=pcbs[i].cpu_usage;
    idle_cpu+=pcbs[i].cpu_usage;
    strcpy(pcbinfo[pcbinfos].name,pcbs[i].name);
    pcbinfo[pcbinfos].priority=pcbs[i].priority;
    pcbinfos++;
   }
   else {
    pcbinfo[i].cpu_usage=0;
   }
  }

  /* Quicksort processes. */
  qsort(pcbinfo,pcbinfos,sizeof(struct PCBINFO),top_compar);  
    
  for (i=0;i<pcbinfos;i++) {
   /* Get number of used mailboxes. */
    pcbinfo[i].mailboxes=0;
   for (j=0;j<MAX_MBOX;j++) {
     if (getbit(pcbs[pcbinfo[i].pin].mailbox_used,j)) {
       pcbinfo[i].mailboxes++;
     }  
   }
   if (getch()!=ERR) {
    return;
   }
    /* Convert status to a string. */
    switch(pcbinfo[i].status) {
     case 0:
      status=status_strings[0];
      break;
     case 128:
      status=status_strings[1];
      break;
     default:
      status=status_strings[2];
      break;
    }
 
    /* Every line. */
    printf("  %.3i  %.3i  %.7s  %-.10s  %.3i  %.4i  %.5i  %.3i.%-.02i%%  \n\x0d",
           pcbinfo[i].pin,
           pcbinfo[i].parent,
           status,
           pcbinfo[i].name,
           pcbinfo[i].priority,
           pcbinfo[i].mailboxes,
           pcbinfo[i].mem,
           pcbinfo[i].cpu_usage/10000,
           pcbinfo[i].cpu_usage-10000*(pcbinfo[i].cpu_usage/10000));               
    if (i==18) {
         break;
    }
  }

  /* Calculate running/blocked processes. */
  for (i=0;i<MAX_PROCESSES;i++) {
   if (pcbs[i].active) {
    if (pcbs[i].status==0) {
     running++;
    } else if (pcbs[i].status==128) {
     blocked++;
    } 
   }
  }

  /* Calculate idle CPU. This is ugly - I KNOW!!! */
  idle_cpu=1000000-idle_cpu;
  
  /* Calculate number of used mailboxes and messages. */
  mailboxes=0;
  messages=0;
  for (i=0;i<MAX_MBOX;i++) {
   if (mbox[i].active) {
    mailboxes++;
    messages+=mbox[i].msg_count;
   }
  }
  
  /* Top line. */
  cprintf("%@%i Processes: %i run, %i blck, %i mbox, %i msg. %i.%-.02i%% idle.           ",
     	  xy(1,1), processes, running, blocked, mailboxes, messages,
          idle_cpu/10000,
          idle_cpu-10000*(idle_cpu/10000));
  /* Clock up to the right. */          
  cprintf("%@%.02i:%.02i:%.02i\n\x0d",xy(72,1),
          rclock_hour(), rclock_min(), rclock_sec());  
    
  get_uptime(&up_days,&up_hours,&up_minutes);
  /* Led every 14 seconds. */
  led_counter++;
  if (led_counter==7) {
   led_counter=0;
   sprintf(ledbuf,"    [TOP] Uptime: %id %ih %im    ",up_days,up_hours,up_minutes);
   led(ledbuf,strlen(ledbuf));
  }   

  /* Print uptime. */
  cprintf("Uptime: %i days %i hours %i minutes.    \n",
          (int)up_days, 
          (int)up_hours,
          (int)up_minutes);
          
  cprintf("%@",xy(1,4));    		
  if (getch()!=ERR) {
   /* Flush output. */
   sleep(500000);
   cprintf("\n\n\x0d");
   return;
  }

  sleep(2000000);
 
 }  
}



/* ---------------------------------------------------------------- */
/* - top_redraw()						  - */
/* - Redraws tops screen at init and when top gets focus back.    - */
/* ---------------------------------------------------------------- */
static void top_redraw() 
{
 cprintf("%1%2%3",7,0,0);
 cprintf("%0%@",xy(1,4));
 cprintf("  PIN  PAR  STATUS   NAME        PRI  MBOX  STACK   CPU\n\x0d");
 cprintf("----------------------------------------------------------\n\x0d");
}



/* ---------------------------------------------------------------- */
/* - top_compar()						  - */
/* - Comopare-function for qsort.				  - */
/* ---------------------------------------------------------------- */
static int top_compar(const void *pcb1, const void *pcb2) 
{
 struct PCBINFO *pcbinfo1, *pcbinfo2;
 
 pcbinfo1=(struct PCBINFO *)pcb1;
 pcbinfo2=(struct PCBINFO *)pcb2;

 if (pcbinfo1->cpu_usage>pcbinfo2->cpu_usage) {
  return -1;
 } else if (pcbinfo1->cpu_usage<pcbinfo2->cpu_usage) {
  return 1;
 }
 if (pcbinfo1->status<pcbinfo2->status) {
  return -1;
 } else if (pcbinfo1->status>pcbinfo2->status) {
  return 1;
 }
 
 
 return 0;
}


