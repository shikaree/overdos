/* -------------------------------------------------------------------------- */
/* - Console I/O for the ÖverDOS Emulator                                    - */
/* - Copyright (c) Andreas Westling                                          - */
/* - GNU General Public License                                              - */
/* -------------------------------------------------------------------------- */
/* -                                                                        - */
/* - A tiny cross-platform terminal layer.  The emulated UART talks to the  - */
/* - host terminal through these five calls; the platform-specific details  - */
/* - (Windows console API vs. POSIX termios) live in console.c.             - */
/* -                                                                        - */
/* - The guest OS speaks ISO-8859-1 (Latin-1): it emits and expects single  - */
/* - bytes: 0xE4 is 'a-diaeresis', 0xF6 'o-diaeresis', 0xE5 'a-ring'.        - */
/* - translate to and from whatever the host terminal actually uses so the  - */
/* - Swedish letters survive in both directions.                           - */
/* -------------------------------------------------------------------------- */

#ifndef CONSOLE_H
#define CONSOLE_H

/* Put the terminal into raw, non-blocking, no-echo mode and enable ANSI/VT100
   escape handling.  Returns 0 on success (never fatal - a plain terminal still
   works, it just may not interpret colours). */
int  con_init(void);

/* Restore the terminal to the state it had before con_init(). */
void con_shutdown(void);

/* Non-blocking keyboard poll.  Returns 1 and stashes the keystroke (fetch it
   with con_getch()) if one is waiting, otherwise returns 0. */
int  con_kbhit(void);

/* The keystroke stashed by the last successful con_kbhit(), as a Latin-1 byte. */
char con_getch(void);

/* Write one guest output byte (Latin-1) to the terminal, charset-correct. */
void con_putbyte(int byte);

#endif
