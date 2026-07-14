/* ---------------------------------------------------------------- */
/* - strncat.c                                                    - */
/* ---------------------------------------------------------------- */

#include <ansi.h>

char *strncat(char *dest, const char *src, size_t n) 
{
 int i=0;
 char *copyptr_dest, *copyptr_src;
 int count=0;
  
 /* Find asciiz in dest. */
 copyptr_dest=dest;
 while (*copyptr_dest) {
  copyptr_dest++;
 }
 
 copyptr_src=(char *)src;
 
 /* Copy src to the end of dest. */
 for (i=0;i<n;i++) {
  *copyptr_dest=*copyptr_src;
  copyptr_dest++;
  copyptr_src++;
  count++;
 }
 
 /* Add terminating asciiz if allowed. */
 if (count<n) {
  copyptr_dest++;
  *copyptr_dest=0;
 } 
 
 return dest;
}

