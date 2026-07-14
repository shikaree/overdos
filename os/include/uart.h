/* ---------------------------------------------------------------- */ 
/* - Winbond W89777 Driver for ÖverDOS                            - */
/* ---------------------------------------------------------------- */

/* Position of the UART0 registers */
#define UART0_ADDR 0xbff80fe0


/* For interrrupt-driven I/O. */

#define UART_ICR	0xbff80fe4
#define UART_IMR	0xbff90000
#define UART_IXR0	0xbff9000c

/* For polling the UART. */

#define UART_USR 	0xbff80ff4
#define UART_TBR	0xbff80fe0
#define UART_RBR	0xbff80fe0

/* Output buffer size. */

#define UART_BUFFER_SIZE 256
#define UART_OUTPUT_SLICE MAX_MSG_LEN

#define UART_FOCUSCHAR '`'
#define UART_BASHCHAR  '\''
#ifndef _UART_H_ASM
 #include <mbox.h>
 extern mailbox mbox[];
 int driver_uart_init(void);
 int uart_send(char *msg, int size);
 int driver_uart_send(char b);
 int driver_uart_send_polled(char b);
 int driver_uart_receive(void);
 void uart_int(int force_function); 
 extern buffer_len;
#endif 

