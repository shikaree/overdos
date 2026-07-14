/* -------------------------------------------------------------------------- */
/* - Driver for MIPS 4032 LED - declarations				    - */
/* -------------------------------------------------------------------------- */

#define LED_REG_ADDR 	 0xbff20010
#define LED_FIFO_SIZE 	 1024

/* Times per second to output to LED. Must be higher than 0. */
#define LED_OUTPUT_SPEED 3

typedef struct LED_FIFO {
 char buf[LED_FIFO_SIZE];
 int front;
 int rear;
 int count;
} LED_FIFO;

void driver_led_init(void);
void driver_led_setchar (char character, int number);
int driver_led_send(char *msg, int size, int kernel);

