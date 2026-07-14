   /* ---------------------------------------------------------------- */
/* - vsnprintf.c                                                  - */
/* - 								  - */
/* - This is a stripped down, highly modified (when comparing to  - */
/* - the POSIX version) of vsnprintf for ÖverDOS.                 - */
/* - This function form the basis for all *printf-functions       - */
/* - in ÖverDOS and does all the formatting.                      - */
/* -                                                              - */
/* - Supported formatting characters are:                         - */
/* -                                                              - */
/* -  %c, %d/%i, %s, %% - As standard vsnprintf.                  - */
/* -  %0 - Clear screen. (You could also use "\x1b[2J".)          - */
/* -  %1 - Change foreground color.                               - */
/* -  %2 - Change background color.                               - */
/* -  %3 - Change attribute.                                      - */
/* -  %@ - Goto x,y coordinate. Use xy()-macro.                   - */
/* -                                                              - */
/* - A special type of field modifier is supported by '.'.        - */
/* - You can specify for any type the number of bytes it will use - */
/* - and it will ALWAYS use that much (padded with spaces).       - */
/* - This is used in this way (printf-example):                   - */
/* -                                                              - */
/* - printf("%.10s","test");					  - */
/* - printf("%.4i",546826);					  - */
/* -                                                              - */
/* - The '-' flag is used to indicate left-justification and '0'  - */
/* - can be used in front of field size to pad with zeroes 	  - */
/* - instead of spaces:                                           - */
/* -                                                              - */
/* - printf("%.-8,100);                                           - */
/* - printf("%.010",65482);					  - */
/* -                                                              - */
/* - No other flags/conversion characters etc. are supported.     - */
/* -                                                              - */
/* - //Andreas Westling                                          - */
/* ---------------------------------------------------------------- */
#include <stdarg.h>
#include <ansi-mod.h>
#include <ansi.h>

#define MODE_NORMAL	0
#define MODE_FORMATTING 1

#define isnumber(x) (x>='0' && x<='9')

#define XFORMAT_COLLECTING_FIELD_SIZE  1
#define XFORMAT_COLLECTING_FIELD_CHARS 2

typedef struct VSNPRINTF_XFORMAT {
 int formatting;
 int justify_left;
 char paddingchar;
 int paddingchar_lock;
 int field_size;
 int collecting;
} VSNPRINTF_XFORMAT; 

static void vsnprintf_xformat_reset (struct VSNPRINTF_XFORMAT *xformat);
static void vsnprintf_buf(char *str, size_t size, size_t *used, 
   			  struct VSNPRINTF_XFORMAT *xformat, char *buf);
static int vsnprintf_xformat(char *tobuf,
                             struct VSNPRINTF_XFORMAT *xformat,
                             char *xformat_buf, char *frombuf);
                             

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{  
 int i, format_char, format_len, mode=MODE_NORMAL, len, x, y;
 char conv_buf[32];
 char xformat_buf[PRINTF_BUFFER];
 size_t used=0;
 char f_char;
 int f_int;
 char *f_char_ptr;
 struct VSNPRINTF_XFORMAT xformat;
 
 /* Terminate string. */
 str[0]=0;

 /* Reset xformat. */
 vsnprintf_xformat_reset(&xformat);
  
 format_len=strlen(format);

 for (i=0;i<format_len;i++) {
  if (mode==MODE_FORMATTING) {
  
   format_char = format[i];
    
   /* %d = %i */
   if (format_char == 'i') {
    format_char = 'd';
   }
   
   switch(format_char) {
    case 'd':
     f_int = va_arg(ap,int);
     itoa(conv_buf,(long long int) f_int);
     if (xformat.formatting) {
      vsnprintf_xformat(xformat_buf,&xformat, xformat_buf, conv_buf);
      vsnprintf_buf(str, size, &used, &xformat, xformat_buf);
     } else {
      vsnprintf_buf(str, size, &used, &xformat, conv_buf);
     }
     mode = MODE_NORMAL;
     break;
    case 'c':
     f_char = va_arg(ap,int); 
     conv_buf[0]=f_char;
     conv_buf[1]=0;
     if (xformat.formatting) {
      vsnprintf_xformat(xformat_buf,&xformat, xformat_buf, conv_buf);      
      vsnprintf_buf(str, size, &used, &xformat,xformat_buf);
     } else {
      vsnprintf_buf(str, size, &used, &xformat,conv_buf);     
     }       
     mode = MODE_NORMAL;
     break;
    case 's':
     f_char_ptr = va_arg(ap,char *);
     if (xformat.formatting) {
      vsnprintf_xformat(xformat_buf,&xformat, xformat_buf, f_char_ptr);
      vsnprintf_buf(str, size, &used, &xformat,xformat_buf);
     } else {
      vsnprintf_buf(str, size, &used, &xformat,f_char_ptr);     
     } 
     mode = MODE_NORMAL;
     break;
    case '@':
     f_int = va_arg(ap,int); 
     x = f_int & 0xFF;
     y = f_int >> 8;
     conv_buf[0] = 27;
     conv_buf[1] = '[';
     itoa(conv_buf+2,(long long int)y);
     len=strlen(conv_buf);
     conv_buf[len]=';';
     itoa(conv_buf+len+1,(long long int)x);
     len=strlen(conv_buf);
     conv_buf[len]='H';
     conv_buf[len+1]=0;
     vsnprintf_buf(str, size, &used, &xformat, conv_buf);
     mode = MODE_NORMAL;
     break;
    case '%':
     vsnprintf_buf(str, size, &used, &xformat, "%");
     mode = MODE_NORMAL;
     break;      
    case '.':
     xformat.formatting=1;
     break;
    case '-': 
     xformat.formatting=1;
     xformat.justify_left=1;
     break;
    case '1':
     if (!xformat.formatting) {
      f_int = va_arg(ap, int);
      conv_buf[0] = 27;
      conv_buf[1] = '[';
      len=itoa(conv_buf+2, (long long int)f_int+30)+2;
      conv_buf[len]='m';
      conv_buf[len+1]=0;
      vsnprintf_buf(str, size, &used, &xformat, conv_buf);
      mode = MODE_NORMAL;
      break;
     } 
    case '2':
     if (!xformat.formatting) {
      f_int = va_arg(ap, int);
      conv_buf[0] = 27;
      conv_buf[1] = '[';
      len=itoa(conv_buf+2, (long long int)f_int+40)+2;
      conv_buf[len]='m';
      conv_buf[len+1]=0;
      vsnprintf_buf(str, size, &used, &xformat, conv_buf);
      mode = MODE_NORMAL;
      break;
     } 
    case '3':
     if (!xformat.formatting) {
      f_int = va_arg(ap, int);
      conv_buf[0] = 27;
      conv_buf[1] = '[';
      len=itoa(conv_buf+2, (long long int)f_int)+2;
      conv_buf[len]='m';
      conv_buf[len+1]=0;
      vsnprintf_buf(str, size, &used, &xformat, conv_buf);
      mode = MODE_NORMAL;
      break;
     } 
    case '0':
     if (!xformat.formatting) {
      vsnprintf_buf(str, size, &used, &xformat, "\x1b[2J");
      mode = MODE_NORMAL;
      break;
     } 
    default:
     if (xformat.formatting) {
      if (format_char=='0' && !xformat.paddingchar_lock) {
       xformat.paddingchar='0';
       xformat.paddingchar_lock=1;  
       break;
      }
      if (isnumber(format_char)) {
       /* format_char!='0' to handle (temporary) broken atoi. */
       if (xformat.collecting==-1 && format_char!='0') {
        xformat.paddingchar_lock=1;  
        xformat.collecting=0;
       } else {
        xformat.collecting++;
       }
       xformat_buf[xformat.collecting]=format_char;
      } else {
       if (format_char=='-') {
        xformat.justify_left=1;
       } else {
        mode = MODE_NORMAL;
       } 
      } 
      break;
     }  
   } 
  } else if (format[i] == '%') {
   mode = MODE_FORMATTING;
  } else {
   conv_buf[0] = format[i];
   conv_buf[1] = 0;
   vsnprintf_buf(str, size, &used, &xformat,conv_buf);
  }
  if (mode==MODE_NORMAL) {
   xformat.formatting=0;
  }
 }

 /* If we have space, terminate the string. */
 if (used<size) {
  str[used]=0;
 }

 return used;
}




