/* -------------------------------------------------------------------------- */
/* - Exit functions for ÖverDOS Emulator                                    - */
/* - Copyright (c) Andreas Westling                                         - */
/* - GNU General Public License                                             - */
/* -------------------------------------------------------------------------- */

#include <stdlib.h>
#include "console.h"
#include "mmu.h"
#include "state.h"
#include "exit.h"

static void exit_free_node (MMU_NODE *node);

void exit_exit (STATE *state) 
{
 MMU_NODE *node;
 
 if (state) {
  node = state -> mmu.next;

  exit_free_node (node);
 }
  
 con_shutdown();
 
 exit(0);
}


static void exit_free_node (MMU_NODE *node) 
{

 if (!node) {
  return;
 }
 
 exit_free_node (node->next);
 
 free(node);
 
 return;
}
