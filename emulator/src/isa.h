/* -------------------------------------------------------------------------- */
/* - Instruction Decoder for ÖverDOS Emulator                               - */
/* - Copyright (c) Andreas Westling                                         - */
/* - GNU General Public License                                             - */
/* -------------------------------------------------------------------------- */

#ifndef _ISA_H
 #define _ISA_H

 #include <stdio.h>
 #include "state.h"

#define ISA \
{"ADD",		"00000100000aaaaabbbbbccccc000000", \
 "ADDI",	"iiiiiiiiiiiiiiiiaaaaabbbbb000100", \
 "ADDIU",	"iiiiiiiiiiiiiiiiaaaaabbbbb100100", \
 "ADDU",	"10000100000aaaaabbbbbccccc000000", \
 "AND",		"00100100000aaaaabbbbbccccc000000", \
 "ANDI",	"iiiiiiiiiiiiiiiiaaaaabbbbb001100", \
 "BEQ",		"iiiiiiiiiiiiiiiiaaaaabbbbb001000", \
 "BEQL",	"iiiiiiiiiiiiiiiiaaaaabbbbb001010", \
 "BGEZ",	"iiiiiiiiiiiiiiii10000aaaaa100000", \
 "BGEZAL",	"iiiiiiiiiiiiiiii10001aaaaa100000", \
 "BGEZALL",	"iiiiiiiiiiiiiiii11001aaaaa100000", \
 "BGEZL",	"iiiiiiiiiiiiiiii11000aaaaa100000", \
 "BGTZ",	"iiiiiiiiiiiiiiii00000aaaaa111000", \
 "BGTZL",	"iiiiiiiiiiiiiiii00000aaaaa111010", \
 "BLEZ",	"iiiiiiiiiiiiiiii00000aaaaa011000", \
 "BLEZL",	"iiiiiiiiiiiiiiii00000aaaaa011010", \
 "BLTZ",	"iiiiiiiiiiiiiiii00000aaaaa100000", \
 "BLTZAL",	"iiiiiiiiiiiiiiii00001aaaaa100000", \
 "BLTZALL",	"iiiiiiiiiiiiiiii01001aaaaa100000", \
 "BLTZL",	"iiiiiiiiiiiiiiii01000aaaaa100000", \
 "BNE",		"iiiiiiiiiiiiiiiiaaaaabbbbb101000", \
 "BNEL",	"iiiiiiiiiiiiiiiiaaaaabbbbb101010", \
 "DIV",		"0101100000000000aaaaabbbbb000000", \
 "DIVU",	"1101100000000000aaaaabbbbb000000", \
 "ERET",	"00011000000000000000000001000010", \
 "J",		"iiiiiiiiiiiiiiiiiiiiiiiiii010000", \
 "JAL",		"iiiiiiiiiiiiiiiiiiiiiiiiii110000", \
 "JALR",	"10010000000aaaaa00000bbbbb000000", \
 "JR",		"000100000000000000000aaaaa000000", \
 "LB",		"iiiiiiiiiiiiiiiiaaaaabbbbb000001", \
 "LBU",		"iiiiiiiiiiiiiiiiaaaaabbbbb001001", \
 "LH",		"iiiiiiiiiiiiiiiiaaaaabbbbb100001", \
 "LHU",		"iiiiiiiiiiiiiiiiaaaaabbbbb101001", \
 "LL",		"iiiiiiiiiiiiiiiiaaaaabbbbb000011", \
 "LUI",		"iiiiiiiiiiiiiiiiaaaaa00000111100", \
 "LW",		"iiiiiiiiiiiiiiiiaaaaabbbbb110001", \
 "LWL",		"iiiiiiiiiiiiiiiiaaaaabbbbb010001", \
 "LWR",		"iiiiiiiiiiiiiiiiaaaaabbbbb011001", \
 "LWU",		"iiiiiiiiiiiiiiiiaaaaabbbbb111001", \
 "MFC0",	"00000000000aaaaabbbbb00000000010", \
 "MFHI",	"00001000000aaaaa0000000000000000", \
 "MFLO",	"01001000000aaaaa0000000000000000", \
 "MTC0",	"00000000000aaaaabbbbb00100000010", \
 "MTHI",	"100010000000000000000aaaaa000000", \
 "MTLO",	"110010000000000000000aaaaa000000", \
 "MULT",	"0001100000000000aaaaabbbbb000000", \
 "MULTU",	"1001100000000000aaaaabbbbb000000", \
 "NOR",		"11100100000aaaaabbbbbccccc000000", \
 "OR",		"10100100000aaaaabbbbbccccc000000", \
 "ORI",		"iiiiiiiiiiiiiiiiaaaaabbbbb101100", \
 "SB",		"iiiiiiiiiiiiiiiiaaaaabbbbb000101", \
 "SC",		"iiiiiiiiiiiiiiiiaaaaabbbbb000111", \
 "SH",		"iiiiiiiiiiiiiiiiaaaaabbbbb100101", \
 "SLL",		"000000iiiiiaaaaabbbbb00000000000", \
 "SLLV",	"00100000000aaaaabbbbbccccc000000", \
 "SLT",		"01010100000aaaaabbbbbccccc000000", \
 "SLTI",	"iiiiiiiiiiiiiiiiaaaaabbbbb010100", \
 "SLTIU",	"iiiiiiiiiiiiiiiiaaaaabbbbb110100", \
 "SLTU",	"11010100000aaaaabbbbbccccc000000", \
 "SRA",		"110000iiiiiaaaaabbbbb00000000000", \
 "SRAV",	"11100000000aaaaabbbbbccccc000000", \
 "SRL",		"010000iiiiiaaaaabbbbb00000000000", \
 "SRLV",	"01100000000aaaaabbbbbccccc000000", \
 "SUB",		"01000100000aaaaabbbbbccccc000000", \
 "SUBU",	"11000100000aaaaabbbbbccccc000000", \
 "SW",		"iiiiiiiiiiiiiiiiaaaaabbbbb110101", \
 "SWL",		"iiiiiiiiiiiiiiiiaaaaabbbbb010101", \
 "SWR",		"iiiiiiiiiiiiiiiiaaaaabbbbb011101", \
 "SYSCALL",	"001100iiiiiiiiiiiiiiiiiiii000000", \
 "XOR",		"01100100000aaaaabbbbbccccc000000", \
 "XORI",	"iiiiiiiiiiiiiiiiaaaaabbbbb011100", \
 NULL,		NULL}

