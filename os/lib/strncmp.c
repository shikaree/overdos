/* ---------------------------------------------------------------- */
/* - strncmp.c                                                    - */
/* ---------------------------------------------------------------- */

#include <ansi.h>

int strncmp(const char *s1, const char *s2, size_t n)
{
 int i=0;
 unsigned char u_s1, u_s2;

 /* The case where one of them is shorter is handled by the fact 
    that asciiz is lower than anything.                          */
 for(i=0;i<n;i++) {
  u_s1=(unsigned char)s1[i];
  u_s2=(unsigned char)s2[i];
  if (u_s1<u_s2) {
   return -1;
  } 
  if (u_s1>u_s2) {
   return 1;    
  } 
  if (u_s1==0 && u_s2==0) {
   return 0;
  }
 }

 /* They are equal. */
 return 0;
}

