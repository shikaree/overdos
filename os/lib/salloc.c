/* ---------------------------------------------------------------- */
/* - Simple Library Implementation of malloc for ÖverDOS          - */
/* ---------------------------------------------------------------- */

/* Usage of any of these functions should be avoided at all cost,
   since they are inherently dangerous and wastes memory.
   Speed shouldn't be anything to worry about though, since
   optimizations have been done to speed up the process.
   
   External fragmentation depends on the blocksize. Larger 
   blocksize means more fragmentation. On smaller blocksizes you
   loose memory on the int-value (1 WORD) at the beginning of each 
   allocated segment.
   
   Thus the waste for each segment is:
    waste = segmentsize - 4 bytes - (bytes requested by user)
    
   Where segmentsize is a multiple of requested blocksize in
   salloc_init(). 
   
   Goes thru compilation with -Wall -ansi and -pedantic. <eg>
   
                                                                    */

#include <salloc.h>


static void salloc_set_blocks(struct MEMAREA *memarea, int start, int blocks);
static void salloc_clr_blocks(struct MEMAREA *memarea, int start, int blocks);
static int salloc_fnf (struct MEMAREA *memarea, int blocks);

int exp2[]=RAD2;




/* ---------------------------------------------------------------- */
/* - salloc_init()                                                - */
/* - Makes memory into a dynamically allocated memory handled by  - */
/* - memarea. It fills memarea with necessary information to      - */
/* - do allocating/deallocating.                                  - */
/* - After this has been done on an array, NEVER EVER NEVER use   - */
/* - it in any other way than thru the salloc_*-functions!        - */
/* -                                                              - */
/* - Blocksize can be 0, in which case the default value is used. - */
/* - Returns 1 if sucessfull and 0 if not.                        - */
/* ---------------------------------------------------------------- */
int salloc_init(struct MEMAREA *memarea, void *memory, int size, int blocksize) 
{
 int markers;
 int i;
 
 if (size<=0) {
  return 0;
 }
 
 if (blocksize<=0) {
  blocksize=SALLOC_DEFAULT_BLOCKSIZE;
 }
 
 /* Calculate number of markers. */
 markers=(size/blocksize)+((size%blocksize)>0);
 
 /* Adjust to even byte. */
 while (markers%8) markers++;
 
 /* Make sure that we don't exceed our memory size. */
 while ((markers/8 + markers*blocksize)>size) {
  markers-=8;
  if (markers<=0) {
   return 0;
  }
 }
 
 /* Do we have enought memory? */
 if (markers/8 >= size) {
  return 0;
 }
 
 /* Initialize markers. */
 for (i=0;i<markers/8;i++) {
  *((char *)memory+i)=0;
 }
 
 /* Fill in data. */
 memarea->memory=memory;
 memarea->memstart=(void *)((char *)memory+markers/8);
 memarea->size=size;
 memarea->used=0;
 memarea->markers=markers;
 memarea->blocksize=blocksize;
 
 return size-markers/8;
}




/* ---------------------------------------------------------------- */
/* - salloc()                                                     - */
/* - Allocates a specified number of bytes.                       - */
/* -                                                              - */
/* - Returns pointer to memory if sucessfull and 0 (NULL) if not. - */
/* ---------------------------------------------------------------- */
void *salloc (struct MEMAREA *memarea, int size) 
{
 int blockstart;
 int blocks; 
 int *sizemarker;
 
 if (!memarea || size<1) {
  return (void *)0;
 }
 
 /* Add extra space for size. */
 size+=sizeof(int);
  
 /* Do we have enough memory? */
 if (size>memarea->size-(memarea->used+memarea->markers/8)) {
  return (void *)0;
 }
 
 /* Calculate number of blocks. */
 blocks=(size/memarea->blocksize)+((size%memarea->blocksize)>0);

 /* Find start. */
 blockstart=salloc_fnf(memarea,blocks);
 if (blockstart==-1) {
  return (void *)0;
 }
  
 /* Set the size-marker. */
 sizemarker=(int *)((char *)memarea->memstart+blockstart*memarea->blocksize);
 *sizemarker=blocks;
 
 /* Mark them all used. */
 salloc_set_blocks(memarea, blockstart,blocks);
 
 /* Add to used. */
 memarea->used+=memarea->blocksize*blocks;
 
 /* Return pointer. */
 return (void *)(sizemarker+1);
}




