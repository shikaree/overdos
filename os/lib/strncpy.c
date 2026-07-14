/* ----------------------------------------------------------------*/
/* - strncpy.c                                                    -*/
/* ----------------------------------------------------------------*/

#include <ansi.h>

char *strncpy(char *dest, const char *src, size_t n) 
{
 int i;
 char *copyptr_dest, *copyptr_src;
 
 copyptr_dest=dest-1;
 copyptr_src=((char *)src)-1;
 
 for (i=0;i<n;i++) {
  copyptr_dest++;
  copyptr_src++;
  *copyptr_dest=*copyptr_src;
  if (*copyptr_src==0) {
   return dest;
  } 
 }

 return dest;
}

