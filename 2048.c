/*
 ============================================================================
 Name        : 2048.c
 Author      : Maurits van der Schee
 Description : Console version of the game "2048" for GNU/Linux
 ============================================================================
 */

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

#define SIZE 4

#define COLORSET_ORIGINAL {8,255,1,255,2,255,3,255,4,255,5,255,6,255,7,255,9,0,10,0,11,0,12,0,13,0,14,0,255,0,255,0}
#define COLORSET_BLACKWHITE {232,255,234,255,236,255,238,255,240,255,242,255,244,255,246,0,248,0,249,0,250,0,251,0,252,0,253,0,254,0,255,0}
#define COLORSET_BLUERED {235,255,63,255,57,255,93,255,129,255,165,255,201,255,200,255,199,255,198,255,197,255,196,255,196,255,196,255,196,255,196,255}

#define CELL(i) *(getcell(board, dir, row, i))
typedef uint16_t t_cell;
typedef t_cell t_row[SIZE];
typedef t_row t_board[SIZE];

uint32_t score=0;

void getColor(uint16_t value, char *color, size_t length) {
	uint8_t colorset[] = COLORSET_ORIGINAL;
	uint8_t *scheme = colorset;
	uint8_t *background = scheme+0;
	uint8_t *foreground = scheme+1;
	if (value > 0) while (value >>= 1) {
		if (background+2<scheme+sizeof(colorset)) {
			background+=2;
			foreground+=2;
		}
	}
	snprintf(color,length,"\033[38;5;%d;48;5;%dm",*foreground,*background);
}

void drawBoard(uint16_t board[SIZE][SIZE]) {
	int8_t x,y;
	char color[40], reset[] = "\033[m";
	printf("\033[H");

	printf("2048.c %17d pts\n\n",score);

	for (y=0;y<SIZE;y++) {
		for (x=0;x<SIZE;x++) {
			getColor(board[x][y],color,40);
			printf("%s",color);
			printf("       ");
			printf("%s",reset);
		}
		printf("\n");
		for (x=0;x<SIZE;x++) {
			getColor(board[x][y],color,40);
			printf("%s",color);
			if (board[x][y]!=0) {
				char s[8];
				snprintf(s,8,"%u",board[x][y]);
				int8_t t = 7-strlen(s);
				printf("%*s%s%*s",t-t/2,"",s,t/2,"");
			} else {
				printf("   ·   ");
			}
			printf("%s",reset);
		}
		printf("\n");
		for (x=0;x<SIZE;x++) {
			getColor(board[x][y],color,40);
			printf("%s",color);
			printf("       ");
			printf("%s",reset);
		}
		printf("\n");
	}
	printf("\n");
	printf("        ←,↑,→,↓ or q        \n");
	printf("\033[A");
}

t_cell* getcell (t_board board, char dir, int numrow, int index) {
	dir = dir%4;
	if (dir==1 || dir==2) index = SIZE-1-index;
	if (dir >= 2) numrow = SIZE-1-numrow;
	if (dir%2 == 1) {
		int tmp = numrow;
		numrow = index;
		index = tmp;
	}
	if (numrow<0 || numrow>SIZE || index < 0 || index>SIZE) return NULL;
	return &(board[numrow][index]);
}

char slide (t_board board, char dir) {
	char success = 0;
	int row;
	for (row=0; row < SIZE; row++) {
		int cur=0,next=0,write=0;
		char rowsuccess = 0;
		while (cur < SIZE && CELL(cur) == 0) {
			cur++; next++;
		}
		if (cur>0 && cur<SIZE) rowsuccess=1;
		while (cur < SIZE) {
			do {next++;} while ( next < SIZE && CELL(next) == 0);
			if (next < SIZE && next != cur+1) rowsuccess=1;
			if (next < SIZE && CELL(cur) == CELL(next)) {
				rowsuccess = 1;
				CELL(cur) *= 2;
				score += CELL(cur);
				next++;
			}
			CELL(write) = CELL(cur);
			write++;
			cur = next;
		}
		if (!rowsuccess) continue;
		while (write<SIZE) {
			CELL(write) = 0;
			write++;
		}
		success |= rowsuccess;
	}
	return success;
}


bool canMoveInDir (t_board board, char dir) {
	int row,i;
	for (row=0;row<SIZE;row++) {
		for (i=0;i<SIZE-1;i++) {
			t_cell a=CELL(i), b=CELL(i+1);
			if (a==0 || b == 0 || a == b) return true;
		}
	}
	return false;
}

int16_t countEmpty(uint16_t board[SIZE][SIZE]) {
	int8_t x,y;
	int16_t count=0;
	for (x=0;x<SIZE;x++) {
		for (y=0;y<SIZE;y++) {
			if (board[x][y]==0) {
				count++;
			}
		}
	}
	return count;
}

bool gameEnded(uint16_t board[SIZE][SIZE]) {
	return ( !canMoveInDir(board,0) && !canMoveInDir(board,1));
}

void addRandom(uint16_t board[SIZE][SIZE]) {
	static bool initialized = false;
	int8_t x,y;
	int16_t r,len=0;
	uint16_t n,list[SIZE*SIZE][2];

	if (!initialized) {
		srand(time(NULL));
		initialized = true;
	}

	for (x=0;x<SIZE;x++) {
		for (y=0;y<SIZE;y++) {
			if (board[x][y]==0) {
				list[len][0]=x;
				list[len][1]=y;
				len++;
			}
		}
	}

	if (len>0) {
		r = rand()%len;
		x = list[r][0];
		y = list[r][1];
		n = ((rand()%10)/9+1)*2;
		board[x][y]=n;
	}
}

void setBufferedInput(bool enable) {
	static bool enabled = true;
	static struct termios old;
	struct termios new;

	if (enable && !enabled) {
		// restore the former settings
		tcsetattr(STDIN_FILENO,TCSANOW,&old);
		// set the new state
		enabled = true;
	} else if (!enable && enabled) {
		// get the terminal settings for standard input
		tcgetattr(STDIN_FILENO,&new);
		// we want to keep the old setting to restore them at the end
		old = new;
		// disable canonical mode (buffered i/o) and local echo
		new.c_lflag &=(~ICANON & ~ECHO);
		// set the new settings immediately
		tcsetattr(STDIN_FILENO,TCSANOW,&new);
		// set the new state
		enabled = false;
	}
}

void signal_callback_handler(int signum) {
	printf("         TERMINATED         \n");
	setBufferedInput(true);
	printf("\033[?25h");
	exit(signum);
}

int main(int argc, char *argv[]) {
	t_board board;
	char c;
	bool success;

	printf("\033[?25l\033[2J\033[H");

	// register signal handler for when ctrl-c is pressed
	signal(SIGINT, signal_callback_handler);

	memset(board,0,sizeof(board));
	addRandom(board);
	addRandom(board);
	drawBoard(board);
	setBufferedInput(false);
	while (true) {
		c=getchar();
		switch(c) {
			case 68:	// left arrow
				success = slide(board,3);  break; 
			case 67:	// right arrow
				success = slide(board,1); break; 
			case 65:	// up arrow
				success = slide(board,0);    break; 
			case 66:	// down arrow
				success = slide(board,2);  break; 
			default: success = false;
		}

		if (success) {
			drawBoard(board);
			usleep(150000);
			addRandom(board);
			drawBoard(board);
			if (gameEnded(board)) {
				printf("         GAME OVER          \n");
				break;
			}
		}
		if (c=='q') {
			printf("            QUIT            \n");
			break;
		}
	}
	setBufferedInput(true);

	printf("\033[?25h");

	return EXIT_SUCCESS;
}
