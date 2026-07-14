#define SALLOC_DEFAULT_BLOCKSIZE 64

#define RAD2 {1,2,4,8,16,32,64,128}

#define GETBYTE(x)	(x>>3)
#define GETBIT(x)	(x%8)

#define SETBIT(x,b)	(x | exp2[b])
#define CLRBIT(x,b)	(x & (255-exp2[b]))
#define ISBIT(x,b)	(x & exp2[b])

typedef struct MEMAREA {
 void *memory;
 void *memstart;
 int size;
 int used;
 int markers;
 int blocksize; 
} MEMAREA;

int salloc_init(struct MEMAREA *memarea, void *memory, int size, int blocksize);
void *salloc (struct MEMAREA *memarea, int size);
void sfree(struct MEMAREA *memarea, void *memory);