/* ---------------------------------------------------------------- */
/* - sfree()    	                                          - */
/* - Deallocates a pointer returned from salloc().                - */
/* - Make sure you pass EXACTLY the same adress to this function  - */
/* - that you received from salloc()!				  - */
/* ---------------------------------------------------------------- */
void sfree(struct MEMAREA *memarea, void *memory) 
{
 int *sizemarker;
 int block;
 
 /* Get the blocknumber. */
 block=(((char *)memory-(char *)memarea->memstart)-sizeof(int))/memarea->blocksize;
  
 /* Get the sizemarker. */
 sizemarker=(int *)((char *)memory-sizeof(int));
 
 /* Check that we don't go outside. */
 if (*sizemarker+block > memarea->markers) {
  return;
 } 

 /* Clear the memory. */
 salloc_clr_blocks(memarea, block, *sizemarker) ;
 memarea->used-=*sizemarker*memarea->blocksize;
  
 return;
}





/* ---------------------------------------------------------------- */
/* - STATIC FUNCTIONS                                             - */
/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
/* - salloc_set_blocks()                                          - */
/* - Marks a number of blocks as allocated.                       - */
/* ---------------------------------------------------------------- */
static void salloc_set_blocks(struct MEMAREA *memarea, int start, int blocks) 
{
 int i;
 char *ptr;
 
 ptr=memarea->memory;
 for (i=start; i<start+blocks; i++) {
  ptr[GETBYTE(i)]=SETBIT(ptr[GETBYTE(i)],GETBIT(i));
 }
 return;
}




/* ---------------------------------------------------------------- */
/* - salloc_clr_blocks()                                          - */
/* - Marks a number of blocks as not allocated.                   - */
/* ---------------------------------------------------------------- */
static void salloc_clr_blocks(struct MEMAREA *memarea, int start, int blocks) 
{
 int i;
 char *ptr;
 
 ptr=memarea->memory;
 for (i=start; i<start+blocks; i++) {
  ptr[GETBYTE(i)]=CLRBIT(ptr[GETBYTE(i)],GETBIT(i));
 }
 return;
}




/* ---------------------------------------------------------------- */
/* - salloc_fnf()     	                                          - */
/* - Finds next free hole of memory of a specific size.           - */
/* ---------------------------------------------------------------- */
static int salloc_fnf (struct MEMAREA *memarea, int blocks) 
{
 unsigned char *ptr;
 int i,j;
 int blockstart=-1, blocksize=-1;
 
 /* Is there memory at all? */
 if (memarea->size-memarea->used < blocks*memarea->blocksize) {
  return -1;
 }
 
 ptr=(unsigned char *)memarea->memory;
 
 /* Find byte with empty space. Optimized. */
 for (i=0;i<memarea->markers>>3;i++) {
  if (ptr[i]!=255) {
  
   /* Found one. Go into bitmode. */
   for (j=i<<3;j<memarea->markers;j++) {
    if (!ISBIT(ptr[GETBYTE(j)],GETBIT(j))) {
     if (blockstart==-1) {
      blockstart=j;
      blocksize=0;
     }
     blocksize++;
     if (blocksize==blocks) {
      return blockstart;
     }
    }
    else {
     blockstart=-1;
     /* Byteborder? If so, we can break into bytemode. */
     if (GETBIT(j)==7) {
      i=GETBYTE(j)+1;
      break;
     }
    }
   }
   
  }
 }

 return -1;
}



