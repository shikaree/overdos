/* ---------------------------------------------------------------- */
/* - getch()							  - */
/* - Receives a single character from stdin if there are any.	  - */
/* - Returns: The character received - if there was one.	  - */
/* - 	      ERR - if no input was waiting.			  - */
/* ---------------------------------------------------------------- */
#include <ansi-mod.h>
#include <os.h>

int getch(void)
{
   char inp[256];
   if(receive_msg_cond(inp,-2)!=-1)
      return inp[0];
   return ERR;
}
