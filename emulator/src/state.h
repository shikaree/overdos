/* -------------------------------------------------------------------------- */
/* - Emulation State Types for ÖverDOS Emulator                             - */
/* - Copyright (c) Andreas Westling                                         - */
/* - GNU General Public License                                             - */
/* -------------------------------------------------------------------------- */

#ifndef _STATE_H
 #define _STATE_H

 #include "mmu.h"

typedef struct {
 unsigned int pc;
 unsigned int regs[32];
 unsigned int regs0[32];
 unsigned int hi;
 unsigned int lo;
 MMU mmu;
} STATE;

int init(STATE *state);

#endif

