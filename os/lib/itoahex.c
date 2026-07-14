#include <ansi-mod.h>

char hexch[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

/* ---------------------------------------------------------------- */
/* - itoahex()							  - */
/* - Writes a string containing the number i in hexadecimal form  - */
/* - to buf. buf is assumed to be at least 12 bytes long.	  - */
/* - Returns: The number of digits written to buf.		  - */
/* ---------------------------------------------------------------- */

int itoahex(char *buf,unsigned int i)
{
   int x=11,b;
   buf[11]='\0';
   while(i){
      buf[--x]=hexch[i%16];
      i/=16;
   }
   for(b=x;b<12;b++)
      buf[b-x]=buf[b];
   return 11-x;
}

