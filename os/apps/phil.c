/*
   Dining philosophers
*/
#include <ansi-mod.h>
#include <ansi.h>
#include <os.h>

#define PHIL_VERSION "0.1.0"

int main_phil(void) {
 int mbox_comm;
 int length,i;
 char msg[256];
 char name[16];
 int focusswitch=0;
 int p;
 
 /* Register a focusswitch. */
 register_focusswitch(&focusswitch);
 
 cprintf("Dining philosophers version %s\n\n\x0d",PHIL_VERSION);
 
 /* Create a communications mailbox. */
 p=get_pin();
 sprintf(name,"phil_%i",p);
 mbox_comm=create_mbox(8,name);
 
 if (mbox_comm==-1) {
  cprintf("Error creating communications mailbox.\n\x0d");
  while(getch()==ERR);
  return 0;
 }
 
 /* Send out numbers to threads and create them. */
 cprintf("Assigning numbers to threads...\n\x0d");
 for (i=0;i<5;i++) {
  msg[0]=i;
  send_msg(msg,1,mbox_comm);
 } 

 cprintf("Starting threads...\x0d\n");
 for (i=0;i<5;i++) {
  if(create_process("phil_th",0)==-1) {
   cprintf("Error creating threads.\n\x0d");
   while(getch()==ERR);
   return 0;
  }
 } 
 cprintf("Waiting for sync...\x0d\n");
 
 /* Wait for all processes to get their data. */
 sleep(3000000);
 
 cprintf("Waiting for output data from threads...\n\n\x0d");
 /* Receive comm-data from threads. */
 while(getch()==ERR) {
  length=receive_msg(msg,mbox_comm);
  msg[length]=0;
  
  /* Print out only if we have focus (to stop us from being blocked) */
  if (focusswitch==1) {
   cprintf("%s\n\x0d",msg);
  }  
 }
 
 return 1;
}
