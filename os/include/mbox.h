#ifndef _MBOX_H
#define _MBOX_H
#define MAX_MBOX_NAME 16
#define MAX_BLOCKED 16
#define MAX_MSG_LEN 128

#include <pcb.h>

/* meddelanden */
typedef struct mbox_msg {
	char msg[MAX_MSG_LEN];	
	int size;
        } mbox_msg;

typedef struct mbox_name {
   	int mbox_nr;
        char bokstav;
} mbox_name;

/*blockerade processer 
typedef struct mbox_blocked {
	int pin;
	} mbox_blocked;
*/

typedef struct mailbox {	
	char active;
        char name[MAX_MBOX_NAME];
	int number; 
	int size;
	mbox_msg msgs[MAX_MSG];
        int msg_front;
        int msg_rear;
        int msg_count;
	int counter;
	int blocked[MAX_BLOCKED];
        int blocked_front;
        int blocked_rear;
        int blocked_count;
	}  mailbox;
mailbox mbox[MAX_MBOX];

void proc_block(int pin, int mbox_nr);
void proc_unblock(int mbox_nr);
void fifo_2linear(int front, int rear, int *left, int *right);
int mbox_translate(int mbox_nr, int pin);
void mbox_init();
int mbox_name2nr(char *name);
int mbox_create(int size, char *name);
int mbox_destroy(int);
int send(char *msg,int size,int mbox_nr);
int receive(char *msg, int mbox_nr);
int receive_cond(char *msg, int mbox_nr);


#endif
