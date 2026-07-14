/* ---------------------------------------------------------------- */
/* - Memcpy32 - memcpy optimized for 32-bit systems.              - */
/* -                                                              - */
/* - Not too fast though, but faster than the naïve one and thats - */
/* - always something.                                            - */
/* ---------------------------------------------------------------- */
#include <ansi.h>

void *memcpy(void *dest, const void *src, size_t n) 
{
 int i, ndiv4, rem;

 /* Don't bother for n<5. */
 if (n<5) {
  for (i=0;i<n;i++) {
   *(((char *)dest)+i)=*(((char *)src)+i);
  }
  return dest;
 }
   
 /* Calculate number of words. */
 ndiv4=(n>>2);
 
 /* Copy word by word. */
 for (i=0;i<ndiv4;i++) {
   *(((int *)dest)+i)=*(((int *)src)+i);
 }
 
 /* Calculate number of remaining bytes. */
 ndiv4 <<= 2;
 rem     = n - ndiv4;
 dest   += ndiv4;
 src    += ndiv4;
 
 /* Copy remaining data byte by byte. */
 for (i=0;i<rem;i++) {
  *(((char *)dest)+i)=*(((char *)src)+i);
 }
 
 return dest;
}


