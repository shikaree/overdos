/* ----------------------------------------------------------------*/
/* - to.c                                                         -*/
/* ----------------------------------------------------------------*/

#include <ansi.h>

static char char_array_lower[]=LOCALE_LOWER;
static char char_array_upper[]=LOCALE_UPPER;
static char char_array_length=LOCALE_LENGTH;

int toupper(int c)
{
 int i;
 
 for (i=0;i<char_array_length;i++)
 {
  if ((int)char_array_lower[i]==c) {
   return (int)char_array_upper[i];
  } 
 }

 return c;
}

int tolower(int c)
{
 int i;
 
 for (i=0;i<char_array_length;i++)
 {
  if ((int)char_array_upper[i]==c) {
   return (int)char_array_lower[i];
  } 
 }

 return c;
}
