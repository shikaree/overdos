/* -------------------------------------------------------------------------- */
/* - Instruction Execution Engine for ÖverDOS Emulator                      - */
/* - Copyright (c) Andreas Westling                                         - */
/* - GNU General Public License                                             - */
/* -------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include "mmu.h"
#include "state.h"
#include "regs.h"
#include "isa.h"

/* --- host monotonic clock -------------------------------------------------
   Returns a monotonically increasing time in nanoseconds, used to drive the
   guest timer off the real wall clock (see the timer block in isa_run) so the
   guest's sense of time tracks reality regardless of interpreter speed. */
#ifdef _WIN32
#include <windows.h>
static unsigned long long host_ns (void)
{
 static LARGE_INTEGER freq; static int have_freq = 0;
 LARGE_INTEGER now;
 if (!have_freq) { QueryPerformanceFrequency(&freq); have_freq = 1; }
 QueryPerformanceCounter(&now);
 return (unsigned long long)((double)now.QuadPart * 1e9 / (double)freq.QuadPart);
}
#else
#include <time.h>
static unsigned long long host_ns (void)
{
 struct timespec ts;
 clock_gettime(CLOCK_MONOTONIC, &ts);
 return (unsigned long long)ts.tv_sec * 1000000000ULL + (unsigned long long)ts.tv_nsec;
}
#endif

/* Register numbers are 5-bit fields (0-31).  isa_decode() stores -1 for an
   operand slot an instruction doesn't use; masking with 31 keeps that (and any
   other stray value) in bounds.  On the original 32-bit build an unmasked
   (unsigned)-1 index wrapped harmlessly, but on 64-bit it is a wild access. */
#define GPR(x) 	       state->regs[i_regs[x] & 31]
#define CPR(x)	       state->regs0[i_regs[x] & 31]
#define BRANCH_DELAY   (state->pc + 4 + ((int)((signed short int)(i_imm))) * 4 )
#define EXC_MMU_LOAD	1
#define EXC_MMU_STORE	2
#define EXC_SYSCALL	3

/* --- per-process HI/LO virtualization -------------------------------------
   The ÖverDOS interrupt handler (kernel/regs2pcb.sx save_regs) destroys HI/LO
   on every interrupt - it does `mul $k0,$k0,$k1` to compute a PCB offset - and
   the PCB has no slot to preserve them.  So any user-mode multiply/divide
   result still "in flight" (computed but not yet moved out with mflo/mfhi)
   when a timer tick lands is silently corrupted.  itoa() - used to format
   every colour code and cursor coordinate - is full of div/mul, so the
   corruption scaled with how fast a program was redrawing (garbled tetris /
   nibbles, "blocks left behind").  We fix it faithfully by shadowing HI/LO per
   process, keyed by the kernel's pcb_executing: save on exception entry,
   restore on ERET, so each process keeps its own HI/LO across interrupts and
   context switches. */
#define PCB_EXECUTING_ADDR 0x8003328c   /* &pcb_executing (int), from the ELF symtab */
#define HILO_SLOTS 66                    /* pcb_executing is -1..MAX_PROCESSES(64); index = pcb+1 */
static unsigned int hilo_hi[HILO_SLOTS], hilo_lo[HILO_SLOTS];
static unsigned char hilo_valid[HILO_SLOTS];

static int hilo_index (STATE *state)
{
 unsigned int v = 0;
 int p;
 mmu_load(&state->mmu, (void *)&v, PCB_EXECUTING_ADDR, 4);
 p = (int)isa_reorder(v) + 1;            /* -1 (idle) maps to slot 0 */
 if (p < 0) p = 0;
 if (p >= HILO_SLOTS) p = HILO_SLOTS - 1;
 return p;
}
static void hilo_save (STATE *state)     /* on exception entry */
{
 int i = hilo_index(state);
 hilo_hi[i] = state->hi; hilo_lo[i] = state->lo; hilo_valid[i] = 1;
}
static void hilo_restore (STATE *state)  /* on ERET, keyed by the resuming process */
{
 int i = hilo_index(state);
 if (hilo_valid[i]) { state->hi = hilo_hi[i]; state->lo = hilo_lo[i]; }
}

