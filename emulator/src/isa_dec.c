/* -------------------------------------------------------------------------- */
/* - Instruction Decoder for ÖverDOS Emulator                               - */
/* - Copyright (c) Andreas Westling                                         - */
/* - GNU General Public License                                             - */
/* -------------------------------------------------------------------------- */


#include <stdlib.h>
#include <string.h>
#include "isa.h"

/* The boot image is big-endian MIPS.  On a little-endian host (x86, ARM, ...)
   the fetched instruction word must be byte-swapped before decoding; on a
   big-endian host it is already in the right order.  gcc and clang expose the
   host byte order through __BYTE_ORDER__, so no -D flag is needed. */
#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
# define HOST_BIG_ENDIAN 1
#else
# define HOST_BIG_ENDIAN 0
#endif

/* -------------------------------------------------------------------------- */
/* - isa_decode()                                                           - */
/* - Expects an instruction in normal MIPS MSB format.                      - */
/* - Returns register numbers in r[0..2] and immediates in i.               - */
/* - Instruction number is returned by the function.                        - */
/* - Returns -1 if an error occured.                                        - */
/* -------------------------------------------------------------------------- */
int isa_decode (unsigned int *r, unsigned int *i, unsigned int instr)
{
 static ISA_HASH *hash=NULL;
 ISA_INDEX *index;
 int k,j;

#if !HOST_BIG_ENDIAN
 instr = isa_reorder (instr);
#endif
  /* Initialize hash if necessary. */
 if (!hash) {
  hash = isa_make_hash ();
  if (!hash) {
   printf("ISA: Decoder Initialization Error. Unable to create hash.\n");
   return -1;
  }
 }

 /* Calculate a key from instruction. */
 k = isa_make_hashkey(instr);

 /* Search for key. */
 index = hash->indexes[k];

 while (index) {
  if (isa_ismatch(index->pattern, index->mask, instr)) {
   /* Extract all registers if there are any. */
   for (j=0; j<3; j++) {
    r[j] = -1;
    if (index->regstart[j] != -1) {
     r[j] = isa_getbits(index->regstart[j], index->regstart[j]+4, instr);
    }
   }
   /* Extract immediates. */
   if (index->immstart != -1) {
    *i = isa_getbits(index->immstart, index->immend, instr);
   }
   return index->nr;
  }
  index=index->next;
 }

 return -1;
}



/* -------------------------------------------------------------------------- */
/* - isa_ismatch()                                                          - */	
/* - Matches an instruction agains a pattern not considering bits in mask.  - */
/* - 0 if no match and 1 if match.                                          - */
/* -------------------------------------------------------------------------- */
int isa_ismatch (unsigned int pattern, unsigned int mask, unsigned int instr) 
{

 /* Zero out all bits in mask from instr. */
 instr = (instr | mask) ^ mask;

/* printf("Patt: ");
 debugbits(pattern);
 printf("Mask: ");
 debugbits(mask);*/
 
 /* Compare. */
 return  (instr == pattern);
}



/* -------------------------------------------------------------------------- */
/* - isa_make_hash()                                                        - */
/* - Creates and allocates a ISA_HASH from ISA.				    - */
/* - Returns NULL if an error occured.					    - */
/* - It does NOT clean up after itself (program should exit anyway).        - */
/* - Should only be called ONCE.					    - */
/* -------------------------------------------------------------------------- */
ISA_HASH *isa_make_hash (void)
{
 static char *isa[] = ISA;
 ISA_HASH *hash;
 ISA_INDEX *index;
 int i=-1, j, b, k;
 char *first, *last;
 
 /* Create and initialize a hash. */
 hash = (ISA_HASH *) malloc (sizeof(ISA_HASH));
 if (!hash) {
  return NULL;
 }
 for (j=0; j<ISA_HASH_PRIME; j++) {
  hash->indexes[j] = NULL;
 } 
 
 /* Create indexes. */
 while (isa[i+=2]) {
  index = (ISA_INDEX *) malloc (sizeof(ISA_INDEX));  
  if (!index) {
   return NULL;
  }
  
  /* Make a pattern and mask. 
     Patterns are the '0's and '1's in ISA.
     Masks are 'i', 'a' ... in ISA (to be masked
     out before comparing with pattern).         */
  index->pattern = 0;
  index->mask = 0;
  for (b = 0; b<32; b++) {
   if (isa[i][b] == '1') {
    index->pattern |= (1<<b); 
   } else if (isa[i][b] != '0') {
    index->mask |= (1<<b);
   }
  }
  
  /* Find all registers and immediate. */
  for (j=0;j<3;j++) {
   index->regstart[j] =-1;
   first = strchr(isa[i],97+j);
   if (first) {
    index->regstart[j] = (int)(first - isa[i]);
   }
  }
  index->immstart = -1;
  index->immend = -1;
  first = strchr(isa[i],'i');
  if (first) {
   index->immstart = (int)(first - isa[i]);
   last = strrchr(isa[i],'i');
   index->immend = (int)(last - isa[i]);
  }
   
  index->nr = i / 2; 
  
  /* Create a hash-key and connect to that index. */
  k = isa_make_hashkey(index->pattern);
      
  index->next = hash->indexes[k];
  hash->indexes[k] = index;
 }

 return hash;
}



/* -------------------------------------------------------------------------- */
/* - isa_make_hashkey()                                                     - */
/* - Calculates a key for a given value (instruction)			    - */
/* -------------------------------------------------------------------------- */
int isa_make_hashkey (unsigned int instr)
{
 unsigned int key;
 
 /* If high 6 bits is 0 then use low 6 bits. 
    This complicates things a bit when it comes
    to certain B*-operations, but cannot be helped. */
 key = isa_getbits(26,31,instr);

 if (!key) {
  key = isa_getbits(0,4,instr);
 } 

 return (key % ISA_HASH_PRIME);
}



/* -------------------------------------------------------------------------- */
/* - isa_getbits()                                                          - */
/* - Extracts all bits in value from bit start to bit end.                  - */
/* - Byte-order safe.							    - */
/* -------------------------------------------------------------------------- */
int isa_getbits (int start, int end, unsigned int value)
{
 unsigned int width = (unsigned int)(end - start + 1);
 unsigned int mask = (width >= 32) ? 0xFFFFFFFFu : ((1u << width) - 1u);

 return (value >> start) & mask;
}



/* -------------------------------------------------------------------------- */
/* - isa_reorder()                                                          - */
/* - Converts from MSB to LSB format and vice versa.                        - */
/* -------------------------------------------------------------------------- */
int isa_reorder (int value)
{
 return (int) __builtin_bswap32 ((unsigned int) value);
}



/* -------------------------------------------------------------------------- */
/* - isa_reorder_short()                                                    - */
/* - Converts from 16 bit MSB to LSB format and vice versa.                 - */
/* -------------------------------------------------------------------------- */
int isa_reorder_short (short int value)
{
 return (short int) __builtin_bswap16 ((unsigned short) value);
}



/* -------------------------------------------------------------------------- */
/* - debugbits()                                                            - */
/* - Just prints out the bits.					            - */
/* -------------------------------------------------------------------------- */
void debugbits( unsigned int value) 
{
 int b;
 
 for (b = 0; b<32; b++){
  if (value & (1<<b)) {
   printf("1");
  } else {
   printf("0");
  }
 }
 printf("\n");
 
 return;
}



