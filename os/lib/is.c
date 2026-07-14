/* ---------------------------------------------------------------- */
/* - is.c                                                         - */
/* ---------------------------------------------------------------- */

#include <ansi.h>

/* This is a non-libc function. It takes a 0-terminated array of
   characters and a character and determines if the character is  
   one of the characters in the array. It returns 1 if so and 0 if  
   not.                                                            */
   
static int isany (int c,char *array) 
{
 int i=0;

 while (array[i]!=0) {
  if ((char)c==array[i]) {
   return 1;
  }
  i++;
 }
 return 0;
}

int isalnum(int c)
{
 return isalpha(c) || isdigit(c);
}

int isalpha(int c)
{
 return isupper(c) || islower(c);
}

/* BSD/SVID extension. */
int isascii(int c)
{
 if ((c^128)==c) {
  return 1;
 }  
 return 0;
}

/* GNU extension. */
int isblank(int c) 
{
 return isany(c," \t");
}

int iscntrl(int c)
{
 /* 0 is a control character (in any locale). */
 if (c==0) {
  return 0;
 } 
 return isany(c,LOCALE_CNTRL);
}

int isdigit(int c) 
{
 return isany(c,LOCALE_DIGIT);
}

int isgraph(int c)
{
 if (c==0)
  return 0;
  
 return isprint(c); 
}

int islower(int c)
{
 return isany(c,LOCALE_LOWER);
}

int isprint(int c)
{
 return !iscntrl(c);
}

int ispunct(int c)
{
 if (isspace(c) || isalpha(c))
  return 0;
 return isprint(c);  
}

int isspace(int c)
{
 return isany(c,LOCALE_SPACE);
}

int isupper(int c)
{
 return isany(c,LOCALE_UPPER);
}

int isxdigit(int c)
{
 return isdigit(c) || isany(c,"abcdefABCDEF");
}
