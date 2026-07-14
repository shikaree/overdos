/* -------------------------------------------------------------------------- */
/* - Memory Management Unit for ÖverDOS Emulator                            - */
/* - Copyright (c) Andreas Westling                                         - */
/* - GNU General Public License                                             - */
/* -------------------------------------------------------------------------- */

#ifndef MMU_H
 #define MMU_H

 typedef struct MMU_NODE {
  unsigned int addr;
  unsigned int size;
  char *data;
  void (*callback)(void *, int, int, int);
  struct MMU_NODE *next; 
 } MMU_NODE;

 typedef struct MMU {
  unsigned int entry;
  MMU_NODE *end;
  MMU_NODE *next;
 } MMU;

 MMU_NODE *mmu_node_new(MMU *mmu);
 MMU_NODE *mmu_addr2node (int addr, MMU *mmu);
 int mmu_store(MMU *mmu, void *data, int vaddr, int size);
 int mmu_load(MMU *mmu, void *data, int vaddr, int size);
 int load_elf(MMU *mmu, const char *filename);

 #define MMU_LOAD  0
 #define MMU_STORE 1

#endif
