#include <os.h>
#include <ansi-mod.h>
#include <vt100.h>
#include <ansi.h>

int atoi2(int x){
return (x-48);

}

int main_setdate() {
int a,in[80],i=0;
int year,month,day,hour,min,sec;
printf("\x0d\nFormat: Year:Month:Day:Hour:Min:Sec");
printf("\x0d\nTwo digits! ex, 02:01:10:23:03:34");
printf("\x0d\nNew date? ");
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



    in[i]=0;
    }

   year=10*atoi2(in[0])+atoi2(in[1]);
   month=10*atoi2(in[3])+atoi2(in[4]);
   day=10*atoi2(in[6])+atoi2(in[7]);
   hour=10*atoi2(in[9])+atoi2(in[10]);
   min=10*atoi2(in[12])+atoi2(in[13]);
   sec=10*atoi2(in[15])+atoi2(in[16]);

printf("\x0d\n%i:%i:%i:%i:%i:%i",year,month,day,hour,min,sec);

    if ((year>-1) && (year<101) &&
       (month>0) && (month<13) &&
       (day>0) && (day<33) &&
	(min>-1) && (min<60) &&
	(sec>-1) && (sec<60)) {
	wclock_year(year);
	wclock_month(month);
	wclock_day_month(day);
	wclock_hour(hour);
	wclock_min(min);
	wclock_sec(sec);
        while ((a=getchar()!=13)) { }
	return 1;
   }
	
  printf("\x0d\nNo can do!");

  while ((a=getchar()!=13)) { }
  return 0;	
}
