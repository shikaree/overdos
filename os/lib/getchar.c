/* ---------------------------------------------------------------- */
/* - getchar()							  - */
/* - Receives a single character from stdin.			  - */
/* - Returns: The character received. 				  - */
/* ---------------------------------------------------------------- */

#include <os.h>

int getchar(void)
{
   char inp[256];
   receive_msg(inp,-2);
   send_msg(inp,1,-1);
   return (int)inp[0];
}

/* ---------------------------------------------------------------- */
/* - getcharne()						  - */
/* - Receives a single character from stdin without echoing.	  - */
/* - Returns: The character received. 				  - */
/* ---------------------------------------------------------------- */

int getcharne(void)
{
   char inp[256];
   receive_msg(inp,-2);
   return (int)inp[0];
}
   
