/* ---------------------------------------------------------------- */
/* - itoa.c                                                       - */
/* ---------------------------------------------------------------- */

#include <ansi.h>

/* Itoa is a reversal of atoi. It only handles 32 bit values,
   signed or unsigned.
   The input buffer must be large enough to handle 2^32 and
   a minus sign (12 characters incl. ASCIIZ). The result is left
   and right trimmed.                                            
   
   The function returns the length of the string s.              */

int itoa(char *s, long long int i) {
 char digits[]=LOCALE_DIGIT;
 int m=1000000000;
 int div;
 int pos=0;
 int abs;
 int firstnonzero=0;
  
 /* Overflow? */
 if (i>(4294967295LL)) {
  strcpy(s,"(overflow)");
  return 10;
 }
 
 /* Zero? */
 if (i==0) {
  s[0]='0';
  s[1]=0;
  return 1;
 } 
  
 /* Add the minus sign if it is there. */
 if (i<0) {
  s[0]='-';
  pos++;
 }
 
 /* Calculate absolute value. */
 if (i<0) {
  abs=0-i;
 } 
 else {
  abs=i;
 } 

 /* Divide downwards and subtract. */
 while (m>0) {
  div=abs/m;
  if (div!=0) {
   firstnonzero=1;
  }
  if (div>0 || firstnonzero) {
   s[pos]=digits[div]; 
   abs=abs-div*m;
   pos++;
  }
  if (m==1) {
   m=0;
  }
  else {
   m/=10;
  } 
 }
 
 /* Terminate the buffer. */
 s[pos]=0;
    
 return pos;
}


int itoa_wrap(char *s,int i)
{
   return itoa(s,(long long int)i);
}
