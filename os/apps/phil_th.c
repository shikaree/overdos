/*
   Dining philosopher thread. 
   MUST be started from main philosopher program!
*/   
#include <os.h>
#include <ansi.h>
#include <ansi-mod.h>

int main_phil_th (void) 
{
 int mbox_nr[2];
 int mbox_comm;
 int my_number;
 int parent;
 char msg[256];
 char name[16];
 char output_template[32];
 char itoabuf[12];
 char msg_go_ahead[2];
 
 
 /* Give main process focus. */
 parent=get_parent(-1);
 give_focus(parent);
 
 /* Get the communications mailbox. */
 sprintf(name,"phil_%i",parent);
 mbox_comm=create_mbox(8,name);
 
 /* Get our number. */
 receive_msg(msg,mbox_comm);
 my_number=msg[0];

 /* Create two mailboxes for us. */
 sprintf(name,"ph_%i_%i",parent,my_number);
 mbox_nr[0]=create_mbox(8,name);

 sprintf(name,"ph_%i_%i",parent,(my_number+1)%5);
 mbox_nr[1]=create_mbox(8,name);

 /* Create output template. */
 strcpy(output_template, "#");
 itoa(itoabuf,my_number);
 strcat(output_template, itoabuf);
 strcat(output_template,": ");
 
 /* Start thinking/eating. */
 msg_go_ahead[0]='G';
 send_msg(msg_go_ahead,1,mbox_nr[0]);
 while(1) {
  /* --- Thinking --- */
  
  /* Output and sleep */
  strcpy(msg,output_template);
  strcat(msg,"Thinking");
  send_msg(msg,strlen(msg),mbox_comm);
  sleep(rand()%1000000+6000);
   
  /* --- Eating --- */

  /* Wait for go ahead and sleep. */
  receive_msg(msg,mbox_nr[0]);
  receive_msg(msg,mbox_nr[1]);

  /* Output */  
  strcpy(msg,output_template);
  strcat(msg,"Eating");
  send_msg(msg,strlen(msg),mbox_comm);
  sleep(rand()%1000000+6000);

  /* Tell other processes to go ahead. */
  send_msg(msg_go_ahead,1,mbox_nr[0]);
  send_msg(msg_go_ahead,1,mbox_nr[1]);
 }
 
 
 return 1;
}



