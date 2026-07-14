/* ----------------------------------------------------------------*/
/* - atoi.c                                                       -*/
/* ----------------------------------------------------------------*/

#include <ansi.h>

int atoi(const char *nptr) 
{
 int value=0;
 char *ptr;

 ptr=(char *)nptr;
 
 while(*ptr==' ') ptr++;
 
 while (*ptr!=0 && isdigit((int)*ptr)) {
  value*=10;
  value+=(*ptr)-'0';
  ptr++;
 }

 return value;
}
