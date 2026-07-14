#include <ansi-mod.h>
#include <ansi.h>
#include <pcb.h>
#include <sched.h>
#include <uart.h>

char exc_name[32][6]={"Int","Mod","TLBL","TLBS","AdEL","AdES","IBE","DBE","Sys","Bp","RI","CpU","Ov","Tr","VCEI","FPE","-","-","-","-","-","-","-","WATCH","-","-","-","-","-","-","-","VCED"};

extern int pcb_executing;

void kprint(char *text);

/* ---------------------------------------------------------------- */
/* - strpad()							  - */
/* - Pads the string str to length a using character c.		  - */
/* - Behaviour is undefined when i < strlen(str).		  - */
/* ---------------------------------------------------------------- */

void strpad(char *str,char c,int i)
{
   int a,b;
   b=(a=strlen(str));
   if(i!=a){
      while(b++<i-a)
         str[b]=c;
      for(b=0;b<=a;b++){
         str[i-b]=str[a-b];
         str[a-b]=c;
      }
   }
}


/* ---------------------------------------------------------------- */
/* - bluescreen()						  - */
/* - Displays the dreaded blue screen.				  - */
/* ---------------------------------------------------------------- */

void bluescreen(int exc_code, int epc)
{
   char a[12];
   int b;
   kprint("\x1b[1m\x1b[37m\x1b[44m\x1b[2J");

/*   kprint("\x1b[8;37H\x1b[34m\x1b[46m");*/
   kprint("\x1b[8;37H\x1b[34m\x1b[46m");
   kprint(" ÖverDOS ");

   kprint("\x1b[10;6H");
   kprint("\x1b[1m\x1b[37m\x1b[44m");

   kprint("A fatal exception ");
   
   b=itoahex(a,exc_code);
   strpad(a,'0',2);
   kprint(a);
   
   kprint(" (");kprint(exc_name[exc_code]);
   
   kprint(") has occurred at ");
   
   b=itoahex(a,epc);
   strpad(a,'0',8);
   kprint(a);
   
   kprint(" in ");
   
   if(proc_get_name(a,pcb_executing))
      kprint(a);
   else
      kprint("körnel");
   kprint(".");
   
   kprint("\x1b[11;6H");
   kprint("The current application will be terminated.");
   kprint("\x1b[13;6H");
   kprint("*  Press any key to terminate the current application.");
   kprint("\x1b[14;6H");
   kprint("*  Press CTRL+ALT+DEL if you feel like it, but it is unlikely");
   kprint("\x1b[15;9H");
   kprint("to do you any good.");
   kprint("\x1b[17;28H");
   kprint("Press any key to continue ");
   while(driver_uart_receive()==-1);
   kprint("\x1b[37m\x1b[40m\x1b[0m\x1b[2J");
   
   proc_kill(-1);

}
         
