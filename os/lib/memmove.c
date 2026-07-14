/* ---------------------------------------------------------------- */
/* - memmove.c                                                    - */
/* ---------------------------------------------------------------- */

#include <ansi.h>

void *memmove(void *dest, const void *src, size_t n) 
{
 unsigned char *c_dest, *c_src;
 unsigned int i;
 
 /* Set up pointers. */ 
 c_dest=(unsigned char *)dest;
 c_src=(unsigned char *)src;
 
 /* Dest lies within src and src+(n-1). 
    Start from dest+(n-1) and go to dest. */
 if (c_dest>=c_src && c_dest<c_src+n) {
  i=n;
  while(i>0) {
   i--;
   c_dest[i]=c_src[i];
  }
 }
 /* Src lies within dest and dest+(n-1).
    Start from dest and go to dest+(n-1). */
 else if (c_src>=c_dest && c_src<c_dest+n) {
  for (i=0;i<n;i++) {
    c_dest[i]=c_src[i];
   }
 }
 /* They do not overlap. Just do a memcopy. */
 else {
  return _memcpy(dest,src,n);
 }
 
 return dest;
}
