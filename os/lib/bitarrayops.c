/* Use setbit to set bit val in the imagined bitarray arr[] to 1*/

void setbit(unsigned char arr[], int val){
  arr[val>>3] |= (1<<(val&7));
  return;
}

/* Use resetbit to set bit val in the imagined bitarray arr[] to 0*/

void resetbit(unsigned char arr[], int val){
  unsigned char c;
  c = 255;
  c ^= (1<<(val&7));
  arr[val>>3] &= c;
  return;
}
 
/* Use getbit to get the value, 0 or 1, of bit val in the imagined 
   bitarray */

unsigned char getbit(unsigned char arr[], int val){
  return (((arr[val>>3])&(1<<(val&7)))!=0);}
 