#define ISA_SIZE 100

#define REGNAMES {"$zero",	"$at",	"$v0",	"$v1", \
   		  "$a0",	"$a1",	"$a2",	"$a3", \
                  "$t0",	"$t1",	"$t2",	"$t3", \
                  "$t4",	"$t5",	"$t6",	"$t7", \
                  "$s0",	"$s1",	"$s2",	"$s3", \
                  "$s4",	"$s5",	"$s6",	"$s7", \
                  "$t8",	"$t9",	"$k0",	"$k1", \
                  "$gp",	"$sp",	"$s8",	"$ra"}
          
          
#define ISA_HASH_PRIME		61
          

typedef struct ISA_INDEX {
 unsigned int nr;
 unsigned int pattern;
 unsigned int mask;
 int regstart[3];
 int immstart;
 int immend;
 struct ISA_INDEX *next;
} ISA_INDEX;

typedef struct {
 ISA_INDEX *indexes[ISA_HASH_PRIME];
} ISA_HASH;


/* Decoding (isa_dec.c) */
int isa_ismatch (unsigned int pattern, unsigned int mask, unsigned int instr);
ISA_HASH *isa_make_hash (void);
int isa_make_hashkey (unsigned int instr);
int isa_getbits (int start, int end, unsigned int value);
int isa_decode (unsigned int *r, unsigned int *i, unsigned int instr);
int isa_reorder (int value);
int isa_reorder_short (short int value);
void debugbits( unsigned int value);

/* Executing (isa_exec.c) */
void isa_run (STATE *state);

#endif
