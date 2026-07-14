/* -------------------------------------------------------------------------- */
/* - Driver for MIPS 4032 LED						    - */
/* -------------------------------------------------------------------------- */
#include <led.h>
#include <pcb.h>
#include <sched.h>

extern timer_interval;
extern timer_counter;

struct LED_FIFO led_fifo;

static int driver_led_test_init(int value);



/* ---------------------------------------------------------------- */
/* - driver_led_init()						  - */
/* - Initializes the LED-FIFO.					  - */
/* ---------------------------------------------------------------- */
void driver_led_init(void) 
{
 int i;
  
 led_fifo.front=0;
 led_fifo.rear=3;
 led_fifo.count=4;
 for (i=0;i<LED_FIFO_SIZE;i++) {
  led_fifo.buf[i]=' ';
 }
 
 driver_led_test_init(1);

 return;
}



/* ---------------------------------------------------------------- */
/* - driver_led_test_init()					  - */
/* - This is a test and set function for led_fifo_initialized,    - */
/* - althoughi t will NEVER set led_fifo_initialized=0.		  - */
/* - So, send value=0 to poll and value=1 to set.		  - */
/* - Returns:							  - */
/* -          Value of led_fifo_initialized.			  - */
/* ---------------------------------------------------------------- */
static int driver_led_test_init(int value) 
{
 static int led_fifo_initialized=0;

 if (value) {
  led_fifo_initialized=1;
  return 1;
 }
 
 return led_fifo_initialized;
}



/* ---------------------------------------------------------------- */
/* - driver_led_setchar()					  - */
/* - Sets position number (0-3, 0=leftmost) to character on LED   - */
/* ---------------------------------------------------------------- */
void driver_led_setchar (char character, int number) 
{
 volatile int *ledptr=(volatile int *)LED_REG_ADDR;

 if (number<0 || number>3) {
 
 }
 ledptr[4-number]=(int)character;

 return;
}



/* ---------------------------------------------------------------- */
/* - driver_led_send()						  - */
/* - Sends a message to the LED-FIFO.				  - */
/* - Application needs to poll until it returns >0. If an 	  - */
/* - application tries to send more than LED_FIFO_SIZE bytes, the - */
/* - message will be truncated.          			  - */
/* - Only the process with focus will be allowed to send.	  - */
/* - If led_fifo_unitialized=0 this function will always 	  - */
/* - return 0.	 						  - */
/* - The variable kernel is to bypass the intial pcb_focus test	  - */
/* - for kernel send  						  - */
/* - Returns:							  - */
/* -	      number of characters send or			  - */
/* -	      0 - If no characters were sent.			  - */
/* ---------------------------------------------------------------- */
int driver_led_send(char *msg, int size, int kernel) 
{
 int i;
 
 /* Fool processes not having focus. */
 if (pcb_focus!=pcb_executing && !kernel) {
  return size;
 }


 if (!driver_led_test_init(0)) {
  return 0;
 }

 
 /* Too large message. Truncate. */
 if (size>LED_FIFO_SIZE) {
  size=LED_FIFO_SIZE;
 }

 /* Not enough free space in buffer? */
 if (size>LED_FIFO_SIZE-led_fifo.count) {
  return 0;
 } 

 /* Copy the message. */
 for (i=0;i<size;i++) {
  /* This is a Shikaree-style FIFO, just like in mbox_*. */
  if (led_fifo.count)  {
   led_fifo.rear=(led_fifo.rear+1) % LED_FIFO_SIZE;
  } 
  led_fifo.buf[led_fifo.rear]=msg[i];
  led_fifo.count++;
 }

 return size;
}



/* ---------------------------------------------------------------- */
/* - driver_led_int()						  - */
/* - Hook for interrupt-handler. Takes characters from the FIFO   - */
/* - and sends them to the LED.					  - */
/* ---------------------------------------------------------------- */
void driver_led_int(void) 
{
 static int microsecond=0;
 char outbuf[4]="    ";
 int i;

 /* Output LED_OUTPUT_SPEED times per second. */
 microsecond+=timer_interval;
 if (microsecond<1000000/LED_OUTPUT_SPEED) {
  return; 
 }
 microsecond=0;
 
 /* Get four characters. */
 for(i=0;i<4;i++) {
  if ((i+1)<led_fifo.count) {
   outbuf[i]=led_fifo.buf[led_fifo.front+i];
  }
 }

 /* output them. */
 
 for (i=0;i<4;i++) {
  driver_led_setchar(outbuf[i],i);
 }

 if (!led_fifo.count) {
  return;
 }

 led_fifo.buf[led_fifo.front]=' ';
 if (led_fifo.count!=1) {
   led_fifo.front=(led_fifo.front+1) % LED_FIFO_SIZE;
 }
 
 led_fifo.rear=(led_fifo.rear+1) % LED_FIFO_SIZE;
 led_fifo.count--;
      			
 return;
}

