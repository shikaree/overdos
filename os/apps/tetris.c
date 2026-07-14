/* Tetris... */

#include <vt100.h>
#include <ansi-mod.h>
#include <ansi.h>
#include <tetris.h>
#include <os.h>

#define CF(x) bcol[(int)field[y][x]],bchar[(int)field[y][x]],bchar[(int)field[y][x]]

/* collision - Returns 1 if block b with rotation r does not fit
   at position x,y in the playing field. */
int tetris_collision(char **field,int b, int r, int x, int y)
{
   int i,j;
   for(j=0;j<4;j++)
      for(i=0;i<4;i++)
         if((block[b][r][j][i])&&(field[y+j][x+i]))return 1;
   return 0;
}

/* init - clear the playing field and create borders... */
void tetris_init(char **field)
{
   int x,y;
   cprintf("%1%2%3%0",7,0,1);
   for(y=0;y<21;y++)
      for(x=2;x<12;x++)
         field[y][x]=COL_BG;
   for(y=0;y<21;y++){
      field[y][0]=COL_BORDER; field[y][1]=COL_BORDER;
      field[y][12]=COL_BORDER; field[y][13]=COL_BORDER;
   }
   for(x=0;x<14;x++){
      field[21][x]=COL_BORDER; field[22][x]=COL_BORDER;
      field[0][x]=COL_BORDER;
   }
}

void tetris_xortofield(char **field,int b, int r, int x, int y)
{
   int i,j;
   for(j=0;j<4;j++)
      for(i=0;i<4;i++)
         field[y+j][x+i]^=block[b][r][j][i];
}

void tetris_render_nr(int i,int y)
{
   char a[12];
   int x;
   x=itoa(a,i);
   cprintf("%@%1%2%d",xy(70-x,4+y*3),7,0,i);
}

void tetris_render_all(char **field)
{
   int y;   
   cprintf("%2%3%0",0,1);
   for(y=0;y<22;y++)
      cprintf("%@%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c",xy(30,y+2),
         CF(1),CF(2),CF(3),CF(4),CF(5),CF(6),CF(7),CF(8),CF(9),CF(10),CF(11),CF(12));
         
   cprintf("%@%2%1Score:%@Lines:%@Level:%@Next:%1",xy(56,4),0,6,xy(56,7),xy(56,10),xy(56,13),7);
}

/* render_block - Draws block b,r at x,y. If dx|dy|dr is non-zero
   this function also removes the last block. */
void tetris_render_block(int b,int r,int x,int y,int dx,int dy,int dr)
{
   char buffer[1024];
   char news[128];
   int i,j,te;
   char blackmap[6][6],newmap[6][6],chc,chb;
   te=80;
   chc=bchar[b+1];chb=bchar[COL_BG];
   for(j=0;j<6;j++)for(i=0;i<6;i++){blackmap[j][i]=0;newmap[j][i]=0;}
   for(j=0;j<4;j++)
      for(i=0;i<4;i++){
	 blackmap[j+1-dy][i+1-dx]=block[b][(r+4-dr)&3][j][i];
         newmap[j+1][i+1]=block[b][r][j][i];
      }
   for(j=0;j<4;j++)
      for(i=0;i<4;i++){
         blackmap[j+1][i+1]&=(block[b][r][j][i]^(b+1));
         newmap[j+1-dy][i+1-dx]&=(block[b][(r+4-dr)&3][j][i]^(b+1));
      }
   sprintf(buffer,"%2",bcol[COL_BG]);
   for(j=0;j<6;j++)
      for(i=0;i<6;i++)
         if(blackmap[j][i]){sprintf(news,"%@%c%c",xy(2*(x+i-3)+32,(y+j+1)),chb,chb);strcat(buffer,news);}

   sprintf(news,"%2",bcol[b+1]);strcat(buffer,news);
   for(j=0;j<6;j++)
      for(i=0;i<6;i++)
	      if(newmap[j][i]){sprintf(news,"%@%c%c",xy(2*(x+i-3)+32,(y+j+1)),chc,chc);strcat(buffer,news);}
   printf("%s",buffer);
}

