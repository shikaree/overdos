#include <sched.h>
#include <mbox.h>
#include <os.h>
#include <fs.h>
#include <stacks.h>
#include <bitarrayops.h>
#include <ansi-mod.h>
#include <ansi.h>
#include <vt100.h>
#include <pcb.h>

float version;

/*-------------------------------------*/
void *pars56(char *name){
  int i=0,j=0,we_are_here;
  pin n;
  bool b;
  char arg1[80];
  char arg2[5];
  char filename[64];

/*-------------------------------------*/
  for(i=0;(name[i]!=' ')&&(name[i]!=13)&&(i<76);i++);
  if(i>=76) { printf("Too long command"); return (void *)0; }
/*-------------------------------------*/
 
  if(name[0]==13){ return (void *)0;
  }


  /* NO Arguments!! */
 
  if (name[i]==13) {

    name[i] = 0;
    
    if(0==strcmp(name,"clear")){
      cprintf("%1%2%3%0%@",7,0,0,xy(1,1));
      return (void *)0;
    }

    if(0==strcmp(name,"däcka")){
      *(int *)0 = 28;
      return (void *) 0;
    }

    if(0==strcmp(name,"pez")){
      cprintf("%0"); 
      cprintf("%@Namn%@PIN%@Prioritet",xy(1,1),xy(11,1),xy(16,1));
      j = 2;
      for(i=0;i<MAX_PROCESSES;i++){
	if(pcbs[i].active==TRUE){
	  cprintf("%@%s%@%d%@%d",xy(1,j),pcbs[i].name,xy(11,j),
		  i,xy(16,j),(int)pcbs[i].priority);
	  j++;
	}
      }
      return (void *)0;
    }
    
    if(0==strcmp(name,"reboot")){
       reboot();
    }

    if(0==strcmp(name,"ls"))
    { 
      cprintf("\n\n\x0d");
      do {
	i=dir_index(filename,i);
        if (i!=-1) {   
	 cprintf(" %s\n\x0d",filename);
	}
	i++;
      } while(i);
      return (void *)0;
    }
  
     n = create_process(name, 0);
     if (n>-1) {
      printf("%d",n);
      return (void *)0;
     }
    printf("\x0d\nbars: %s: command not found or argument missing",name);
    return (void *)0;
  }
/*-------------------------------------*/
  /* get first argument */
  if(name[i]==' '){
     name[i]=0; /* so we can call strcmp */
     for(j=i+1;name[j]!=13&&name[j]!=' ';j++){   
      /*  if(j==5+i+1){
	       printf("Bad argument");
               return (void *)0;
               } 
               */
        arg1[j-i-1] = name[j];
     }
  }

  /*set last char to NULL so we can call atoi*/
  arg1[j-i-1] = 0;
/*-------------------------------------*/    
  /* Kill arg1 */
  we_are_here=j; 
  if (0==strcmp(name,"kill")) { 
      if (name[j]!=13) { printf("Bad argument"); return (void *)0; }
      b = proc_kill(atoi(arg1));
      if(b==FALSE) printf("\x0d\nInvalid pin");
      return (void *)0;
    }
    
    if (0==strcmp(name,"get_priority")) {
      if (name[j]!=13) { printf("Bad argument"); return (void *)0; }
      b = proc_get_priority(atoi(arg1));
      if(b==255) { 
         printf("\x0d\nInvalid pin");
         return (void *)0;
         }
      printf("\x0d\n%i",b);
      return (void *)0;
    }

/*
   .
   .
   more commands with 1 arg.
   .
   .
*/
  if (name[we_are_here]==13) {  printf("\x0d\nbars: %s: command not found",name);return (void *)0;}
/*-------------------------------------*/  
/* Ok, lets check commands with 2 args.*/
  if(name[we_are_here]==' '){
     for(j=we_are_here+1;name[j]!=13&&name[j]!=' ';j++){   
        arg2[j-i-1] = name[j];
    }
  }
  /*set last char to NULL so we can call atoi*/
  arg2[j-i-1] = 0;

 if (!strcmp(name,"command_with_2arg")) { 
      if (name[j-i-1]!=13) { printf("Bad argument"); return (void *)0; }
/*      b = command_with_2arg(atoi(arg1)); */
      if(b==FALSE) printf("Invalid pin");
      return (void *)0;
    }

/*
   .
   .
   more commands with 2 args.
   .
   .
*/

  printf("\x0d\nbars: %s: command not found",name);

  return (void *)0;
}



/*-------------------------------------*/
void main_bars(){
  char a;
  int i = 0;
  void *point;
  char in[80];
/*-------------------------------------*/
version=3.8;
loop:
  printf("\x0d\n#bärs3.8> ");
  while((a=getchar())!=13){
    if (a!=8) {
    in[i] = a;
    i++;
    }
    if (i==30) {
    while((a=getchar())!=13&&a!=8){ 
    printf("\x08");
    printf(" ");
    printf("\x08");
    };
    if (a==13) break;
    }
    if ((a==8)&&(i==0)) printf("\x1b[1C");
    if ((a==8)&&(i>0)) {
    printf(" ");
    printf("\x08");
    i--;
    }
  
  
  }
  in[i] = 13;
  point = pars56(in);
  i=0;
  goto loop;
  /* for in[i]=0 ???*/
}
