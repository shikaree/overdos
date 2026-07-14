/* ---------------------------------------------------------------- */
/* - os.h							  - */
/* - This file should be included from all programs that feel	  - */
/* - like using syscalls.					  - */
/* ---------------------------------------------------------------- */

void syscall(int func, int a0, int a1, int a2);

int create_process(char *name, int prior);
int kill_process(int nr);
int set_priority(int nr, int prior);
int get_priority(int nr);
int get_name(char *buffer,int nr);
int get_parent(int p); 
int get_pin(void);
int get_state(int p);

int give_focus(int p);

int create_mbox(int size,char *name);
int destroy_mailbox(int nr);
int send_msg(char *msg, int size, int nr);
int receive_msg(char *buffer, int nr);
int receive_msg_cond(char *buffer, int nr);

int led(char *buffer, int size);

int get_timer(void);
int get_timer_interval(void);
void sleep(int t);
void get_uptime (int *days, int *hours, int *minutes);

int register_focusswitch(int *switchvar);

int rclock_sec();
int rclock_min();
int rclock_hour();
int rclock_day_week();
int rclock_day_month();
int rclock_month();
int rclock_year();
int wclock_sec(char sec);
int wclock_min(char min);
int wclock_hour(char h);
int wclock_day_week(char d);
int wclock_day_month(char d);
int wclock_month(char m);
int wclock_year(char y);
int printclock(); /* Do not use */
int printdate(); /* Do not use */
int settime(char h, char m, char s);
int setdate(char x, char y, char z);

int dir_length(void);
int dir_index(char *name, int index);

void reboot(void);







