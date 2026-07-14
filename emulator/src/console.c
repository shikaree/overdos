/* -------------------------------------------------------------------------- */
/* - Console I/O for the ÖverDOS Emulator                                    - */
/* - Copyright (c) Andreas Westling                                          - */
/* - GNU General Public License                                              - */
/* -------------------------------------------------------------------------- */
/* -                                                                        - */
/* - Cross-platform terminal layer.  See console.h for the contract.  The   - */
/* - two halves below present the same five functions on top of the         - */
/* - Windows console API and POSIX termios respectively.                    - */
/* -                                                                        - */
/* - Charset: the guest OS uses ISO-8859-1.  ISO-8859-1 is exactly Unicode  - */
/* - U+0000..U+00FF, which makes the mapping simple on both hosts:          - */
/* -   Windows - the console is switched to code page 1252 (a Latin-1       - */
/* -             superset) so raw output bytes render as-is, and input is    - */
/* -             read as wide (Unicode) characters, U+00xx -> byte 0xxx.     - */
/* -   POSIX   - terminals are UTF-8, so output bytes >=0x80 are encoded as  - */
/* -             two-byte UTF-8 and typed characters are UTF-8 decoded back  - */
/* -             to their code point (a byte when it is <=0xFF).             - */
/* -------------------------------------------------------------------------- */

#include "console.h"
#include <stdio.h>

static char con_pending = 0;   /* keystroke stashed by con_kbhit() */


/* ========================================================================== */
/* == Windows ============================================================== */
/* ========================================================================== */
#ifdef _WIN32

#include <conio.h>
#include <wchar.h>
#include <windows.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

int con_init(void)
{
 HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
 DWORD  mode = 0;

 /* Enable ANSI/VT100 escape handling so the OS's colours, cursor moves and
    screen clears are interpreted instead of shown as raw escape codes.  If
    stdout has been redirected, fall back to the real screen buffer (CONOUT$).
    Non-fatal: a pre-Win10 host or a "legacy console" window accepts the call
    but ignores the flag, so we confirm it took and warn with the fix. */
 if (!(hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &mode))) {
  hOut = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
                     FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
 }
 if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &mode)) {
  SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  GetConsoleMode(hOut, &mode);
  if (!(mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
   fprintf(stderr,
     "\n[overdos] This console does not support ANSI/VT100 output, so colours and\n"
     "          screen drawing (e.g. `top`) will appear as raw escape codes.\n"
     "          Fix: title-bar right-click > Properties > Options > uncheck\n"
     "          \"Use legacy console (requires relaunch)\", then reopen - or run\n"
     "          in Windows Terminal instead.\n\n");
  }
 }

 /* Interpret output bytes as Windows Latin-1 (a superset of ISO-8859-1) so the
    Swedish letters render correctly instead of as CP-437/850 line-draw junk. */
 SetConsoleOutputCP(1252);
 return 0;
}

void con_shutdown(void)
{
 /* Nothing to undo: conio input needs no mode change and the output code page
    is a harmless per-process setting. */
}

/* Read the key as a WIDE (Unicode) char with _getwch() rather than a code-page
   byte with _getch(): ISO-8859-1 == Unicode U+0000..U+00FF, so a Latin-1 letter
   maps straight onto its byte value (a=U+00E5 -> 0xE5).  _getch() would instead
   return the console's OEM code page (e.g. CP-850, a=0x86), which the OS does
   not understand - so the a/a/o keys would do nothing. */
int con_kbhit(void)
{
 if (_kbhit()) {
  wint_t wc = _getwch();
  con_pending = (wc <= 0xFF) ? (char) wc : 0;   /* only Latin-1 fits in one byte */
  return 1;
 }
 return 0;
}

void con_putbyte(int byte)
{
 putchar(byte & 0xFF);       /* console code page is 1252 */
 fflush(stdout);
}


/* ========================================================================== */
/* == POSIX (Linux, macOS, *BSD) =========================================== */
/* ========================================================================== */
#else

#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <stdlib.h>

static struct termios con_orig;
static int con_raw = 0;

/* Restore the terminal, then re-raise the signal's default action, so Ctrl-C
   quits the emulator without leaving the shell in raw mode. */
static void con_on_signal(int sig)
{
 con_shutdown();
 signal(sig, SIG_DFL);
 raise(sig);
}

int con_init(void)
{
 struct termios raw;

 /* If stdin is not a terminal (piped/redirected) there is nothing to switch;
    reads still work in the default mode. */
 if (!isatty(STDIN_FILENO)) {
  return 0;
 }
 if (tcgetattr(STDIN_FILENO, &con_orig) != 0) {
  return 0;
 }

 raw = con_orig;
 /* Unbuffered, no echo.  ISIG is kept on so Ctrl-C still quits; output
    post-processing (c_oflag) is left alone so a bare '\n' from the OS still
    maps to CR+LF, matching how the Windows console behaves. */
 raw.c_lflag &= ~(ICANON | ECHO);
 raw.c_cc[VMIN]  = 0;   /* non-blocking: return immediately... */
 raw.c_cc[VTIME] = 0;   /* ...with whatever is available (possibly nothing) */
 if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
  return 0;
 }

 con_raw = 1;
 atexit(con_shutdown);
 signal(SIGINT,  con_on_signal);
 signal(SIGTERM, con_on_signal);
 return 0;
}

void con_shutdown(void)
{
 if (con_raw) {
  tcsetattr(STDIN_FILENO, TCSANOW, &con_orig);
  con_raw = 0;
 }
}

/* Read one byte if available; if it starts a UTF-8 sequence, pull the
   continuation bytes and fold them back to a code point.  A code point <=0xFF
   is a Latin-1 byte the OS understands; anything larger is dropped (the OS has
   no representation for it), matching the Windows side. */
int con_kbhit(void)
{
 unsigned char c;
 unsigned int cp = 0;
 int extra = 0;

 if (read(STDIN_FILENO, &c, 1) != 1) {
  return 0;
 }

 if (c < 0x80) {
  cp = c;
 } else if ((c & 0xE0) == 0xC0) {
  cp = c & 0x1F; extra = 1;
 } else if ((c & 0xF0) == 0xE0) {
  cp = c & 0x0F; extra = 2;
 } else if ((c & 0xF8) == 0xF0) {
  cp = c & 0x07; extra = 3;
 } else {
  return 0;                       /* stray continuation / invalid lead: ignore */
 }

 if (c >= 0x80) {
  while (extra-- > 0) {
   unsigned char cc;
   int r, tries = 0;
   /* The continuation bytes are normally already buffered with the lead byte;
      spin briefly in the rare case they are split across reads. */
   while ((r = read(STDIN_FILENO, &cc, 1)) != 1 && ++tries < 4096) {
   }
   if (r != 1) {
    return 0;
   }
   cp = (cp << 6) | (cc & 0x3F);
  }
 }

 if (cp == 0 || cp > 0xFF) {
  return 0;                       /* NUL, or a character wider than one byte */
 }
 con_pending = (char) cp;
 return 1;
}

void con_putbyte(int byte)
{
 unsigned int c = (unsigned int) byte & 0xFF;
 if (c < 0x80) {
  putchar((int) c);
 } else {                         /* encode 0x80..0xFF as two-byte UTF-8 */
  putchar((int) (0xC0 | (c >> 6)));
  putchar((int) (0x80 | (c & 0x3F)));
 }
 fflush(stdout);
}

#endif


/* ========================================================================== */
/* == shared =============================================================== */
/* ========================================================================== */
char con_getch(void)
{
 return con_pending;
}
