#include <asmclock.h>
#include <ansi-mod.h>
#include <os.h>
int readclock_sec(){
return (readclock(0));
}

int readclock_min(){
return (readclock(2));
}

int readclock_hour(){
return (readclock(4));
}

int readclock_day_week(){
return readclock(6);
}

int readclock_day_month(){
return readclock(7);
}

int readclock_month(){
return readclock(8);
}

int readclock_year(){
return readclock(9);
}

int writeclock_sec(char x){
if (x<61) writeclock(0,x);
return 1;
}

int writeclock_min(char x){
if (x<61) writeclock(2,x);
return 1;
}

int writeclock_hour(char x){
if (x<25) writeclock(4,x);
return 1;
}

int writeclock_day_of_week(char x){
if ((x>0)&&(x<8)) writeclock(6,x);
return 1;
}

int writeclock_day_of_month(char x){
if ((x>0)&&(x<32)) writeclock(7,x);
return 1;
}

int writeclock_month(char x){
if ((x>0)&&(x<13)) writeclock(8,x);
return 1;
}

int writeclock_year(char x){
if (x<100) writeclock(9,x);
return 1;
}

int print_clock() {
char x,y,z;

x=readclock_hour();
y=readclock_min();
z=readclock_sec();
if (x<10) printf("\x0d\n0%i",x); else printf("\x0d\n%i",x); 
printf(":");
if (y<10) printf("0%i",y); else printf("%i",y); 
printf(":");
if (z<10) printf("0%i",z); else printf("%i",z); 

return 1;
}

int print_date() {
char x,y,z;
x=readclock_year();
y=readclock_month();
z=readclock_day_month();
if (x<10) printf("\x0d\n0%i",x); else printf("\x0d\n%i",x); 
printf(".");
if (y<10) printf("0%i",y); else printf("%i",y); 
printf(".");
if (z<10) printf("0%i",z); else printf("%i",z); 
return 1;
}
 
int set_time(char h, char m, char s){
writeclock_hour(h);
writeclock_min(m);
writeclock_sec(s);
return 1;

}

int set_date(char y, char m, char d){
writeclock_year(y);
writeclock_month(m);
writeclock_day_of_month(d);
return 1;
}
