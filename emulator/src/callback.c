/* -------------------------------------------------------------------------- */
/* - MMU Callbacks for ÖverDOS Emulator                                     - */
/* - Copyright (c) Andreas Westling                                         - */
/* - GNU General Public License                                             - */
/* -------------------------------------------------------------------------- */

#include "mmu.h"
#include "console.h"
#include "isa.h"
#include <time.h>

static unsigned char rtc_2bcd (unsigned char value);

/* -------------------------------------------------------------------------- */
/* - cb_uart0()                                                             - */ 
/* - Callback-handler for memory-mapped UART0 data.                         - */
/* - This also handles ICR.                                                 - */
/* - AREA: 0xbff80fe0 - 24 bytes.                                           - */
/* -------------------------------------------------------------------------- */
void cb_uart0 (void *data, int size,int offset, int action)
{
 static MMU_NODE *node=NULL;
 static int pending_valid = 0;   /* a keystroke is waiting to be read via RBR */
 static int pending_char  = 0;
 unsigned int value_int;

 if (!node) {
  node = mmu_addr2node (0xbff80fe0, NULL);
 }

 /* Poll the host keyboard on any UART read, and keep the key pending until it
    is actually consumed via RBR.  The kernel first reads USR to test the
    "data ready" bit and then reads RBR - those are two separate callback
    calls, so the pending state must survive between them. */
 if (action == MMU_LOAD && !pending_valid) {
  if (con_kbhit()) {
   int c = (unsigned char) con_getch();
   if (c == 27) {
    /* swallow a lone ESC */
   } else {
    pending_char  = c;
    pending_valid = 1;
   }
  }
 }

 switch(action) {
 case MMU_LOAD:
   /* RBR - receive buffer register. */
   if (offset >= 0 && offset <= 3){
     if (pending_valid) {
       value_int = pending_char;
       if (value_int == 10) {    /* deliver Enter (LF) as CR */
        value_int = 13;
       }
       pending_valid = 0;        /* consume */
     } else {
       value_int = 0;
     }
     *((int *)node->data) = isa_reorder(value_int);
   }
   /* USR - line/UART status register. */
   if (offset >= 20 && offset <=23) {
     value_int = isa_reorder(*((int *)(((char *)node->data) + 20)));
     value_int |= (1<<5);        /* transmitter always ready (TBR empty) */
     if (pending_valid) {
       value_int |= 1;           /* bit 0: receive data available */
     } else {
       value_int &= ~1u;
     }
    *((int *)(((char *)node->data) + 20)) = isa_reorder(value_int);
   }
   break;
 case MMU_STORE: 
   /* Load TBR and print. */
   if (offset >= 0 && offset <= 3){
     value_int = isa_reorder(*(((int *)node->data)));
     con_putbyte(value_int);
   }
   /* Set USR. TBR is allways empty. :) */
   if (offset >= 20 && offset <=23) {
     value_int = isa_reorder(*((int *)((char *)(node->data + 20))));
     value_int |= (1<<5);
     *((int *)((char *)(node->data + 20))) = isa_reorder(value_int);
   }
   break;
 default:
   return;
 }
}



/* -------------------------------------------------------------------------- */
/* - cb_interrupt_config()                                                  - */ 
/* - Callback-handler for memory-mapped UART0 config.                       - */
/* - AREA: 0xbff90000 - 16 bytes.                                           - */
/* -------------------------------------------------------------------------- */
void cb_interrupt_config (void *data, int size,int offset, int action)
{
 return;
}



/* -------------------------------------------------------------------------- */
/* - cb_led()                                                               - */ 
/* - Callback-handler for memory-mapped LED.                                - */
/* - AREA: 0xbff20010 - 16 bytes.                                           - */
/* -------------------------------------------------------------------------- */
void cb_led (void *data, int size,int offset, int action)
{
 return;

}



/* -------------------------------------------------------------------------- */
/* - cb_rtc()                                                               - */ 
/* - Callback-handler for memory-mapped Real-Time Clock.                    - */
/* - AREA: 0xbff00000 - 8 bytes.                                           - */
/* -------------------------------------------------------------------------- */
void cb_rtc (void *data, int size,int offset, int action)
{
 MMU_NODE *node=NULL;
 static int hour_format = 24, data_format = 1;
 unsigned char value;
 time_t t;
 struct tm *t_breakup;
 
 if (!node) {
  node = mmu_addr2node (0xbff00000, NULL);
 }

 /* Handle read request from RTC_ADDR. */
 if (action == MMU_STORE && offset == 0) {
  time(&t);
  t_breakup = localtime(&t);
  
  switch (*(char *)data) {
   case 11: /* RTC_B */
    value = 0;   
    if (hour_format==24) {
     value |= (1<<1);
    }
    if (data_format) {
     value |= (1<<2);
    }
    *(((char *)node->data)+4) = value;
    break; 
   case 10: /* RTC_A */
    *(((char *)node->data)+4) = 0;     
    break;
   case 9: /* Year */
    value = t_breakup->tm_year;
    if (!data_format  ) {
     value = rtc_2bcd(value);
    }
    *(((char *)node->data)+4) = value;    
    break;
   case 8: /* Month */
    value = t_breakup->tm_mon;
    if (!data_format) {
     value = rtc_2bcd(value);
    }
    *(((char *)node->data)+4) = value;    
    break;
   case 7: /* Day */
    value = t_breakup->tm_mday;
    if (!data_format) {
     value = rtc_2bcd(value);
    }
    *(((char *)node->data)+4) = value;    
    break;
   case 6: /* Day of the week */
    value = t_breakup->tm_wday +1;
    if (!data_format) {
     value = rtc_2bcd(value);
    }
    *(((char *)node->data)+4) = value;    
    break;
   case 4: /* Hours */
    value = t_breakup->tm_hour;
    if (hour_format !=24) {
     if (value>12) {
      value = value%12 | 128;
     } else {
      value = value%12;
     }
    }
    if (!data_format) {
     value = rtc_2bcd(value);
    }
    *(((char *)node->data)+4) = value;    
    break;
   case 2: /* Minutes */
    value = t_breakup->tm_min;
    if (!data_format) {
     value = rtc_2bcd(value);
    }
    *(((char *)node->data)+4) = value;    
    break;
   case 0: /* Seconds */
    value = t_breakup->tm_sec;
    if (!data_format) {
     value = rtc_2bcd(value);
    }
    *(((char *)node->data)+4) = value;    
    break;
  } 
 }


 return;
}


/* Pack a 0-99 value into two BCD nibbles: high nibble = tens, low = units.
   (The original expression relied on '+' binding tighter than '<<', so it
   evaluated as one huge shift instead of a BCD pack.) */
static unsigned char rtc_2bcd (unsigned char value)
{
 return (unsigned char)(((value / 10) << 4) | (value % 10));
}

