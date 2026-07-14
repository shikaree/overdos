/*
   Filesystem functions
*/

int main_tetris(void);
int main_bars(void);
int main_top(void);
int main_tic(void);
int main_time(void);
int main_nibbles(void);
int main_phil(void);
int main_phil_th(void);
int main_visky(void);
int main_bordello(void);
int main_prodcons(void);
int main_setdate(void);

unsigned char *files[]={"bärs","bordello","nibbles","phil","phil_th","prodcons","setdate","tetris","tic","time","top",
   			"visky",0};


int fs_dirlength(void) 
{
  int i=-1;

  while(files[++i]!=0);
  return i;
}


int fs_dirindex(char *name, int index) 
{
  static int length=-1;

  if (length==-1) {
    length=fs_dirlength();
  }

  if (index>length-1) {
    return -1;
  }

  strcpy(name,files[index]);

  return index;
}


void *fs_lookup(unsigned char *name)
{ 
 int i=0;
 int found=-1;
 void *ptr=0;

 while (files[i]) {
  if (!strcmp(name,files[i])) {
   found=i;
   break;
   }
  i++;
 }

 if (found==-1) {
  return (void *)0;
 }

 switch (found) {
  case 0:
   ptr=(void *)&main_bars; 
   break;
  case 1:
   ptr=(void *)&main_bordello;
   break;
  case 2:
   ptr=(void *)&main_nibbles;
   break;
  case 3:
   ptr=(void *)&main_phil;
   break;
  case 4:
   ptr=(void *)&main_phil_th;
   break;
  case 5:
   ptr=(void *)&main_prodcons;
   break;
  case 6:
   ptr=(void *)&main_setdate;
   break;
  case 7:
   ptr=(void *)&main_tetris;
   break;
  case 8:
   ptr=(void *)&main_tic;
   break;
  case 9:
   ptr=(void *)&main_time;
   break;
  case 10:
   ptr=(void *)&main_top;
   break;
  case 11:
   ptr=(void *)&main_visky;
   break;
  };
 return ptr;
}


