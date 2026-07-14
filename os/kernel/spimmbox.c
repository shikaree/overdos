#include <mbox.h>
#include <pcb.h>
#include <sched.h>

/* ---------------------------------------------------------------- */
/* - check_stdin()						  - */
/* - Checks if stdin of process p in non-empty.		 	  - */
/* - Returns: 1 if not empty.					  - */
/* -	      0 if empty.					  - */
/* ---------------------------------------------------------------- */
int check_stdin(pin p)
{
   if(mbox[pcbs[p].stdin].msg_count)
      return 1;
   return 0;
}

/* ---------------------------------------------------------------- */
/* - check_stdout()						  - */
/* - Checks if stdout of process p in non-full.		 	  - */
/* - Returns: 1 if not full.					  - */
/* -	      0 if full.					  - */
/* ---------------------------------------------------------------- */
int check_stdout(pin p)
{
   if(mbox[pcbs[p].stdout].msg_count<mbox[pcbs[p].stdout].size)
      return 1;
   return 0;
}

/* ---------------------------------------------------------------- */
/* - spim_send_char()						  - */
/* - Sends a single character to the output mailbox of process p. - */
/* ---------------------------------------------------------------- */

void spim_send_char(int c,int p)
{
   char b[2],l=1;
   b[0]=(char)c;
   if(b[0]=='\n') b[(int)l++]='\x0d';
   send(b,l,pcbs[p].stdout);
}

/* ---------------------------------------------------------------- */
/* - spim_rec_char()						  - */
/* - Receives a single character from the input mailbox of 	  - */
/* - process p.							  - */
/* - Returns: The character received.				  - */
/* ---------------------------------------------------------------- */

int spim_rec_char(int p)
{
   char b[10];
   receive(b,pcbs[p].stdin);
   return (int)b[0];
}

/* ---------------------------------------------------------------- */
/* - spim_failed()						  - */
/* - Alerts user when spim emulation is causing mayhem.		  - */
/* ---------------------------------------------------------------- */

void spim_failed(void)
{
   static char fail[]="Spim emulation is failing (badly!).\n\x0d";
   if(check_stdout(pcb_executing))
      send(fail,38,pcbs[pcb_executing].stdout);
}
