/* -------------------------------------------------------------------------- */
/* - ÖverDOS Emulator - entry point                                          - */
/* - Copyright (c) Andreas Westling                                          - */
/* - GNU General Public License                                              - */
/* -------------------------------------------------------------------------- */
/* -                                                                        - */
/* - Loads a big-endian MIPS ELF boot image (the ÖverDOS OS by default),    - */
/* - sets up the emulated Algorithmics P4032 machine, and runs it.          - */
/* -                                                                        - */
/* -   overdos [-v] [bootfile]                                              - */
/* -                                                                        - */
/* -   -v         verbose boot (print the ELF header and memory map)         - */
/* -   bootfile   path to the boot image (default: "bootfile")              - */
/* -------------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include "mmu.h"
#include "isa.h"
#include "exit.h"

/* When set, the ELF loader and init dump their progress (see elf.c). */
int emu_verbose = 0;

int main(int argc, char **argv)
{
 STATE state;
 MMU_NODE *node;
 const char *bootfile = "bootfile";
 int i;

 for (i = 1; i < argc; i++) {
  if (strcmp(argv[i], "-v") == 0) {
   emu_verbose = 1;
  } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
   printf("usage: %s [-v] [bootfile]\n", argv[0]);
   return 0;
  } else {
   bootfile = argv[i];
  }
 }

 /* Unbuffered output so the interactive OS shows characters immediately
    (and so diagnostics survive a crash). */
 setvbuf(stdout, NULL, _IONBF, 0);

 /* Start from a clean machine state (regs, cp0 regs, hi/lo all zero). */
 memset(&state, 0, sizeof(state));

 if (load_elf(&state.mmu, bootfile)) {
  fprintf(stderr, "overdos: could not load boot image '%s'\n", bootfile);
  return 1;
 }

 if (emu_verbose) {
  for (node = state.mmu.next; node != NULL; node = node->next) {
   printf("Allocated %u bytes for VM at 0x%08x.\n", node->size, node->addr);
  }
 }

 if (init(&state)) {
  exit_exit(&state);
 }

 isa_run(&state);
 return 0;
}
