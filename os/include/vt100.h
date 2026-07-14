/* ---------------------------------------------------------------- */
/* - VT100 for ÖverDOS (this is the only file)	 	          - */
/* ---------------------------------------------------------------- */

#define CURSES_ESC 27

/* vt100_init()
   Sets 80 column mode. */
#define vt100_init()  	    printf("%c[?3l",CURSES_ESC); 
   
/* gotoxy()
   Moves the cursor to a specific location on the screen.
   Values for x and y start at 0.                         */
#define gotoxy(x,y)         printf("%c[%i;%iH",CURSES_ESC,y,x)
   
/* cls()
   Clears the screen and homes the cursor. */
#define cls()               printf("%c[2J",CURSES_ESC)


/* fgcolor()
   Sets one of the colors ANSI_COL_* as foreground color. */
#define fgcolor(c)	    printf("%c[%im",CURSES_ESC,c+30)
   
/* bgcolor()
   Sets one of the colors ANSI_COL_* as background color. */
#define bgcolor(c)	    printf("%c[%im",CURSES_ESC,c+40)
     
/* attribute()
   Sets one of the attributes ANSI_ATT_* as attribute. */
#define attribute(a)	    printf("%c[%im",CURSES_ESC,a)
   
#define xy(x,y)		    (((y)<<8)|(x))
   
#define ANSI_COL_BLACK		0
#define ANSI_COL_RED		1
#define ANSI_COL_GREEN		2
#define ANSI_COL_YELLOW		3
#define ANSI_COL_BLUE		4
#define ANSI_COL_MAGENTA	5
#define ANSI_COL_CYAN		6
#define ANSI_COL_WHITE		7
   
#define ANSI_ATT_NORMAL		0
#define ANSI_ATT_BOLD		1
#define ANSI_ATT_UNDERLINE	2
#define ANSI_ATT_BLINK		3
#define ANSI_ATT_REVERSE	4
#define ANSI_ATT_INVISIBLE	5


     
     
