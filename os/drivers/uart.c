#include <mbox.h>
#include <ansi.h>
#include <uart.h>
#include <pcb.h>
#include <sched.h>


int buffer_len=0;
static char buffer[UART_BUFFER_SIZE];

/* ---------------------------------------------------------------- */
/* - uart_send()						  - */
/* - Receives a message from a mailbox and sends it thru the	  - */
/* - driver. If this function returns 1, the msg MUST be removed  - */
/* - from the mailbox!!!			       		  - */
/* - Returns:							  - */
/* -   	      1 - If the message was send.			  - */
/* -          0 - If not. 					  - */
/* ---------------------------------------------------------------- */
int uart_send(char *msg, int size)
{
 int transferred=0, slice=0;
 int i;

 if (buffer_len)
 {
  return 0;
 }

 /* Receive the message. */
 if (size) {
  for (i=size-1;i>=0;i--) {
   buffer[size-i-1]=msg[i];
  } 
 }
  
 buffer_len=size;

 /* Try to send the message. */
 while (buffer_len>0) {
  transferred=driver_uart_send(buffer[buffer_len-1]);
  buffer_len--;
  slice++;
  if (slice==UART_OUTPUT_SLICE) {
   break;
  }
 }
  
 return 1;
}




/* ---------------------------------------------------------------- */
/* - uart_int()							  - */
/* - Polls UART. Is called from inthandler.			  - */
/* - Force function is for inthandler "pass-thru" and allowes	  - */
/* - polled IO (which in fact is what we are using in this OS).   - */
/* ---------------------------------------------------------------- */
void uart_int(int force_function) 
{
 int transferred=0;
 int function;
 char buf[1];
 char msg[MAX_MSG_LEN];
 int size, bashpin, pcb_executing_save;
 
  
 if (force_function) {
  function = force_function;
 } else {
  function = *(int *)UART_USR;
 }
  
 /* --- Output --- */ 
 if (function & 32) {
  /* Are there any more messages to send? */
  
  if (buffer_len) {
   transferred=driver_uart_send(buffer[buffer_len-1]);
   }
   
  if (transferred) {
   buffer_len--;
  }

  /* Buffer empty? Copy a new message if there is one. */ 
  if (pcb_focus!=-1 && mbox[pcbs[pcb_focus].stdout].active && !buffer_len && 
      mbox[pcbs[pcb_focus].stdout].msg_count) {
   size=receive(msg, pcbs[pcb_focus].stdout+65536);
   if (size>0) {
    uart_send(msg,size); 
   }
  }
 }
 
 /* --- Input --- */
 if (function & 1) {
  /* Try to receive. */
  transferred=driver_uart_receive();
  
  /* Focusswitch? */
  if ((char)transferred == UART_FOCUSCHAR) {
   switch_focus();
   transferred=-1;
  }

  /* New command interpreter? */
  if ((char)transferred == UART_BASHCHAR) {
   pcb_executing_save=pcb_executing;
   pcb_executing=-1;
   bashpin=proc_create("visky",0);
   pcb_executing=pcb_executing_save;
   transferred=-1;
  }
 
  /* Is the process in focus active? */
  if (!pcbs[pcb_focus].active) {
   return;
  }
      
    
  /* Destructive send. */
  if (transferred!=-1) {  
   /* Is mailbox full? */
   if (pcb_focus!=-1 && mbox[pcbs[pcb_focus].stdin].msg_count<
       mbox[pcbs[pcb_focus].stdin].size) {
       
    /* Send the message. */
    buf[0]=(char)transferred;
    send(buf,1,pcbs[pcb_focus].stdin);

   }
  }
 }

 
 return;
}
