/* -------------------------------------------------------------------------- */
/* - Memory Management Unit for ÖverDOS Emulator                            - */
/* - Copyright (c) Andreas Westling                                         - */
/* - GNU General Public License                                             - */
/* -------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mmu.h"


MMU_NODE *mmu_node_new(MMU *mmu)
{
 MMU_NODE *node;
 
 if (!mmu) {
  return NULL;
 }
 
 /* Set up mmu for first time. */
 mmu_addr2node (0, mmu);
 
 node = (MMU_NODE *) malloc(sizeof(MMU_NODE));
 
 if (!node) {
  return NULL;
 }

 node->addr = 0;
 node->size = 0;
 node->data = NULL;
 node->callback = NULL;
 
 node->next = NULL;
 
 if (mmu->end == NULL) {
  mmu->next = node;
  mmu->end = node;
 } else {
  mmu->end->next = node;
  mmu->end = node;
 }
 
 return node;
}


/* True when [vaddr, vaddr+size) lies inside node.  Comparisons are unsigned
   (node->addr is unsigned, so vaddr is promoted) - matches the original. */
#define MMU_IN(node,vaddr,size) \
 ((node)->addr <= (vaddr) && ((node)->addr + (node)->size) >= ((vaddr) + (size)))

static MMU_NODE *mmu_find(MMU *mmu, int vaddr, int size)
{
 MMU_NODE *node;

 for (node = mmu->next; node; node = node->next) {
  if (MMU_IN(node, vaddr, size)) {
   return node;
  }
 }

 return NULL;
}


int mmu_store(MMU *mmu, void *data, int vaddr, int size)
{
 MMU_NODE *node = mmu_find(mmu, vaddr, size);

 if (!node) {
  /* Unmapped: caller turns this into a TLB (address) exception. */
  return 1;
 }

 if (data) {
  memcpy(node->data + (vaddr - node->addr), data, size);
 }

 if (node->callback) {
  node->callback(data, size, vaddr - node->addr, MMU_STORE);
 }

 return 0;
}


int mmu_load(MMU *mmu, void *data, int vaddr, int size)
{
 MMU_NODE *node = mmu_find(mmu, vaddr, size);

 if (!node) {
  /* Unmapped: caller turns this into a TLB (address) exception. */
  return 1;
 }

 if (node->callback) {
  node->callback(data, size, vaddr - node->addr, MMU_LOAD);
 }

 if (data) {
  memcpy(data, node->data + (vaddr - node->addr), size);
 }

 return 0;
}



MMU_NODE *mmu_addr2node (int addr, MMU *mmu)
{
 static MMU *static_mmu = NULL;
 MMU_NODE *node;
 
 if (!addr || mmu) {
   static_mmu = mmu; 
 }

 node = static_mmu->next;
 while (node) {
  if (addr>= node->addr && addr<= (node->addr + node->size)) {
   return node;
  }
  node = node->next;
 }
 
 return NULL;
}
