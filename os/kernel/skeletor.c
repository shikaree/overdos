#include <ansi-mod.h>

void kprint(char *);

void skeletor(int i)
{
   char adolf[12];
   itoa(adolf,(long long int)i);
   kprint("Skeletor sez: ");
   kprint(adolf);
   kprint("\x0d\n");
}