void isa_run (STATE *state)
{
 unsigned int instruction;
 unsigned int branch=0;
 unsigned int delayed_jump_address=0;
 unsigned int i_regs[3], i_imm;
 unsigned int address;
 unsigned int exception=0;
 unsigned int value_int;
 unsigned char value_char;
 unsigned short int value_short;
 unsigned long long value_long;
 static char *isa[] = ISA;
 unsigned int debug_imm;
 /* Real (wall-clock) time base for the timer, so guest time tracks reality. */
 unsigned long long t_last = host_ns(), t_now;
 int timer_poll = 0;

do {
#ifdef PERFCOUNT
  { static unsigned long long _n=0;
    if(((++_n)%20000000ULL)==0) fprintf(stderr,"[%lluM instrs]\n",_n/1000000ULL); }
#endif
  /* Fetch the instruction. */
  mmu_load (&state->mmu, (void *)&instruction, state->pc, 4);

  /* Decode it. */
  debug_imm = instruction;              /* keep the raw word for diagnostics */
  instruction = isa_decode (i_regs, &i_imm, instruction);

  /* isa_decode returns -1 (== 0xffffffff here) for an encoding that is not
     in the ISA table.  Guard against it so we don't index used[]/isa[] out
     of bounds (which would corrupt the stack or crash). */
  if (instruction > 70) {
   printf("ISA: Undecodable instruction word 0x%08x at pc 0x%08x\n",
          isa_reorder(debug_imm), state->pc);
   exit(1);
  }

  /* Process it. */
  if (!branch) {
   delayed_jump_address = 0;
  }

  branch = 0;

  switch (instruction) {
   case 0: /* ADD - Add */
    GPR(0) = GPR(2) + (signed)GPR(1);
    break;
   case 1: /* ADDI - Add immediate */ 
    GPR(0) = GPR(1) + (signed short int) i_imm;
    break;
   case 2: /* ADDIU - Add immediate unsigned */ 
    GPR(0) = GPR(1) + (signed short int) i_imm;
    break;		
   case 3: /* ADDU - Add unsigned */
    GPR(0) = GPR(2) + (signed)GPR(1);
    break;
   case 4: /* AND - And */ 
    GPR(0) = GPR(2) & GPR(1);
    break;
   case 5: /* ANDI - And immediate */ 
    GPR(0) = GPR(1) & (i_imm);
    break;
   case 6: /* BEQ - Branch on equal */
    branch = 1;
    if (GPR(0) == GPR(1)) {
     delayed_jump_address = BRANCH_DELAY;
    }
    break;
   case 12: /* BGTZ - Branch on greater than zero. */
    branch = 1;
    if ((signed)GPR(0) > 0) {
     delayed_jump_address = BRANCH_DELAY;   
    }
    break;
   case 14: /* BLEZ - Branch on less than or equal to zero */
    branch = 1;
    if ((signed)GPR(0) <= 0) {
     delayed_jump_address = BRANCH_DELAY;   
    }
    break;
   case 16: /* BLTZ - Branch on less than zero */
    branch = 1;
    if ((signed)GPR(0) < 0) {
     delayed_jump_address = BRANCH_DELAY;
    }
    break;
   case 20: /* BNE - Branch on not equal */
    branch = 1;
    if (GPR(0) != GPR(1)) {
     delayed_jump_address = BRANCH_DELAY;
    } 
    break;
   case 22: /* DIV - Divide */
    if (GPR(0) != 0) {
     state->lo = (signed)GPR(1) / (signed)GPR(0);
     state->hi = (signed)GPR(1) % (signed)GPR(0);
    } 
    break;
   case 24: /* ERET - Exception return */
    /* Resume at EPC exactly (no delay slot).  Route it through
       delayed_jump_address with branch=0 so the post-switch pc update sets
       pc = EPC instead of adding the usual +4.  (The old code did EPC+4 here
       and then the main loop added another +4, i.e. EPC+8 - which skipped two
       instructions of any interrupted process and crashed it.) */
    delayed_jump_address = state->regs0[reg0_epc];
    branch = 0;
    state->regs0[reg0_sr] = (state->regs0[reg0_sr] | 2)^2;  /* clear EXL */
    hilo_restore(state);          /* restore the resuming process's HI/LO */
    break;
   case 25: /* J - Jump */
    branch = 1;
    delayed_jump_address = (state->pc + 4) & ((1<<28)|(1<<29)|(1<<30)|(1<<31));
    delayed_jump_address |= (((unsigned)i_imm)<<2);
    break;  
   case 26: /* JAL - Jump and link. */
    branch = 1;
    delayed_jump_address = (state->pc + 4) & ((1<<28)|(1<<29)|(1<<30)|(1<<31));
    delayed_jump_address |= (((unsigned)i_imm)<<2);
    state->regs[reg_ra] = state->pc + 8;
    break;      
   case 28: /* JR - Jump register. */
    branch = 1;
    delayed_jump_address = GPR(0); 
    break;
   case 29: /* LB - Load byte. */
    address = (signed short)i_imm + GPR(1);
    if (mmu_load(&state->mmu, (void *)&value_char, address, 1)==1) {
     exception = EXC_MMU_LOAD;
    } else {
     GPR(0) = (signed char)value_char;
    }
    break;
   case 30: /* LBU - Load byte unsigned */
    address = (signed short)i_imm + GPR(1);
    if (mmu_load(&state->mmu, (void *)&value_char, address, 1)==1) {
     exception = EXC_MMU_LOAD;
    } else {
     GPR(0) = (unsigned char)value_char;
    }
    break;
   case 34: /* LUI - Load upper immediate */
    GPR(0) = ((unsigned)i_imm << 16);
    break ;
   case 35: /* LW - Load word */
    address = (signed short)i_imm + GPR(1);
    if (mmu_load(&state->mmu, (void *)&value_int, address, 4)==1) {
     exception = EXC_MMU_LOAD;
    } else {
     GPR(0) = (signed)(isa_reorder(value_int));
    }
    break;
   case 36: /* LWL - Load word left (big-endian, standard MIPS semantics) */
    /* Load the aligned word, shift the wanted bytes into the HIGH end of the
       register and keep the low (address&3) bytes.  The old code read a partial
       word and OR-ed it in, which merged stale register bits. */
    address = (signed short)i_imm + GPR(1);
    if (mmu_load(&state->mmu, (void *)&value_int, address & ~3u, 4)==1) {
     exception = EXC_MMU_LOAD;
    } else {
     unsigned int W = isa_reorder(value_int);      /* big-endian word value */
     unsigned int sh = (address & 3) * 8;
     GPR(0) = (W << sh) | (GPR(0) & ((1u << sh) - 1u));
    }
    break;
   case 37: /* LWR - Load word right (big-endian, standard MIPS semantics) */
    address = (signed short)i_imm + GPR(1);
    if (mmu_load(&state->mmu, (void *)&value_int, address & ~3u, 4)==1) {
     exception = EXC_MMU_LOAD;
    } else {
     unsigned int W = isa_reorder(value_int);
     unsigned int sh = (3 - (address & 3)) * 8;
     unsigned int lm = 0xFFFFFFFFu >> sh;          /* mask of the loaded low bytes */
     GPR(0) = (W >> sh) | (GPR(0) & ~lm);
    }
    break;
   case 39: /* MFC0 - Move from coprocessor 0 register */    
    GPR(1) = CPR(0);
    break;
   case 41: /* MFLO - Move from LO */
    GPR(0) = state->lo;
    break;
   case 42: /* MTCO - Move to compr0cessor 0 register */
    CPR(0) = GPR(1);
    break;
   case 45: /* MULT - Multiply */
    value_long = (signed long long)GPR(1) * (signed long long)GPR(0);
    state->lo = (value_long<<32)>>32;
    state->hi = (value_long>>32);
    break;
   case 46: /* MULTU - Multiply unsigned */
     value_long = (long long)GPR(1) * (long long)GPR(0);
     state->lo = (value_long<<32)>>32;
     state->hi = (value_long>>32);
     break;
   case 48: /* OR - Or */
    GPR(0) = GPR(2) | GPR(1);
    break;
   case 49: /* ORI - Or immediate */
    GPR(0) = GPR(1) | (unsigned short)i_imm;
    break;
   case 50: /* SB - Store byte */
    address = (signed short)i_imm + GPR(1);
    value_char = GPR(0);
    if (mmu_store(&state->mmu, (void *)&value_char, address, 1)==1) {
     exception = EXC_MMU_STORE;
    }
    break;
   case 53: /* SLL - Shift left logical */
    GPR(0) = (unsigned)GPR(1) << (unsigned short)i_imm;     
    break;
   case 54: /* SLLV - Shift left logical variable */
    GPR(0) = (unsigned)GPR(1) << (unsigned)GPR(2);     
    break;
   case 55: /* SLT - Set on less than */
     if ((signed)GPR(2) < (signed)GPR(1)) {
     GPR(0) = 1;
    } else {
     GPR(0) = 0;
     } 
    break;
   case 56: /* SLTI - Set on less than immediate */
    if ((signed)GPR(1) < (signed short)i_imm) {
     GPR(0) = 1;
    }  else {
     GPR(0) = 0;
    }
    break;
   case 57: /* SLTIU - Set on less than immediate unsigned*/
    if ((unsigned)GPR(1) < (unsigned)i_imm) {
     GPR(0) = 1;
    }  else {
     GPR(0) = 0;
    }
    break;
   case 58: /* SLTU - Set on less than unsigned */
    if ((unsigned)GPR(2) < (unsigned)GPR(1)) {
     GPR(0) = 1;
    } else {
     GPR(0) = 0;
    } 
    break;
   case 59:  /* SRA - Shift right arithmetic */
    GPR(0) = (signed int)GPR(1) >> (i_imm & 31);
    break;
   case 61: /* SRL - Shift right logical */
    GPR(0) = (unsigned)GPR(1) >> i_imm;
    break;
   case 64: /* SUBU - Subtract unsigned */
    GPR(0) = (signed)GPR(2) - (signed)GPR(1);
    break;
   case 65: /* SW - Store word */
    address = (signed short)i_imm + GPR(1);
    value_int = isa_reorder(GPR(0));

    if (mmu_store(&state->mmu, (void *)&value_int, address, 4)==1) {
     exception = EXC_MMU_STORE;
    }
    break;
   case 66: /* SWL - Store word left (big-endian, standard MIPS semantics) */
    /* Store the register's HIGH bytes from `address` up to the word boundary,
       most-significant first.  The old code's SWR counterpart dropped a byte
       whenever the store address was 1 (mod 4), silently corrupting memory. */
    address = (signed short)i_imm + GPR(1);
    {
     unsigned int rt = GPR(0), k;
     unsigned char b;
     for (k=0; k <= 3-(address&3); k++) {
      b = (rt >> (24 - k*8)) & 0xff;
      if (mmu_store(&state->mmu, (void *)&b, address+k, 1)==1) { exception = EXC_MMU_STORE; break; }
     }
    }
    break;
   case 67: /* SWR - Store word right (big-endian, standard MIPS semantics) */
    /* Store the register's LOW bytes ending at `address`, least-significant
       first, walking DOWN to the word boundary. */
    address = (signed short)i_imm + GPR(1);
    {
     unsigned int rt = GPR(0), k;
     unsigned char b;
     for (k=0; k <= (address&3); k++) {
      b = (rt >> (k*8)) & 0xff;
      if (mmu_store(&state->mmu, (void *)&b, address-k, 1)==1) { exception = EXC_MMU_STORE; break; }
     }
    }
    break;
   case 68: /* SYSCALL - Syscall */
    exception = EXC_SYSCALL;
    break;
   case 69: /* XOR - Xor */
    GPR(0) = GPR(2) ^ GPR(1);
    break;
   case 70: /* XORI - Xor immediate */
    GPR(0) = GPR(1) ^ i_imm;
    break;

   /* ----------------------------------------------------------------- */
   /* - Instructions added for full ISA coverage (compiled C code uses - */
   /* - these; hitting an unimplemented one used to exit()).           - */
   /* ----------------------------------------------------------------- */
   case 7: /* BEQL - Branch on equal likely */
    branch = 1;
    if (GPR(0) == GPR(1)) {
     delayed_jump_address = BRANCH_DELAY;
    } else {
     branch = 0; delayed_jump_address = 0; state->pc += 4; /* nullify slot */
    }
    break;
   case 8: /* BGEZ - Branch on greater than or equal to zero */
    branch = 1;
    if ((signed)GPR(0) >= 0) {
     delayed_jump_address = BRANCH_DELAY;
    }
    break;
   case 9: /* BGEZAL - Branch on >= 0 and link */
    branch = 1;
    state->regs[reg_ra] = state->pc + 8;
    if ((signed)GPR(0) >= 0) {
     delayed_jump_address = BRANCH_DELAY;
    }
    break;
   case 10: /* BGEZALL - Branch on >= 0 and link, likely */
    branch = 1;
    state->regs[reg_ra] = state->pc + 8;
    if ((signed)GPR(0) >= 0) {
     delayed_jump_address = BRANCH_DELAY;
    } else {
     branch = 0; delayed_jump_address = 0; state->pc += 4;
    }
    break;
   case 11: /* BGEZL - Branch on >= 0 likely */
    branch = 1;
    if ((signed)GPR(0) >= 0) {
     delayed_jump_address = BRANCH_DELAY;
    } else {
     branch = 0; delayed_jump_address = 0; state->pc += 4;
    }
    break;
   case 13: /* BGTZL - Branch on > 0 likely */
    branch = 1;
    if ((signed)GPR(0) > 0) {
     delayed_jump_address = BRANCH_DELAY;
    } else {
     branch = 0; delayed_jump_address = 0; state->pc += 4;
    }
    break;
   case 15: /* BLEZL - Branch on <= 0 likely */
    branch = 1;
    if ((signed)GPR(0) <= 0) {
     delayed_jump_address = BRANCH_DELAY;
    } else {
     branch = 0; delayed_jump_address = 0; state->pc += 4;
    }
    break;
   case 17: /* BLTZAL - Branch on < 0 and link */
    branch = 1;
    state->regs[reg_ra] = state->pc + 8;
    if ((signed)GPR(0) < 0) {
     delayed_jump_address = BRANCH_DELAY;
    }
    break;
   case 18: /* BLTZALL - Branch on < 0 and link, likely */
    branch = 1;
    state->regs[reg_ra] = state->pc + 8;
    if ((signed)GPR(0) < 0) {
     delayed_jump_address = BRANCH_DELAY;
    } else {
     branch = 0; delayed_jump_address = 0; state->pc += 4;
    }
    break;
   case 19: /* BLTZL - Branch on < 0 likely */
    branch = 1;
    if ((signed)GPR(0) < 0) {
     delayed_jump_address = BRANCH_DELAY;
    } else {
     branch = 0; delayed_jump_address = 0; state->pc += 4;
    }
    break;
   case 21: /* BNEL - Branch on not equal likely */
    branch = 1;
    if (GPR(0) != GPR(1)) {
     delayed_jump_address = BRANCH_DELAY;
    } else {
     branch = 0; delayed_jump_address = 0; state->pc += 4;
    }
    break;
   case 23: /* DIVU - Divide unsigned */
    if (GPR(0) != 0) {
     state->lo = (unsigned)GPR(1) / (unsigned)GPR(0);
     state->hi = (unsigned)GPR(1) % (unsigned)GPR(0);
    }
    break;
   case 27: /* JALR - Jump and link register */
    branch = 1;
    delayed_jump_address = GPR(1);       /* rs = jump target */
    GPR(0) = state->pc + 8;              /* rd = return address */
    break;
   case 31: /* LH - Load halfword */
    address = (signed short)i_imm + GPR(1);
    if (mmu_load(&state->mmu, (void *)&value_short, address, 2)==1) {
     exception = EXC_MMU_LOAD;
    } else {
     GPR(0) = (signed short)isa_reorder_short((signed short)value_short);
    }
    break;
   case 32: /* LHU - Load halfword unsigned */
    address = (signed short)i_imm + GPR(1);
    if (mmu_load(&state->mmu, (void *)&value_short, address, 2)==1) {
     exception = EXC_MMU_LOAD;
    } else {
     GPR(0) = (unsigned short)isa_reorder_short((unsigned short)value_short);
    }
    break;
   case 33: /* LL - Load linked (treated as LW) */
    address = (signed short)i_imm + GPR(1);
    if (mmu_load(&state->mmu, (void *)&value_int, address, 4)==1) {
     exception = EXC_MMU_LOAD;
    } else {
     GPR(0) = (signed)(isa_reorder(value_int));
    }
    break;
   case 38: /* LWU - Load word unsigned */
    address = (signed short)i_imm + GPR(1);
    if (mmu_load(&state->mmu, (void *)&value_int, address, 4)==1) {
     exception = EXC_MMU_LOAD;
    } else {
     GPR(0) = (unsigned)(isa_reorder(value_int));
    }
    break;
   case 40: /* MFHI - Move from HI */
    GPR(0) = state->hi;
    break;
   case 43: /* MTHI - Move to HI */
    state->hi = GPR(0);
    break;
   case 44: /* MTLO - Move to LO */
    state->lo = GPR(0);
    break;
   case 47: /* NOR - Nor */
    GPR(0) = ~(GPR(2) | GPR(1));
    break;
   case 51: /* SC - Store conditional (always succeeds here) */
    address = (signed short)i_imm + GPR(1);
    value_int = isa_reorder(GPR(0));
    if (mmu_store(&state->mmu, (void *)&value_int, address, 4)==1) {
     exception = EXC_MMU_STORE;
    } else {
     GPR(0) = 1;
    }
    break;
   case 52: /* SH - Store halfword */
    address = (signed short)i_imm + GPR(1);
    value_short = isa_reorder_short(GPR(0));
    if (mmu_store(&state->mmu, (void *)&value_short, address, 2)==1) {
     exception = EXC_MMU_STORE;
    }
    break;
   case 60: /* SRAV - Shift right arithmetic variable */
    GPR(0) = (signed int)GPR(1) >> (GPR(2) & 31);
    break;
   case 62: /* SRLV - Shift right logical variable */
    GPR(0) = (unsigned)GPR(1) >> (GPR(2) & 31);
    break;
   case 63: /* SUB - Subtract */
    GPR(0) = (signed)GPR(2) - (signed)GPR(1);
    break;

   default:
    printf("ISA: Unknown instruction %u (%s) at pc 0x%08x\n",
           instruction, isa[instruction*2], state->pc);
    exit(1);
  }

  if (exception == EXC_SYSCALL) {
   hilo_save(state);             /* preserve the caller's HI/LO */
   state->regs0[reg0_sr] |= 2;   /* set EXL: mask interrupts during the handler */
   state->regs0[reg0_cause] = 0 | 32;
   /* EPC = the syscall instruction itself.  The kernel's syscall handler
      adds 4 to the saved PC, so ERET (which now resumes at EPC exactly)
      continues just after the syscall. */
   state->regs0[reg0_epc] = state->pc;
   delayed_jump_address = 0;
   branch = 0;
   state->pc = 0xa0000180;
  }

  if (exception == EXC_MMU_LOAD || exception == EXC_MMU_STORE) {
   /* An access to an unmapped address.  Deliver a TLB (address) exception to
      the kernel instead of aborting the emulator.  The kernel's spim_stuff
      handler uses TLBL/TLBS to emulate SPIM-style memory-mapped console I/O
      (programs such as `tic` talk to a UART at 0xFFFF0000); a genuinely bad
      access becomes a kernel panic that kills only the offending process. */
   hilo_save(state);                            /* preserve the faulting process's HI/LO */
   state->regs0[reg0_sr] |= 2;                  /* set EXL: mask interrupts */
   state->regs0[8] = address;                  /* BadVAddr (CP0 reg 8) */
   if (exception == EXC_MMU_LOAD) {
    state->regs0[reg0_cause] = (2 << 2);        /* ExcCode 2 = TLBL */
   } else {
    state->regs0[reg0_cause] = (3 << 2);        /* ExcCode 3 = TLBS */
   }
   if (delayed_jump_address) {                  /* faulted in a branch delay slot */
    state->regs0[reg0_cause] |= 0x80000000;     /* Cause.BD */
    state->regs0[reg0_epc] = state->pc - 4;     /* the branch instruction */
   } else {
    state->regs0[reg0_epc] = state->pc;         /* the faulting instruction */
   }
   delayed_jump_address = 0;
   branch = 0;
   state->pc = 0xa0000180;
  }

  if (!exception) {
    if (!branch && delayed_jump_address) {
      state->pc = delayed_jump_address;
    } else {
      state->pc+=4;
    }
  }

  /* --- Timer Handling ---
     First check that interrupt is on (bit 1) and
     then that timer interrupt is on (bit 15).    
     
     To make it easy we don't allow timer interrupts
     to occur in branches.                           */
  if ((state->regs0[reg0_sr] & 1)  &&
      !(state->regs0[reg0_sr] & 2) &&   /* EXL clear: not masked by exception level */
      (state->regs0[reg0_sr] & (1<<15)) &&
      !branch &&
      state->pc<0xa0000000) {
   /* Fire the timer based on REAL elapsed wall-clock time (not instruction
      count) so the guest's sense of time - tetris piece-drop speed, sleep(),
      uptime, ... - tracks reality regardless of how fast this interpreter
      runs.  The kernel programmed compare = interval_us * 67 (it assumes a
      67 MHz CPU), so one tick is compare/67 microseconds of real time. */
   if (++timer_poll >= 256 && state->regs0[reg0_compare] > 0) {
    timer_poll = 0;
    t_now = host_ns();
    /* compare is in 67 MHz ticks (the kernel sets compare = interval_us * 67),
       so one tick is compare/67 microseconds = compare*1000/67 nanoseconds. */
    if (t_now - t_last >=
        (unsigned long long)state->regs0[reg0_compare] * 1000ULL / 67ULL) {
     t_last = t_now;
     /* Timer interrupt! */
     hilo_save(state);             /* preserve the interrupted process's HI/LO */
     state->regs0[reg0_sr] |= 2;   /* set EXL: mask interrupts during the handler */
     state->regs0[reg0_count]=state->regs0[reg0_compare];
     state->regs0[reg0_cause]=0 | (1<<15);
     state->regs0[reg0_epc] = state->pc;
     delayed_jump_address = 0;
     state->pc = 0xa0000180;
    }
   }
  }
  
  /* This is to fix up jumps for interrupthandlers. */
  if (state->pc >= 0xa0000200) {
   state->pc -= 0x20000000;
  }
  if (delayed_jump_address >= 0xa0000200) {
   delayed_jump_address -= 0x20000000;
  }
  
  /* Force $zero to be $zero. */
  state->regs[reg_zero] = 0;
    
  exception = 0;
 } while (1);
}