/* render_new - Draws a block to the screen. */
void tetris_render_new(int b,int r,int x,int y)
{
   int i,j;
   cprintf("%2",bcol[b+1]);
   for(j=0;j<4;j++)
      for(i=0;i<4;i++)
         if(block[b][r][j][i])
            cprintf("%@%c%c",xy(2*(x+i)+28,(y+j)+2),bchar[b+1],bchar[b+1]);
}


/* Remove lines starting att y-position y */
/* REturns the number of lines deleted (0-4) */
int tetris_demolish(char **field,int y)
{
   int b,z,x,c,miny=20,redraw=0;
   if(y>20)y=20;
   for((y>4)?(z=y-4):(z=1);z<=y;z++){
      for(x=2,c=0;x<12;x++)
         if(field[z][x])
            c++;
      if(c==10){
         redraw++;
         for(b=z;b>1;b--)
            for(x=2;x<12;x++){
               if(field[b][x])
                  if(b<miny)
                     miny=b;
               field[b][x]=field[b-1][x];
            }
               
         for(x=2;x<12;x++)
            field[1][x]=0;
      }
   }
   if(redraw)
      for(;y>miny-2;y--)
         cprintf("%@%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c%2%c%c",xy(32,y+2),
            CF(2),CF(3),CF(4),CF(5),CF(6),CF(7),CF(8),CF(9),CF(10),CF(11));
   return redraw;
}
      
      


void tetris_get_input(int *dx,int *drop,int *dr,int *q)
{
   int inp;
   inp=getch();
   ((char)inp=='j')?(*dx=-1):(((char)inp=='l')?(*dx=1):(*dx=0));
   ((char)inp=='k')?(*dr=1):(*dr=0);
   ((char)inp==' ')?(*drop=1):(*drop=0);
   ((char)inp=='q')?(*q=0):(*q=7);
}




int main_tetris(void)
{
   char *field[23];
   int b,r,x,y,alive,speed,s,dx,dr,drop,q;
   unsigned int timez,interv;
   int level=1,lines=0,score=0,next;
   int focus;
   char fielddata[14*23];
   
   for (s=0;s<23;s++) {
    field[s]=&fielddata[s*14];
   }
   
   tetris_init(field);
   register_focusswitch(&focus);
   interv=get_timer_interval();
   
   next=rand()%7;
   b=rand()%7;
   r=0;x=5;y=1;speed=1000000;alive=7;
   tetris_render_all(field);
   tetris_render_nr(score,0);
   tetris_render_nr(lines,1);
   tetris_render_nr(level,2);
   tetris_render_new(next,0,14,13);
   tetris_render_new(b,r,x,y);
   while(alive){
      timez=get_timer();
      while((get_timer()-timez)*interv<speed){ 
         if(focus==1){focus=0;tetris_render_all(field);tetris_render_new(b,r,x,y);}
         tetris_get_input(&dx,&drop,&dr,&q);
         if(!q){alive=0;break;}
         if(drop){
            score+=((20-y)*level);
            break;
         }
         if((dx||dr)&&(!tetris_collision(field,b,(r+dr)&3,x+dx,y))){ 
            r=(r+dr)&3;x+=dx;
            tetris_render_block(b,r,x,y,dx,0,dr);
         }
      }     
      if(drop)
         while(!tetris_collision(field,b,r,x,y+1))
            tetris_render_block(b,r,x,++y,0,1,0);
      if(tetris_collision(field,b,r,x,y+1)){
         tetris_xortofield(field,b,r,x,y);
         if((s=tetris_demolish(field,y+3))){
            lines+=s;
            score+=((100<<(s-1))*level);
            tetris_render_nr(lines,1);
            if((level-10)&&((lines/10)>=level)){
               tetris_render_nr(++level,2);
               speed-=100000;
            }
         }
         if(s||drop)
            tetris_render_nr(score,0);
         
         b=next; r=0; x=5; y=1; next=rand()%7;
         cprintf("%@%2        %@        %@        %@        ",xy(56,15),0,xy(56,16),xy(56,17),xy(56,18));
         tetris_render_new(next,0,14,13);
    
 
         tetris_render_new(b,r,x,y);
         if(tetris_collision(field,b,r,x,y))
            alive=0;
      } else {
         y++;
         tetris_render_block(b,r,x,y,0,1,0);
      }
   }
   cprintf("%@%1%2---- GAME  OVER ----",xy(32,12),7,0);
   while(getch()==ERR);
   return 0;
} 
