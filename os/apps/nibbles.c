/* ---------------------------------------------------------------- */
/* - Nibbles for ÖverDOS by Andreas Westling                      - */
/* ---------------------------------------------------------------- */
#include <ansi-mod.h>
#include <ansi.h>
#include <vt100.h>
#include <nibbles.h>
#include <os.h>

int main_nibbles(void)
{
 signed char wormx[(SCREEN_X-1)*(SCREEN_Y-1)];
 signed char wormy[(SCREEN_X-1)*(SCREEN_Y-1)];
 signed char applex=(SCREEN_X-1)/2;
 signed char appley=(SCREEN_Y-1)/2;
 char alive=1;
 int wormlength=3;
 signed char dirx=1, diry=0;
 int key,i,j, had_apple=0;
 int score=0;
 int focusswitch;
       
 wormx[0]=3; wormx[1]=2; wormx[2]=1;
 wormy[0]=10; wormy[1]=10; wormy[2]=10;
    			
 register_focusswitch(&focusswitch);
      
 cprintf("%2",COLOR_BACKGROUND);
 redraw(wormx,wormy,applex, appley,wormlength); 
 
 while(alive) {
  if (focusswitch) {
   redraw(wormx,wormy,applex, appley,wormlength); 
   focusswitch=0;
  }
  key=getch();
  switch(key) {
   case 'i':
    dirx=0; diry=-1;
    break;
   case 'k':
    dirx=0; diry=1;
    break;
   case 'l':
    dirx=1; diry=0;
    break;
   case 'j':
    dirx=-1; diry=0;
    break;
   case 'q':
    return 0;   
   case 'r':
    redraw(wormx,wormy,applex,appley,wormlength);
    break; 
   } 
   
   /* Set color for worm. */
   cprintf("%1",COLOR_WORM);
   
   /* Remove last. */
   if (!had_apple) {
    cprintf("%@ ",xy(wormx[wormlength-1]+2,wormy[wormlength-1]+2));
   }
   
   /* Shuffle wormsegments. */ 
   for (i=wormlength;i>0;i--) {
    wormx[i]=wormx[i-1];
    wormy[i]=wormy[i-1];
   }
   
   if (had_apple) {
    wormlength++;
   }
   
   had_apple=0;
   
   /* Move head. */
   wormx[0]+=dirx;
   wormy[0]+=diry;
    
   /* Draw head and next-to-head. */
   cprintf("%1%@%c",COLOR_WORM_HEAD,xy(wormx[0]+2,wormy[0]+2),CHAR_WORM_HEAD);
   cprintf("%1%@%c%@",COLOR_WORM,xy(wormx[1]+2,wormy[1]+2),CHAR_WORM,xy(1,1));
   
   /* Check crash. */
   for (i=0;i<wormlength;i++) {
    for(j=i+1;j<wormlength;j++) {
     if (wormx[i]==wormx[j] && wormy[i]==wormy[j]) {
      alive=0;
      break;
     }
    } 
   }
   /* With borders... */
   if (alive) {
    for (i=0;i<wormlength;i++) {
      if (wormx[i]<0 || wormx[i]>=SCREEN_X-2 ||
          wormy[i]<0 || wormy[i]>=SCREEN_Y-2) {
       alive=0;   
       break;
      }
    }
   } 
   /* With apple. */
   if (wormx[0]==applex && wormy[0]==appley){
    score++;
    /* Update score. */
    cprintf("%1%@Score: %i",COLOR_SCORE,xy(1,SCREEN_Y+1),score);
    /* New apple. */
    applex=-1;
    while (applex==-1) {
     applex=rand()%(SCREEN_X-3);
     appley=rand()%(SCREEN_Y-3);
     for (i=0;i<wormlength;i++) {
      if (applex==wormx[i] && appley==wormy[i]) {
       applex=-1;
       break;
      }
     }
    }
    had_apple=1;
    cprintf("%1%@%c", COLOR_APPLE,xy(applex+2,appley+2),CHAR_APPLE);  
   }

  sleep(100000-score*1000);
 }

 return 0;
}


/* --- redraws screen totally --- */
void redraw(char *wormx, char *wormy, char applex, char appley, int wormlength) 
{
 int y,x;

 cprintf("%0");

 /* Horizontal lines. */ 
 cprintf("%1%@",COLOR_BORDER,xy(1,1));
 for (x=0;x<SCREEN_X;x++) {
  cprintf("%c",CHAR_BORDER);
 }
 cprintf("%@",xy(1,SCREEN_Y));
 for (x=0;x<SCREEN_X;x++) {
  cprintf("%c",CHAR_BORDER);
 }

 /* Vertical lines. */
 cprintf("%@",xy(1,2));
 for (y=1;y<SCREEN_Y;y++) {
  cprintf("%@%c",xy(1,y),CHAR_BORDER);
  cprintf("%@%c",xy(SCREEN_X,y),CHAR_BORDER);
 }

 /* Worm. */
 cprintf("%1",COLOR_WORM);
 for (x=0;x<wormlength;x++) {
  cprintf("%@%c",xy(wormx[x]+2,wormy[x]+2),CHAR_WORM);
 }
 
 /* Apple. */
 cprintf("%1%@%c", COLOR_APPLE,xy(applex+2,appley+2),CHAR_APPLE);

 return;
}




