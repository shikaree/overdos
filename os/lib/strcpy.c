/* ----------------------------------------------------------------*/
/* - strcpy.c                                                     -*/
/* ----------------------------------------------------------------*/

char *strcpy(char *dest, const char *src) 
{
 char *copyptr_dest, *copyptr_src;
 
 copyptr_dest=dest-1;
 copyptr_src=((char *)src)-1;
 
 do {
  copyptr_dest++;
  copyptr_src++;
  *copyptr_dest=*copyptr_src;
 } while (*copyptr_src!=0);

 return dest;
}
