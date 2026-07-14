/* -------------------------------------------------------------------------- */
/* - Initialization Procedures for ÖverDOS Emulator                        - */
/* - Copyright (c) Andreas Westling                                         - */
/* - GNU General Public License                                             - */
/* -------------------------------------------------------------------------- */

#include "regs.h"
#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "console.h"
#include "callback.h"

static void init_regs (STATE *state);
static int init_memory (MMU *mmu);
static MMU_NODE *init_memory_newarea (unsigned int addr, unsigned int size, 
                                      void (*callback)(void *, int, int, int));


/* -------------------------------------------------------------------------- */
/* - init()								    - */
/* - Main initialization function.   					    - */
/* - To be called when bootfile has been loaded.                            - */
/* - Returns 0 if everything is ok.                                         - */
/* -------------------------------------------------------------------------- */
int init(STATE *state) 
{
 state->pc = state->mmu.entry;

 /* Memory. */
 init_regs (state);
 if (init_memory(&state->mmu)) {
  return 1;
 }
 
 /* Keyboard. */
 if (con_init()) {
  return 1;
 }
 
 return 0;
}



/* -------------------------------------------------------------------------- */
/* - init_regs()                                                            - */
/* - Sets registers to init values.                                         - */
/* -------------------------------------------------------------------------- */
static void init_regs (STATE *state)
{

 /* PMON sets $sp. */
 state->regs[reg_sp] = 0x8077ffd8;
 
 /* Status register (as in PMON).
    Bit  1: Set IE = 1. 
    Bit 28: Coprocessor 0 usable.   
    Bit 11: Interrupt 1 on. 
    Bit 14: Interrupt 4 on.       */    
 state->regs0[reg0_sr] = 1 | (1<<28) | (1<<11) | (1<<14);

 return;
}


/* -------------------------------------------------------------------------- */
/* - init_memory()                                                          - */
/* - Adds extra special and general memory segments to mmu. 		    - */
/* - Returns 0 if no error.                                                 - */
/* -------------------------------------------------------------------------- */
static int init_memory (MMU *mmu)
{
 MMU_NODE *node;
 
 if (!mmu) {
  return 1;
 }
 
 /* --- UART0 data, area 0xbff80fe0 - 0xbff80ff8 --- 
    Frequently called so placed after memory allocated
    by ELF-loader.				       */
 node = init_memory_newarea (0xbff80fe0, 24, cb_uart0);
 if (node) {
  mmu->end->next = node;
  mmu->end = node;
 } else {
  return 1;
 }

 /* --- Exception handler jump area, 0xa0000000 - 0xa0000200 ---  
    Called once in a while and always allowed to write to
    since we only emulate kernel mode...                         */
 node = init_memory_newarea (0xa0000000, 512, NULL);
 if (node) {
  mmu->end->next = node;
  mmu->end = node;
 } else {
  return 1;
 } 


 /* --- RTC data, area 0xbff00000 - 0xbff00008 --- 
                                                      */     
 node = init_memory_newarea (0xbff00000, 8, cb_rtc);
 if (node) {
  mmu->end->next = node;
  mmu->end = node;
 } else {
  return 1;
 }

 
 /* --- LED data, area 0xbff20010 - 0xbff20020 --- 
    Not so frequently called but is a data-register, so
    it is placed before all the configuration registers
    who will (hopefully) only be called once or twice
    or so...                                            */     
 node = init_memory_newarea (0xbff20010, 32, cb_led);
 if (node) {
  mmu->end->next = node;
  mmu->end = node;
 } else {
  return 1;
 }

 /* --- Interrupt config, area 0xbff90000 - 0xbff90010 --- 
    Frequently called so placed after memory allocated
    by ELF-loader.				       */
 node = init_memory_newarea (0xbff90000, 16, cb_interrupt_config);
 if (node) {
  mmu->end->next = node;
  mmu->end = node;
 } else {
  return 1;
 }

 /* --- General area 0x80000000 - 0x8078ffd8 ---
    Extra available memory not allocated by the ELF loader.  Placed at the end
    of the list; where it overlaps an ELF segment the earlier (first-match)
    node wins in mmu_find(). */
 node = init_memory_newarea (0x80000000, 0x0078ffd8, NULL);
 if (node) {
  mmu->end->next = node;
  mmu->end = node;
 } else {
  return 1;
 }

 return 0;
}



static MMU_NODE *init_memory_newarea (unsigned int addr, unsigned int size, 
   				      void (*callback)(void *, int, int, int))
{
 MMU_NODE *node = NULL;
 
 node = (MMU_NODE *) malloc (sizeof(MMU_NODE));
 if (!node) {
  printf("INIT: Error when allocating new MMU node.\n");
  return NULL;
 }

 node->data = (char *) malloc (size);
 if (!node->data) {
  printf("INIT: Error when allocating new memory mapping area.\n");
  free(node);
  return NULL;
 }

 /* Zero the area (malloc gives garbage on Windows). */
 memset(node->data, 0, size);

 node->addr = addr;
 node->size = size;
 node->callback = callback;
 node->next = NULL;
   
 return node;
}
