#include <ansi.h>
#include <ansi-mod.h>
#include <os.h>

int main_prodcons(void)
{
   char buf[256];
   int focus=1,m=0,mb,size=-1;
   register_focusswitch(&focus);
   printf("Detecting producer/consumer status...\n\x0d");
   m=create_mbox(1,"pcctrl");
   mb=receive_msg_cond(buf,m);
   if(mb==ERR){
      printf("Seems like I'm supposed to be the producer.");
      sprintf(buf,"pccomm%d",get_pin());
      printf("Mailbox name: %s",buf);
      while((size<0)||(size>9)){
         printf("\x0d\n\nEnter mailbox size (0-9): ");
         size=getchar()-48;
         if(!(size<0)||(size>9)){
            mb=create_mbox(size,buf);
               if(mb==-1){
               printf("\n\x0dSize not supported by current kernel.");
               size=-1;
            }
         }
      }
      printf("\n\n\x0d");
      printf("Creating consumer / Switching focus ...\n\n\x0d");
      send_msg(buf,strlen(buf)+1,m);
      m=0;
      create_process("prodcons",get_priority(-1));
      while(1){
         sleep((rand()%100)*100+10000);
         sprintf(buf,"%d: Random value: %d\n\x0d",++m,rand()&1023);
         if(focus==1) printf("Producing: Message %s",buf);
         send_msg(buf,strlen(buf)+1,mb);
      }
      return 0;
   }
   printf("Seems like I'm supposed to be the consumer.\n\n\x0d");
   printf("Looking in mbox %s",buf);
   mb=create_mbox(0,buf);
   while(1){
      receive_msg(buf,mb);
      if(focus==1) printf("Consuming: Message %s",buf);
   }
   return 0;
}
