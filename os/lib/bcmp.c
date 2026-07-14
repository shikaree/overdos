/* ---------------------------------------------------------------- */
/* - bcmp.c                                                       - */
/* ---------------------------------------------------------------- */

#include <ansi.h>

int bcmp(const void *s1, const void *s2, size_t n) 
{
 unsigned int i;
 char *c_s1, *c_s2;

 c_s1=(char *)s1;
 c_s2=(char *)s2;
 
 /* Go thru every index. */
 for (i=0;i<n;i++) {
  /* Unequal. */
  if (c_s1[i]!=c_s2[i])
   return 1;
 }
 
 /* They are equal. */
 return 0;
}
