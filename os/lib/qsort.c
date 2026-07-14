/* ---------------------------------------------------------------- */
/* - qsort                                                        - */
/* ---------------------------------------------------------------- */ 

#include <ansi.h>

static void qsort_helper(void *base, size_t nmemb, size_t size,
           int(*compar)(const void *, const void *), int left, int right);
static void qsort_swap(void *ptr1, void *ptr2, size_t size);



 
void qsort(void *base, size_t nmemb, size_t size,
           int(*compar)(const void *, const void *)) 
{             
 return qsort_helper(base, nmemb, size, compar,0,nmemb-1);             
}



 
static void qsort_helper(void *base, size_t nmemb, size_t size,
           int(*compar)(const void *, const void *), int left, int right) { 
 int last,i;
 
 if (left>=right) {
  return;
 }
 
 qsort_swap(base+left*size, base+((left+right)>>1)*size,size);
 last = left;
 for (i=left+1;i<=right;i++) {
  if (compar(base+i*size,base+left*size)<0) {
   qsort_swap(base+(++last)*size, base+i*size,size);
  }
 }
 qsort_swap(base+left*size, base+last*size,size);
 
 qsort_helper(base,nmemb,size,compar,left,last);
 qsort_helper(base,nmemb,size,compar,last+1,right); 
 
 return;
} 



 
static void qsort_swap(void *ptr1, void *ptr2, size_t size) 
{
 unsigned char holder;
 int i;
 
 for (i=0;i<size;i++) {
  holder = *(((unsigned char *)ptr1)+i);
  *(((unsigned char *)ptr1)+i) = *(((unsigned char *)ptr2)+i);
  *(((unsigned char *)ptr2)+i) = holder;
 }
 
 return;
} 
  

