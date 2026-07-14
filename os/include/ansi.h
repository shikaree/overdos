#ifndef _ANSI_H
#define _ANSI_H

#define size_t int
#define RAND_MAX 2147483647

/* Locales. LOCALE_LOWER and LOCALE_UPPER MUST be of equal length! */
#define LOCALE_DIGIT "0123456789"

/* SE locale or standard C locale. C locale is default. */
#ifdef LOCALE_SE
 #define LOCALE_UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZÅÄÖ"
 #define LOCALE_LOWER "abcdefghijklmnopqrstuvwxyzåäö"
 #define LOCALE_LENGTH 29
#else
 #define LOCALE_UPPER "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
 #define LOCALE_LOWER "abcdefghijklmnopqrstuvwxyz"
 #define LOCALE_LENGTH 26
#endif

/* Add IACs too? */
#define LOCALE_CNTRL "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x10\x11\x12\x13\x14\x15\x16\x17x18\x19\x1A\x1B\x1C\x1D\x1E\x1F" 
#define LOCALE_SPACE " \f\n\r\t\v"

#define memcpy _memcpy
void *_memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int bcmp(const void *s1, const void *s2, size_t n);

char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
size_t strlen(const char *s);
int strcasecmp(const char *s1, const char *s2);

void srand(unsigned int seed);
int rand(void);

int isalnum(int c);
int isalpha(int c);
int isascii(int c);
int isblank(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);

int toupper(int c);
int tolower(int c);

int atoi(const char *nptr);

int getchar(void);

void qsort(void *base, size_t nmemb, size_t size,
           int(*compar)(const void *, const void *));


#endif





