#include <os.h>
#include <ansi-mod.h>
#include <vt100.h>
#include <ansi.h>

int main_time() {
int x,y,z,d,d2,m,yr;
yr=rclock_year();
m=rclock_month();
d=rclock_day_week();
d2=rclock_day_month();
x=rclock_hour();
y=rclock_min();
z=rclock_sec();

switch(d){
   case 1: printf("\x0d\nSun "); break; 
   case 2: printf("\x0d\nMon "); break;
   case 3: printf("\x0d\nTue "); break;
   case 4: printf("\x0d\nWed "); break;
   case 5: printf("\x0d\nThu "); break;
   case 6: printf("\x0d\nFri "); break;
   case 7: printf("\x0d\nSat "); break;
}

switch(m){
   case 1: printf("Jan "); break;
   case 2: printf("Feb "); break;
   case 3: printf("Mar "); break;
   case 4: printf("Apr "); break;
   case 5: printf("May "); break;
   case 6: printf("Jun "); break;
   case 7: printf("Jul "); break;
   case 8: printf("Aug "); break;
   case 9: printf("Sep "); break;
   case 10: printf("Oct "); break;
   case 11: printf("Nov "); break;
   case 12: printf("Dec "); break;
}
printf("%i ",d2);

if (x<10) printf("0%i",x); else printf("%i",x); 
printf(":");
if (y<10) printf("0%i",y); else printf("%i",y); 
printf(":");
if (z<10) printf("0%i",z); else printf("%i",z); 

if (yr<10) printf(" 0%i",yr); else printf(" %i",yr);
printf("\n\x0d");
return 1;
}

