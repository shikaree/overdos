#define SCREEN_X 80
#define SCREEN_Y 23

#define COLOR_BORDER 	 ANSI_COL_CYAN
#define COLOR_WORM   	 ANSI_COL_BLUE
#define COLOR_WORM_HEAD  ANSI_COL_RED
#define COLOR_BACKGROUND ANSI_COL_BLACK
#define COLOR_APPLE	 ANSI_COL_RED
#define COLOR_SCORE	 ANSI_COL_GREEN
#define CHAR_BORDER  	'#'
#define CHAR_WORM    	'*'
#define CHAR_WORM_HEAD	'*'
#define CHAR_APPLE   	'@'

/* To be able to run on unix. */
#define stdout 0
#define stdin  1
#define EOF    -1

int keypress(void);
void redraw(char *wormx, char *wormy, char applex, char appley, int wormlength);



