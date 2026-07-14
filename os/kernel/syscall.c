/* Syscalls
   
   1-10 spim stuff! hands off!
   
   
   100 - proc_create
   101 - proc_kill
   102 - proc_set_priority
   103 - proc_get_priority
   104 - proc_get_name
   105 - proc_get_parent
   106 - proc_focus_give
   
   200 - mbox_create
   201 - mbox_destroy
   202 - send
   203 - receive
   204 - receive_cond

   300 - led
   
   400 - timer_counter
   401 - timer_interval
   402 - process_sleep
   403 - timer_uptime
   
   500 - register_callback
   
   600 - Jävla massa skit!
   
   800 - fs_dirlength
   801 - fs_getindex
   
   1000 - reboot
*/

#include <fs.h>
#include <led.h>
#include <pcb.h>
#include <mbox.h>
#include <sched.h>
#include <setclock.h>
#include <bitarrayops.h>


extern int timer_counter;
extern int timer_interval;

int interrupt(int a0,int a1,int a2,int a3);
void kill_system(void);


/* ---------------------------------------------------------------- */
/* - syscall()						       	  - */
/* - Runs the function that corresponds to a syscall number       - */
/* - Returns whatever the called function returns or -1 if a bad  - */
/* - number was used.						  - */
/* - This function is only to be used by the kernel.		  - */
/* ---------------------------------------------------------------- */

int syscall(int nr, int a1, int a2, int a3)
{
   switch(nr){   
   
      case 100:return proc_create((char *)a1,a2);
      case 101:return proc_kill(a1);
      case 102:return proc_set_priority(a1,a2);
      case 103:return proc_get_priority(a1);
      case 104:return proc_get_name((char *)a1,a2);
      case 105:return proc_get_parent(a1);
      case 106:return proc_focus_give(a1);
      case 107:return pcb_executing;
      case 108:return proc_get_status(a1);

      case 200:return mbox_create(a1,(char *)a2);
      case 201:return mbox_destroy(a1);
      case 202:return send((char *)a1,a2,a3);
      case 203:return receive((char *)a1,a2);
      case 204:return receive_cond((char *)a1,a2);
      
      case 300:return driver_led_send((char *)a1,a2,0);
      
      case 400:return timer_counter;
      case 401:return timer_interval;
      case 402:process_sleep(a1);return 1;  
      case 403:timer_uptime ((int *)a1,(int *)a2,(int *)a3);return 1;
      
      case 500:return((int)(pcbs[pcb_executing].focusswitch=(void *)a1));

      case 600:return readclock_sec();
      case 601:return readclock_min();
      case 602:return readclock_hour();
      case 603:return readclock_day_week();
      case 604:return readclock_day_month();
      case 605:return readclock_month();
      case 606:return readclock_year();
      case 607:return writeclock_sec(a1);
      case 608:return writeclock_min(a1);
      case 609:return writeclock_hour(a1);
      case 610:return writeclock_day_of_week(a1);
      case 611:return writeclock_day_of_month(a1);
      case 612:return writeclock_month(a1);
      case 613:return writeclock_year(a1);
      case 614:return print_clock();
      case 615:return print_date();
      case 616:return set_time(a1,a2,a3);
      case 617:return set_date(a1,a2,a3);

      case 800:return fs_dirlength();
      case 801:return fs_dirindex((char *)a1,a2);

      case 10000:kill_system();

      default: return -1;
   }
}


/* ---------------------------------------------------------------- */
/* - whatever()						          - */
/* - Wrapper functions for the syscalls to be used by C programs  - */
/* - (and assembly programs coded by lazy people.		  - */
/* - They are pretty much self explanatory.			  - */
/* ---------------------------------------------------------------- */

int create_process(char *name, int prior)
{
   return interrupt(100,(int)name,prior,0);
}

int kill_process(int nr)
{
   int r;
   r=interrupt(101,nr,0,0);
   
   if (pcb_executing == nr) {
    nr = -1;
   }
   
   while (nr==-1);
   return r;
}

int set_priority(int nr, int prior)
{
   return interrupt(102,nr,prior,0);
}

