/*
   INIT
*/
#include <mbox.h>
#include <sched.h>
#include <intmode.h>
#include <uart.h>
#include <led.h>
#include <ansi.h>
#include <pcb.h>
#include <ansi-mod.h>

#define COMMAND_SHELL "visky"

void inthandler_init(void);
void kprint(char *text);
void driver_timer_init (char b);
void dirver_uart_init(void);
int sizeof_pcb;
void ledd (char *buf);


/* ---------------------------------------------------------------- */
/* - init()							  - */
/* - Startup-function. Called by __start.			  - */
/* ---------------------------------------------------------------- */
void init(void) 
{

 /* Do this first of all. */
 
 intmode_disable();
 
 /* Initial setup for pcb. */
 sizeof_pcb=sizeof(struct pcb);
  
 /* Show nice boot-logo. */
 kprint("                               __                     \x0d\x0a");
 kprint(" ____                         /\\ \\			\x0d\x0a");
 kprint("/\\_\\_\\  __  __     __   _  __ \\_\\ \\    ___     ____	\x0d\x0a");
 kprint("\\/ __`\\/\\ \\/\\ \\  /'__`\\/\\`'__\\/'_` \\  / __`\\  /',__\\  \x0d\x0a");
 kprint("/\\ \\L\\ \\ \\ \\_/ |/\\  __/\\ \\ \\//\\ \\L\\ \\/\\ \\L\\ \\/\\__, `\\ \x0d\x0a");
 kprint("\\ \\____/\\ \\___/ \\ \\____\\\\ \\_\\\\ \\___,_\\ \\____/\\/\\____/ \x0d\x0a");
 kprint(" \\/___/  \\/__/   \\/____/ \\/_/ \\/__,_ /\\/___/  \\/___/  \x0d\x0a\x0d\x0a\x0a");
 
 /* Start LED. */
 ledd ("LED");
 kprint("      LED Subsystem: ");
 driver_led_init();
 kprint("enabled.\x0d\n");
 
 /* Start UART. */
 ledd ("UART");
 kprint("     UART Subsystem: ");
 driver_uart_init();
 kprint("enabled.\x0d\n");

 /* Start PROC-handler. */
 ledd ("PROC");
 kprint("     PROC Subsystem: ");
 pcb_init();
 kprint("enabled.\x0d\n");
  
 /* Start MBOX-handler. */
 ledd ("MBOX");
 kprint("     MBOX Subsystem: ");
 mbox_init();
 kprint("enabled.\x0d\n"); 

 /* Set the timer. */
 ledd ("TIME");
 kprint("    TIMER Subsystem: ");
 driver_timer_init(5000);
 kprint("enabled.\x0d\n");

 /* Start interrupt-handler. */
 ledd ("INTR");
 kprint("INTERRUPT Subsystem: ");
 inthandler_init();
 kprint("enabled.\x0d\n\n");

 /* Start command shell. */ 
 kprint("Preparing command shell..."); 
 proc_create(COMMAND_SHELL,0);
 kprint("done.\x0d\n\n");

 /* Enable interrupts */
 kprint("Dispaching...\x0d\n");
 pcb_idleloop=1; 
 pcb_focus=0;

 ledd ("   ");
 idleloop();
}



/* ---------------------------------------------------------------- */
/* - kprint()							  - */
/* - Kernel print.						  - */
/* ---------------------------------------------------------------- */
void kprint(char *text) 
{
 int i;

 for (i=0;i<strlen(text);i++) {
  driver_uart_send(text[i]);
 }

 return;
}



/* ---------------------------------------------------------------- */
/* - ledd()							  - */
/* - LED-Debug function. Just outputs a string to the led.	  - */
/* ---------------------------------------------------------------- */
void ledd (char *buf) {
 volatile int *ledptr=(volatile int *)0xbff20010;
 int i,j,end;

 
 end=strlen(buf);
 if (end>4)
  end=4;
  
 for (i=0;i<end;i++) {
  ledptr[3-i]=(int)buf[i];
 }
 
 for (j=i;j<4;j++) {
    ledptr[3-j]=(int)' ';
 }

 return;
}