/* ---------------------------------------------------------------- */
/* - vsnprintf_xformat_reset()                                    - */
/* - Resets an VSNPRINTF_XFORMAT struct.                          - */
/* ---------------------------------------------------------------- */
static void vsnprintf_xformat_reset (struct VSNPRINTF_XFORMAT *xformat) 
{
 xformat->formatting=0;
 xformat->justify_left=0;
 xformat->paddingchar=' ';
 xformat->paddingchar_lock=0;
 xformat->field_size=-1;
 xformat->collecting=-1;
 
 return;
}




/* ---------------------------------------------------------------- */
/* - vsnprintf_buf()                                              - */
/* - Function to add a buffer to the string. Adds the ENTIRE      - */
/* - buffer or nothing at all.                                    - */
/* ---------------------------------------------------------------- */
static void vsnprintf_buf(char *str, size_t size, size_t *used, 
   			  struct VSNPRINTF_XFORMAT *xformat, char *buf)
{
 int buf_len,i;
 
 buf_len=strlen(buf);
 
 /* Don't copy at all if there isn't enough space (don't break
    ANSI/VT100 commands in half).                              */
 if (buf_len+(*used)>size) {
  return;
 }
 
 /* Copy. */
 for (i=0;i<buf_len;i++) {
  str[*used]=buf[i];
  (*used)++;
 }
 
 return;
}




/* ---------------------------------------------------------------- */
/* - vsnprintf_xformat()                                          - */
/* - Xformat processor.                                           - */
/* ---------------------------------------------------------------- */
static int vsnprintf_xformat(char *tobuf,
                             struct VSNPRINTF_XFORMAT *xformat,
                             char *xformat_buf, char *frombuf) 
{
 int i, field_size, len;
 
 /* Get unfinished business. */
 if (xformat->collecting!=-1) {
   xformat_buf[xformat->collecting+1]=0;
   xformat->field_size=atoi(xformat_buf);
 } 

 field_size=xformat->field_size;
 
 /* Get len and check size. */
 len=strlen(frombuf);
 if (len>PRINTF_BUFFER) {
  len=PRINTF_BUFFER;
 }

 
 /* If field_size is undefined, we define it with len. */
 if (field_size==-1) {
  field_size=len;
 }

 /* Cut len if needed. */
 if (len>field_size) {
  len=field_size;
 }

  
 /* Clear out buffer for format. */
 for(i=0;i<field_size;i++) {
  tobuf[i]=xformat->paddingchar;
 }
 tobuf[field_size]=0;
    
 /* Now string is small enough. */
 if (xformat->justify_left) {
  _memcpy((void *)tobuf,(void *)frombuf,len);
 } else {
  _memcpy((void *)tobuf+(field_size-len),(void *)frombuf,len); 
 }


 
 vsnprintf_xformat_reset(xformat);
 return field_size; 
}
                                   

