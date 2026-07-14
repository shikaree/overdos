#include <ansi.h>
#include <ansi-mod.h>

void clear_board(unsigned char board[10][10])
{
   int x,y;
   for(y=0;y<10;y++)
      for(x=0;x<10;x++)
	 board[y][x]='-';               /* (Not) DOS specific */
   board[4][4]='o';
   board[5][5]='o';
   board[4][5]='*';
   board[5][4]='*';
}

void output_board(unsigned char board[10][10])
{
   int x,y;
   printf(" ");
   for(x=0;x<8;x++)
      printf(" %c",x+97);
   for(y=1;y<9;y++){
      printf("\n\x0d%c",y+48);
      for(x=1;x<9;x++)
	 printf(" %c",board[y][x]);
   }
   printf("\n\x0d");
}

int capture(unsigned char board[10][10],int base_x,int base_y, int dx, int dy, char pl)
{
   int i,n,b,x,y;
   x=base_x+dx;y=base_y+dy;n=0;b=0;
   while(1){
      if(board[y][x]=='-')return 0;    
      if(board[y][x]==pl)break;
      x+=dx;y+=dy;
      n++;
      if(((x==8)||(x==1))||((y==8)||(y==1)))b++;
   }
   x=base_x+dx;y=base_y+dy;
   for(i=0;i<n;i++){
      board[y][x]=pl;
      y+=dy;x+=dx;
   }
   return n+b;
}


int attempt_move(unsigned char board[10][10],int move_x,int move_y, char pl)
{
   int sum=0;
   if((move_x>0)&&(move_x<9)&&(move_y>0)&&(move_y<9)&&(board[move_y][move_x]=='-'))
   {
      sum+=capture(board,move_x,move_y,-1,-1,pl);
      sum+=capture(board,move_x,move_y, 0,-1,pl);
      sum+=capture(board,move_x,move_y, 1,-1,pl);
      sum+=capture(board,move_x,move_y, 1, 0,pl);
      sum+=capture(board,move_x,move_y, 1, 1,pl);
      sum+=capture(board,move_x,move_y, 0, 1,pl);
      sum+=capture(board,move_x,move_y,-1, 1,pl);
      sum+=capture(board,move_x,move_y,-1, 0,pl);
      return sum;
   }
   return 0;
}

int ai_r(unsigned char board[10][10],int depth, char pl)
{
   unsigned char b_board[10][10];
   int x,y,xx,yy,gain,best;
   if(depth==0)return 0;
   best=-10000;
   /*memcpy(b_board[1][1],board[1][, 64); FAN! */


   for(y=0;y<10;y++)
      for(x=0;x<10;x++)
	 b_board[y][x]=board[y][x];

   for(y=1;y<9;y++)
      for(x=1;x<9;x++){
	 gain=attempt_move(board,x,y,pl);
	 if(gain){
	    if(((x==1)||(x==8))&&((y==1)||(y==8)))gain+=100; /* Corners!! */
	    else if((x==1)||(x==8)||(y==1)||(y==8))gain+=5; /* Good!! */
            board[y][x]=pl;
	    gain=gain-ai_r(board,depth-1,pl^69);
	    if(gain>best)best=gain;
	    /* memcpy(board[0],b_board[0], 64); Satan! */
	    for(yy=1;yy<9;yy++)
	       for(xx=1;xx<9;xx++)
		  board[yy][xx]=b_board[yy][xx];
	 }
      }
   if(best==-10000) best=-ai_r(board,depth-1,pl^69);
   return best;
}

int ai_base(unsigned char board[10][10],int depth, char pl)
{
   unsigned char b_board[10][10];
   int x,y,xx,yy,gain,best,bx=0,by=0;
   /* if(depth==0)return 0; Dum! */
   best=-10000;
   for(y=0;y<10;y++)
      for(x=0;x<10;x++)
	 b_board[y][x]=board[y][x];

   for(y=1;y<9;y++)
      for(x=1;x<9;x++){
	 gain=attempt_move(board,x,y,pl);
	 if(gain){
	    if(((x==1)||(x==8))&&((y==1)||(y==8)))gain+=20; /* Corners!! */
	    else if((x==1)||(x==8)||(y==1)||(y==8))gain+=5; /* Good!! */
	    board[y][x]=pl;
	    gain=gain-ai_r(board,depth-1,pl^69);
	    if(gain>best){
	       best=gain;bx=x;by=y;
	    }
	    for(yy=1;yy<9;yy++)
	       for(xx=1;xx<9;xx++)
                  board[yy][xx]=b_board[yy][xx];
	 }
      }
   if(best==-10000) return -1;
   return (bx<<5)|by;
}


int bordello_winlose(unsigned char board[10][10],int force)
{
   int c=0,p=0,x,y;
   for(y=1;y<9;y++)
      for(x=1;x<9;x++)
         if(board[y][x]=='*')c++;
         else if(board[y][x]=='o')p++;
   if(!p)return 2;
   if(!c)return 1;
   if(((p+c)==64)||force)
      if(p==c)
         return 3;
      else 
         if(p<c)
            return 2;
         else
            return 1;
   return 0;
}

int main_bordello(void)
{
   unsigned char board[10][10];              /* Globals rock! */
   unsigned char move_x,move_y;
   unsigned int level;
   char input[READLINE_SCREEN_WIDTH],arg[READLINE_SCREEN_WIDTH];
   char *tabcomp[]={0}; 
   int r,wl,ppass,force=0;
   clear_board(board);
   printf("\n\x0d");
   printf("Bordello - The Evil Othello!\n\n\x0d");
   level=-1;
   while(!((level>0)&&(level<61))){
      do {
         readline(input,"Play at level (1-60): ",tabcomp);
      } while(!readline_getarg(arg,input));
   level=atoi(arg);
   }
   printf("\n\n\x0dOk. Playing at level %d.\n\n\x0d",level);
   while(!(wl=bordello_winlose(board,force)))
   {
      output_board(board);
      ppass=0;
      printf("x: ");
      move_x=getchar();
      printf(" y: ");
      move_y=getchar();
      if((move_x=='q')||(move_y=='q'))break;
      if((move_x==' ')||attempt_move(board,move_x-96,move_y-48,'o')){
         if(move_x!=' '){
            board[move_y-48][move_x-96]='o';
            printf("\n\n\x0d");
            output_board(board);
         }
         if((move_x==' ')&&(ai_base(board,level,'o')!=-1)){
            printf("You are not allowed to pass.\n\n\x0d");
         }else{
            if(move_x==' ')ppass=1;
            printf(" AI is at it...\n\n\x0d");
            r=ai_base(board,level,'*');
            if(r!=-1){
               printf("AI: %c %c\n\n\x0d",(r>>5)+96,(r&15)+48);
               attempt_move(board,r>>5,r&15,'*');
               board[r&15][r>>5]='*';
            }else{
               printf("AI cannot move.\n\n\x0d");
               if(ppass)force=1;
            }
         }
      }
      else printf(" Illegal move!\n\n\x0d");
   }
   output_board(board);
   printf("\n\n\x0d");
   if(wl==3) printf("It's a tie! Unbelievable!");
   if(wl==1) printf("It seems like you've won. Shut up and try the next level.");
   if(wl==2) printf("You lose. Unfortunately this means that you are quite the idiot.");
   printf("\n\x0d");
   getchar();
   return 0;
}