int get_priority(int nr)
{
   return interrupt(103,nr,0,0);
}

int get_name(char *buffer,int nr)
{
   return interrupt(104,(int)buffer,nr,0);
}

int get_parent(int p)
{
   return interrupt(105,p,0,0);
}


int give_focus(int p)
{
   return interrupt(106,p,0,0);
}


int get_pin(void) 
{
   return interrupt(107,0,0,0);
}


int get_state(int p)
{
   return interrupt(108,p,0,0);
}



int create_mbox(int size,char *name)
{
 int r;
  r=interrupt(200,size,(int)name,0);
  if (r!=-1) {
   setbit(pcbs[pcb_executing].mailbox_used,r);
  } 
  return r;
}

int destroy_mailbox(int nr)
{
 int r;
  r=interrupt(201,nr,0,0);
  if (r!=-1) {
   resetbit(pcbs[pcb_executing].mailbox_used,r);      
  }       
  return r;
}

int send_msg(char *msg, int size, int nr)
{
  int r=0;
  while(r==0) {
    r=interrupt(202,(int)msg,size,nr);
   if (!r || r==-1) {    
     while(pcbs[pcb_executing].status==STATUS_BLOCKED);
   }
   if (r==-1) {
    return 0;
   }
  } 
  return r;
}

int receive_msg(char *buffer, int nr)
{
  int r=0;

   while(r==0) {
    r=interrupt(203,(int)buffer,nr,0);
    if (!r) {
     while(pcbs[pcb_executing].status==STATUS_BLOCKED);     
    }
   } 
   return r;
}

int receive_msg_cond(char *buffer, int nr)
{
   return(interrupt(204,(int)buffer,nr,0));
}


int led(char *buffer, int size) 
{
 int r=0;
 while(r==0) {
   r=interrupt(300,(int)buffer,size,0); 
  }
 return r;  
}


int get_timer(void)
{
   return interrupt(400,0,0,0);
}

int get_timer_interval(void)
{
   return interrupt(401,0,0,0);
}


void sleep(int t) {
 unsigned int timerclicks;
   
 timerclicks=(t/timer_interval);
 interrupt(402,timerclicks,0,0); 
 while(pcbs[pcb_executing].status==STATUS_BLOCKED);     
 
 return;
}


void get_uptime (int *days, int *hours, int *minutes)  
{
 interrupt(403,(int)days,(int)hours,(int)minutes);
}


int register_focusswitch(int *switchvar)
{
   return interrupt(500,(int)switchvar,0,0);
}


char rclock_sec(){
return interrupt(600,0,0,0);
}

char rclock_min(){
return interrupt(601,0,0,0);
}

char rclock_hour(){
return interrupt(602,0,0,0);
}

char rclock_day_week(){
return interrupt(603,0,0,0);
}

char rclock_day_month(){
return interrupt(604,0,0,0);
}

char rclock_month(){
return interrupt(605,0,0,0);
}

char rclock_year(){
return interrupt(606,0,0,0);
}

char wclock_sec(char sec){
return interrupt(607,sec,0,0);
}

char wclock_min(char min){
return interrupt(608,min,0,0);
}

char wclock_hour(char h){
return interrupt(609,h,0,0);
}

char wclock_day_week(char d){
return interrupt(610,d,0,0);
}

char wclock_day_month(char d){
return interrupt(611,d,0,0);
}

char wclock_month(char m){
return interrupt(612,m,0,0);
}

char wclock_year(char y){
return interrupt(613,y,0,0);
}

char printclock(){
return interrupt(614,0,0,0);
}

char printdate(){
return interrupt(615,0,0,0);
}

char settime(char h, char m, char s){
return interrupt(616,h,m,s);
}

char setdate(char x, char y, char z){
return interrupt(617,x,y,z);
}


int dir_length(void) 
{
  return interrupt(800,0,0,0);
}

int dir_index(char *name, int index)
{
  return interrupt(801,(int)name,index,0);
}


void reboot(void)
{
   interrupt(10000,0,0,0);
}




