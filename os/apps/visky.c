/* ---------------------------------------------------------------- */
/* - Visky - Destilled Bärs					  - */
/* - 								  - */
/* - Alternate command interpreter for ÖverDOS.			  - */
/* ---------------------------------------------------------------- */
#include <ansi-mod.h>
#include <ansi.h>
#include <os.h>
#include <fs.h>
#include <pcb.h>

#define VISKY_VERSION "0.1.0"

extern unsigned char *files[];

unsigned char *cmd_internal[]={"ls","kill","pez","däcka","help",
                               "get","set","killall","clear",
                               "reboot","bomb","exit",0};


int main_visky (void) 
{
 char cmdline[READLINE_SCREEN_WIDTH+1]; /* you never know... */
 char argument[READLINE_SCREEN_WIDTH+1];
 char argument2[READLINE_SCREEN_WIDTH+1];
 char *tabcomp[32];
 char name[64];
 int tabcomp_count, dirlength,i, pin_nr, priority, kill_count;
 int cmd_internal_count, cmd;
 int year,month,day,hour,minute,second;
                              

 /* Show initial greeting. */
 printf("\n\x0dVisky - Destilled Bärs version %s\n\x0d",VISKY_VERSION);
 printf("\n\x0d");

 
 /* Set up for tab-completion. */
 tabcomp_count=0;
 while (cmd_internal[tabcomp_count]) {
  tabcomp[tabcomp_count]=cmd_internal[tabcomp_count];
  tabcomp_count++;
 }
 cmd_internal_count=tabcomp_count;
 
 /* Add program names. */
 dirlength=0;
 while (files[dirlength]) {
  tabcomp[tabcomp_count]=(unsigned char *)files[dirlength];
  dirlength++;
  tabcomp_count++;
 }
 
 /* End tab-completion. */
 tabcomp[tabcomp_count]=0;
 
 /* Main loop. */
 for (;;) {
  
  readline(cmdline,"visky@ÖverDOS> ",tabcomp);
  
  argument[0]=0;
   
  /* Get first argument. */
  i=readline_getarg(argument,cmdline);
  
  /* Search in internal commands/ */
  cmd=-1;
  for (i=0;i<cmd_internal_count;i++) {
   if (!strcmp(cmd_internal[i],argument)) {
    cmd=i;  
    break;
   }
  }

  
  /* Process internal commands. -1 means to execute program. */
  switch (cmd) {
   case 0: /* Ls. */
    for (i=0;i<dirlength;i++) {
     printf("%s\n\x0d",files[i]); 
    }
    break;
   case 1: /* Kill. */
    i=readline_getarg(argument,cmdline);
    if (!i || !isdigit(argument[0])) {
     printf("Error, you must specify a PIN to kill.\n\x0d");
     break;
    } 
    i=atoi(argument);
    if (!kill_process(i)) {
     printf("Unable to kill process with PIN %i\n\x0d",i);
    }
    break;
   case 2:  /* Pez. */
    printf("PIN  PAR  STATUS   NAME        PRI\n\x0d");
    for (i=0;i<MAX_PROCESSES;i++) {
     if (get_name(name,i)) {
      printf("%.3i  %.3i  %.7s  %-.10s  %.3i\n\x0d",
             i,get_parent(i),(get_state(i)==0 ? "RUNNING" : "BLOCKED"),name,get_priority(i));
     }
    }
    break;
   case 3: /* Däcka. */
    *((int *)0)=0;
    break;
   case 4: /* Help. */
    printf("Visky - Destilled Bärs version %s\n\x0d",VISKY_VERSION);
    printf("\n\x0d");
    printf("Commands:\n\x0d");
    printf(" bomb <name> <prior>              - Starts as many processes as possible.\n\x0d");
    printf(" clear                            - Clears and resets the screen.\n\x0d");
    printf(" däcka                            - Hang Visky (traditional).\n\x0d");
    printf(" get date                         - Shows current date and time.\n\x0d");    
    printf(" get priority <PIN>               - Get priority of process with PIN.\n\x0d");
    printf(" kill <PIN>                       - Kills process with specified PIN.\n\x0d");
    printf(" killall <name>                   - Kills all processes matching name.\n\x0d");
    printf(" ls                               - Show program listing.\n\x0d");    
    printf(" pez                              - Show active processes.\n\x0d");
    printf(" reboot                           - Reboots the system.\n\x0d");
    printf(" set priority <PIN> <prior>       - Change priority for process with PIN.\n\x0d");
    printf(" set date <Y> <M> <D> <H> <M> <S> - Set current date and time.\n\x0d");
    printf("\n\x0d");
    break;
   case 5: /* Get. */
    i=readline_getarg(argument,cmdline);
    if (!i) {
     break;
    }
    /* Priority. */
    if (!strcmp(argument,"priority")) {
     i=readline_getarg(argument,cmdline);
     if (!i || !isdigit(argument[0])) {
      printf("Error, you must specify a PIN query.\n\x0d");
      break;
     } 
     pin_nr=atoi(argument);
     priority=get_priority(pin_nr);
     printf("Current priority for PIN %i is %i.\n\x0d",pin_nr,priority);
     break;
    /* Date. */
    } else if (!strcmp(argument,"date")) {
     year = rclock_year();
     month = rclock_month();     
     day = rclock_day_month();     
     hour = rclock_hour();     
     minute = rclock_min();          
     second = rclock_sec();     
     printf("%.02i-%.02i-%.02i %.02i:%.02i:%.02i\n\x0d",
            year,month,day,hour,minute,second);
     break; 
    }
    break;
   case 6: /* Set */ 
    i=readline_getarg(argument,cmdline);
    if (!i) {
     break;
    }
    /* Priority. */
    if (!strcmp(argument,"priority")) {
     i=readline_getarg(argument,cmdline);
     if (!i || !isdigit(argument[0])) {
      printf("Error, you must specify a PIN to change priority for.\n\x0d");
      break;
     } 
     pin_nr=atoi(argument);
     i=readline_getarg(argument,cmdline);
     if (!i || !isdigit(argument[0])) {
      printf("Error, you must specify a priority.\n\x0d");
      break;
     } 
     priority=atoi(argument);
     if (!set_priority(pin_nr,priority)) {
      printf("Error, unable to change priority for PIN %i to %i\n\x0d",pin_nr,priority);
     }
     break;
    /* Date. */ 
    } else if (!strcmp(argument,"date")) {
     i=readline_getarg(argument,cmdline);
     if (!i) {
      printf("Illegal date.\n");
      break;
     }
     year=atoi(argument);
     i=readline_getarg(argument,cmdline);
     if (!i) {
      printf("Illegal date.\n");
      break;
     }
     month=atoi(argument);
     i=readline_getarg(argument,cmdline);
     if (!i) {
      printf("Illegal date.\n");
      break;
     }
     day=atoi(argument);
     i=readline_getarg(argument,cmdline);
     if (!i) {
      printf("Illegal date.\n");
      break;
     }
     hour=atoi(argument);
     i=readline_getarg(argument,cmdline);
     if (!i) {
      printf("Illegal date.\n");
      break;
     }
     minute=atoi(argument);
     i=readline_getarg(argument,cmdline);
     if (!i) {
      printf("Illegal date.\n");
      break;
     }
     second=atoi(argument);
     
     wclock_year(year);
     wclock_month(month);
     wclock_day_month(day);
     wclock_hour(hour);
     wclock_min(minute);
     wclock_sec(second);                         
     break;
    } 
    break;
   case 7: /* Killall. */
    i=readline_getarg(argument,cmdline);
    if (!i) {
     printf("Error, you must specify program name to query for kill.\n\x0d");
     break;
    } 
    kill_count=0;
    for (i=0;i<MAX_PROCESSES;i++) {
    
    
     if (get_name(name,i) && !strcmp(argument,name)) {
      printf("%.3i %s - ",i,name);
      if (!kill_process(i)) {
       printf("still alive (error).\n\x0d");
      } else {
       printf("dead.\n\x0d");
      }
     }
    }    
    break;
   case 8: /* Clear. */
    printf("%1%2%3%0%@",7,0,0,xy(1,1));
    break;
   case 9: /* Reboot. */
    reboot();
    break;  
   case 10: /* Bomb. */ 
    i=readline_getarg(argument,cmdline);
    if (!i) {
     printf("Error, you must specify program to bomb with.\n\x0d");
     break;
    }
    priority=0;
    i=readline_getarg(argument2,cmdline);
    if (i && !isdigit(argument2[0])) {
     printf("Illegal priority.\n\x0d");
     break;
    }
    priority=atoi(argument2);
    i=0;
    while (create_process(argument,priority)!=-1) {
     i++;   
    }
    printf("%i processes started.\n\x0d",i);
    break;
   case 11: /* Exit */
    return 0; 
   default:
    if (argument[0]!=0) {
     priority=0;
     i=readline_getarg(argument2,cmdline);
     if (i) {
      priority=atoi(argument2);
     } 
     if (i && !isdigit(argument2[0])) {
      printf("Illegal priority.\n\x0d");
      break;
     }
     if ((pin_nr=create_process(argument,priority))==-1) {
      for (i=0;i<dirlength;i++) {
       if (!strcmp(files[i],argument)) {
        printf("Unable to start process.\n\x0d");       
        break;
       }
      }
      if (i==dirlength) { 
       printf("Bad command or file name.\n\x0d");
      }       
     }
    } 
    break;
  }
  
  
 }


 return 0;
}




