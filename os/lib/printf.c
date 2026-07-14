/* ---------------------------------------------------------------- */
/* - printf.c (version 2.0)                                       - */
/* - 								  - */
/* - See vsnprintf for format documentation.                      - */
/* ---------------------------------------------------------------- */
#include <stdarg.h>
#include <ansi-mod.h>
#include <ansi.h>
#include <os.h>


/* ---------------------------------------------------------------- */
/* - This is for printf and cprintf also (cprintf is macronized   - */
/* - for compatibility).					  - */
/* ---------------------------------------------------------------- */
int _printf (const char *format, ...) {
 va_list ap;
 char str[PRINTF_BUFFER+1];
 char *ptr;
 int len, send;
 
 va_start(ap,format); 
 len=vsnprintf(str,PRINTF_BUFFER,format,ap);
 
 va_end(ap);
 
 if (len) {
  ptr=str;
  while (len>0) {
   if (len>MAX_MSG_LEN) {
    send=MAX_MSG_LEN;
   } else {
    send=len;
   }
   send_msg(ptr,send,-1);
   len -= send;
   ptr += send;
  }
 }
 return len;
}


/* ---------------------------------------------------------------- */
/* - Works just like printf but outputs to a mailbox.             - */
/* - for compatibility).					  - */
/* ---------------------------------------------------------------- */
int mprintf(int mbox_nr,  const  char  *format,  ...) 
{
 va_list ap;
 char str[PRINTF_BUFFER+1];
 char *ptr;
 int len, send;
 
 va_start(ap,format); 
 len=vsnprintf(str,PRINTF_BUFFER,format,ap);
 
 va_end(ap);
 
 if (len) {
  ptr=str;
  while (len>0) {
   if (len>MAX_MSG_LEN) {
    send=MAX_MSG_LEN;
   } else {
    send=len;
   }
   send_msg(ptr,send,mbox_nr);
   len -= send;
   ptr += send;
  }
 }
 return len;
}




int _sprintf(char *str, const char *format, ...)
{
 va_list ap;
 int len;
 
 va_start(ap,format);
 len=vsnprintf(str,PRINTF_BUFFER,format,ap);

 /* ALLWAYS terminate the buffer, allways... */
 str[len]=0;
 va_end(ap);
 
 return len;
}




int snprintf(char *str, size_t size, const  char  *format, ...)
{
 va_list ap;
 int len;
 
 va_start(ap,format);
 len=vsnprintf(str,size,format,ap);
 va_end(ap);
 
 return len;
}


