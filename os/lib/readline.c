/* ---------------------------------------------------------------- */
/* - Readline library                                             - */
/* ---------------------------------------------------------------- */
#include <ansi.h>
#include <ansi-mod.h>
#include <os.h>

#define READLINE_CHAR_BACKSPACE         8
#define READLINE_CHAR_TAB               9

static int readline_tabcomplete(char *line, char *savestring, int *lastcomplete,
                                int start, char *tabcomp[]);
static void readline_redraw(char *line);


/* ---------------------------------------------------------------- */
/* - readline()							  - */
/* -								  - */
/* - Input: 							  - */
/* -           line - Where to save the input                     - */
/* -         prompt - Textprompt to put in front of every line    - */
/* -                  needs to be at least READLINE_SCREEN_WIDTH. - */
/* -        tabcomp - NULL-terminated array for name-completion.  - */
/* - Output:							  - */
/* -         Length of line.                                      - */
/* ---------------------------------------------------------------- */
int readline(char *line, char *prompt, char *tabcomp[])
{
 int pos, c=-1, minpos, lastcomplete=-1;
 char savestring[READLINE_SCREEN_WIDTH];
 
 /* No empty buffers. */
 if (!prompt || !line) {
  return 0;
 }

 savestring[0]=0;
 
 /* Copy the prompt. */
 pos=strlen(prompt);
 minpos=pos;
 if (pos>READLINE_SCREEN_WIDTH-1) {
  return 0;
 }
 strcpy(line,prompt);

 /* Start by clearing the line. */
 readline_redraw(line);

 while(c!=13) {
  switch(c) {
   case -1:
    break;
   case READLINE_CHAR_BACKSPACE:
    if (pos>minpos) {
     printf("%c %c",8,8);
     pos--;
     lastcomplete=-1;
    }
    break;
   case READLINE_CHAR_TAB:
    line[pos]=0;
    pos=readline_tabcomplete(line,savestring,&lastcomplete,minpos,tabcomp);
    break;
   default:
    if (pos<READLINE_SCREEN_WIDTH-1) {
     printf("%c",c);
     line[pos++]=c;
     lastcomplete=-1;
    }
  }
  c=getcharne();
 }

 /* New line is required. */
 printf("\n\x0d");
 
 /* Terminate line. */
 line[pos]=0;

 /* Remove prompt. */
 strcpy(line,line+minpos);

 return pos;
}




/* ---------------------------------------------------------------- */
/* - readline_getarg()						  - */
/* -								  - */
/* - Input: 							  - */
/* -       argument - Buffer to copy argument to.                 - */
/* -           line - Buffer containging arguments.               - */
/* -                  The argument will be REMOVED from line!     - */
/* - Output:							  - */
/* -         Length of argument or 0 if no more arguments.        - */
/* ---------------------------------------------------------------- */
int readline_getarg(char *argument, char *line) {
 int start, end;

 if (!argument && !line) {
  return 0;
 }

 /* Loop away spaces. */
 start=0;
 while(line[start]==32 && line[start]!=0) start++;

 /* Search end. */
 end=start;
 while(line[end]!=32 && line[end]!=0) end++;

 /* Equal? */
 if (end==start) {
  return 0;
 }

 /* Copy argument. */
 strcpy(argument,line+start);
 argument[end-start]=0;

 /* Remove argument. */
 strcpy(line,line+end);

 return strlen(argument);
}




static int readline_tabcomplete(char *line, char *savestring, int *lastcomplete,
                                int start, char *tabcomp[])
{
 char *ptr;
 int len, i,index=0;
 int arraylen;

 ptr=line+start;
 len=strlen(ptr);

 /* Find length of array. */
 arraylen=0;
 while(tabcomp[arraylen]) {
  arraylen++;
 }

 /* Have we pressed TAB just before? */
 if (*lastcomplete!=-1) {
  /* Restore original search pattern. */
  strcpy(ptr,savestring);
  len=strlen(ptr);
  index=(*lastcomplete+1) % arraylen;
 }

 /* Search thru the completion list. */
 if (len) {
  for (i=0;i<arraylen;i++) {
   if (!strncmp(ptr,tabcomp[index],len)) {
    if (*lastcomplete==-1) {
     strcpy(savestring,ptr);
    }
    *lastcomplete=index;
    strcpy(ptr,tabcomp[index]);
    len=strlen(ptr);
    break;
   }
   index=(index+1) % arraylen;
  }
 }

 /* Redraw. */
 if (*lastcomplete!=-1) {
   readline_redraw(line);
 }

 return start+len;
}




static void readline_redraw(char *line)
{
 char empty[READLINE_SCREEN_WIDTH+1];
 int i, len;
 
 for(i=0;i<READLINE_SCREEN_WIDTH;i++) {
  empty[i]=32;
  } 
 empty[READLINE_SCREEN_WIDTH]=0;
  
 len=strlen(line);
 
 printf("\x1b[%iD%s",READLINE_SCREEN_WIDTH,line);
 printf("%s",empty+len);
 printf("\x1b[%iD%s",READLINE_SCREEN_WIDTH,line);

}

