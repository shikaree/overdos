#ifndef _ANSIMOD_H
#define _ANSIMOD_H
#include <mbox.h>
#include <ansi.h>
#include <vt100.h>
#include <stdarg.h>

#define PRINTF_BUFFER 		     1024
#define READLINE_SCREEN_WIDTH          80
#define ERR -1

#define size_t int

#define xy(x,y) (((y)<<8)|(x))

int itoa(const char *s, long long int i);
int itoa_wrap(const char *s, int i);
int getch(void);
int getcharne(void);
int itoahex(char *msg, unsigned int i);

int vsnprintf(char *str, size_t size, const char *format, va_list ap);
#define cprintf printf
#define printf _printf
#define sprintf _sprintf

int _printf(const char *format, ...);
int mprintf(int mbox_nr,  const  char  *format,  ...);
int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const  char  *format, ...);

int readline(char *line, char *prompt, char *tabcomp[]);
int readline_getarg(char *argument, char *line);

#endif
