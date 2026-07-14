/* ---------------------------------------------------------------- */
/* - strcat.c                                                     - */
/* ---------------------------------------------------------------- */

char *strcat(char *dest, const char *src) 
{
 char *copyptr_dest, *copyptr_src;
 
 /* Find asciiz in des. */
 copyptr_dest=dest;
 while (*copyptr_dest!=0) {
  copyptr_dest++;
 }
 
 copyptr_src=(char *)src;
 
 /* Copy src to the end of dest. */
 do {
  *copyptr_dest=*copyptr_src;
  copyptr_dest++;
  copyptr_src++;
 } while (*copyptr_src!=0);
 *copyptr_dest=0;
 return dest;
}


