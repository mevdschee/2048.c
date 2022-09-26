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
uint32_t score = 0;
uint32_t moveNumber = 0;
uint8_t scheme = 0;

enum gameStates
{
	inGame,
	gameOver,
	wantQuit,
	restartGame
};

enum gameStates state;

void getColors(uint8_t value, uint8_t *foreground, uint8_t *background)
{
	uint8_t original[] = {8, 255, 1, 255, 2, 255, 3, 255, 4, 255, 5, 255, 6, 255, 7, 255, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 255, 0, 255, 0};
	uint8_t blackwhite[] = {232, 255, 234, 255, 236, 255, 238, 255, 240, 255, 242, 255, 244, 255, 246, 0, 248, 0, 249, 0, 250, 0, 251, 0, 252, 0, 253, 0, 254, 0, 255, 0};
	uint8_t bluered[] = {235, 255, 63, 255, 57, 255, 93, 255, 129, 255, 165, 255, 201, 255, 200, 255, 199, 255, 198, 255, 197, 255, 196, 255, 196, 255, 196, 255, 196, 255, 196, 255};
	uint8_t *schemes[] = {original, blackwhite, bluered};
	*foreground = *(schemes[scheme] + (1 + value * 2) % sizeof(original));
	*background = *(schemes[scheme] + (0 + value * 2) % sizeof(original));
}

uint8_t getNumberLength(uint32_t number)
{
	uint8_t count = 0;
	do
	{
		number /= 10;
		count += 1;
	} while (number);
	return count;
}

void drawBoard(uint8_t board[SIZE][SIZE])
{
	uint8_t x, y, fg, bg;
	printf("\033[H");
	printf("Move: %d | Score: %d pts\n\n",moveNumber, score);
	for (y = 0; y < SIZE; y++)
	{
		for (x = 0; x < SIZE; x++)
		{
			getColors(board[x][y], &fg, &bg);
			printf("\033[38;5;%d;48;5;%dm", fg, bg); // set color
			printf("       ");
			printf("\033[m"); // reset
		}
		printf("\n");
		for (x = 0; x < SIZE; x++)
		{
			getColors(board[x][y], &fg, &bg);
			printf("\033[38;5;%d;48;5;%dm", fg, bg); // set color
			if (board[x][y] != 0)
			{
				uint32_t number = 1 << board[x][y];
				uint8_t t = 7 - getNumberLength(number);
				printf("%*s%u%*s", t - t / 2, "", number, t / 2, "");
			}
			else
			{
				printf("   ·   ");
			}
			printf("\033[m"); // reset
		}
		printf("\n");
		for (x = 0; x < SIZE; x++)
		{
			getColors(board[x][y], &fg, &bg);
			printf("\033[38;5;%d;48;5;%dm", fg, bg); // set color
			printf("       ");
			printf("\033[m"); // reset
		}
		printf("\n");
	}
	switch(state)
	{
		case inGame:
		{
			printf("____________________________\n");
			printf("|               |          |\n");
			printf("|←,↑,→,↓ - move | / - undo |\n");
			printf("|r - restart    | q - exit |\n");
			printf("________________|___________\n");
			break;
		}
		case gameOver:
		{
			printf("____________________________\n");
			printf("|                          |\n");
			printf("|        GAME OVER         |\n");
			printf("| / - undo move | q - exit |\n");
			printf("____________________________\n");
			break;
		}
		case wantQuit:
		{
			printf("____________________________\n");
			printf("|                          |\n");
			printf("|       QUIT? (y/n)        |\n");
			printf("|                          |\n");
			printf("____________________________\n");
			break;
		}
		case restartGame:
		{
			printf("____________________________\n");
			printf("|                          |\n");
			printf("|      RESTART? (y/n)      |\n");
			printf("|                          |\n");
			printf("____________________________\n");
			break;
		}
	}
}

uint8_t findTarget(uint8_t array[SIZE], uint8_t x, uint8_t stop)
{
	uint8_t t;
	// if the position is already on the first, don't evaluate
	if (x == 0)
	{
		return x;
	}
	for (t = x - 1;; t--)
	{
		if (array[t] != 0)
		{
			if (array[t] != array[x])
			{
				// merge is not possible, take next position
				return t + 1;
			}
			return t;
		}
		else
		{
			// we should not slide further, return this one
			if (t == stop)
			{
				return t;
			}
		}
	}
	// we did not find a target
	return x;
}

bool slideArray(uint8_t array[SIZE])
{
	bool success = false;
	uint8_t x, t, stop = 0;

	for (x = 0; x < SIZE; x++)
	{
		if (array[x] != 0)
		{
			t = findTarget(array, x, stop);
			// if target is not original position, then move or merge
			if (t != x)
			{
				// if target is zero, this is a move
				if (array[t] == 0)
				{
					array[t] = array[x];
				}
				else if (array[t] == array[x])
				{
					// merge (increase power of two)
					array[t]++;
					// increase score
					score += (uint32_t)1 << array[t];
					// set stop to avoid double merge
					stop = t + 1;
				}
				array[x] = 0;
				success = true;
			}
		}
	}
	return success;
}

void rotateBoard(uint8_t board[SIZE][SIZE])
{
	uint8_t i, j, n = SIZE;
	uint8_t tmp;
	for (i = 0; i < n / 2; i++)
	{
		for (j = i; j < n - i - 1; j++)
		{
			tmp = board[i][j];
			board[i][j] = board[j][n - i - 1];
			board[j][n - i - 1] = board[n - i - 1][n - j - 1];
			board[n - i - 1][n - j - 1] = board[n - j - 1][i];
			board[n - j - 1][i] = tmp;
		}
	}
}

bool moveUp(uint8_t board[SIZE][SIZE])
{
	bool success = false;
	uint8_t x;
	for (x = 0; x < SIZE; x++)
	{
		success |= slideArray(board[x]);
	}
	return success;
}

bool moveLeft(uint8_t board[SIZE][SIZE])
{
	bool success;
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveDown(uint8_t board[SIZE][SIZE])
{
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveRight(uint8_t board[SIZE][SIZE])
{
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board);
	rotateBoard(board);
	return success;
}

void copyBoard(uint8_t boardToCopy[SIZE][SIZE], uint8_t boardToGetCopy[SIZE][SIZE])
{
	uint8_t x, y;
	for (x = 0; x < SIZE; x++)
	{
		for (y = 0; y < SIZE; y++)
		{
			boardToGetCopy[x][y] = boardToCopy[x][y];
		}
	}
}

bool boardsAreSimilar(uint8_t boardOne[SIZE][SIZE], uint8_t boardTwo[SIZE][SIZE])
{
	bool similar = true;
	uint8_t x, y;
	for (x = 0; x < SIZE; x++)
	{
		for (y = 0; y < SIZE; y++)
		{
			if(boardOne[x][y] != boardTwo[x][y])
				return false;
		}
	}
	return similar;
}

void undoMove(uint8_t board[SIZE][SIZE], uint8_t previousBoard[SIZE][SIZE])
{
	copyBoard(previousBoard, board);
	if(moveNumber != 0)
	{
		moveNumber--;
	}
}

bool findPairDown(uint8_t board[SIZE][SIZE])
{
	bool success = false;
	uint8_t x, y;
	for (x = 0; x < SIZE; x++)
	{
		for (y = 0; y < SIZE - 1; y++)
		{
			if (board[x][y] == board[x][y + 1])
				return true;
		}
	}
	return success;
}

uint8_t countEmpty(uint8_t board[SIZE][SIZE])
{
	uint8_t x, y;
	uint8_t count = 0;
	for (x = 0; x < SIZE; x++)
	{
		for (y = 0; y < SIZE; y++)
		{
			if (board[x][y] == 0)
			{
				count++;
			}
		}
	}
	return count;
}

bool gameEnded(uint8_t board[SIZE][SIZE])
{
	bool ended = true;
	if (countEmpty(board) > 0)
		return false;
	if (findPairDown(board))
		return false;
	rotateBoard(board);
	if (findPairDown(board))
		ended = false;
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	return ended;
}

void addRandom(uint8_t board[SIZE][SIZE])
{
	static bool initialized = false;
	uint8_t x, y;
	uint8_t r, len = 0;
	uint8_t n, list[SIZE * SIZE][2];

	if (!initialized)
	{
		srand(time(NULL));
		initialized = true;
	}

	for (x = 0; x < SIZE; x++)
	{
		for (y = 0; y < SIZE; y++)
		{
			if (board[x][y] == 0)
			{
				list[len][0] = x;
				list[len][1] = y;
				len++;
			}
		}
	}

	if (len > 0)
	{
		r = rand() % len;
		x = list[r][0];
		y = list[r][1];
		n = (rand() % 10) / 9 + 1;
		board[x][y] = n;
	}
}

void initBoard(uint8_t board[SIZE][SIZE])
{
	uint8_t x, y;
	for (x = 0; x < SIZE; x++)
	{
		for (y = 0; y < SIZE; y++)
		{
			board[x][y] = 0;
		}
	}
	addRandom(board);
	addRandom(board);
	drawBoard(board);
	score = 0;
	moveNumber = 0;
}

void setBufferedInput(bool enable)
{
	static bool enabled = true;
	static struct termios old;
	struct termios new;

	if (enable && !enabled)
	{
		// restore the former settings
		tcsetattr(STDIN_FILENO, TCSANOW, &old);
		// set the new state
		enabled = true;
	}
	else if (!enable && enabled)
	{
		// get the terminal settings for standard input
		tcgetattr(STDIN_FILENO, &new);
		// we want to keep the old setting to restore them at the end
		old = new;
		// disable canonical mode (buffered i/o) and local echo
		new.c_lflag &= (~ICANON & ~ECHO);
		// set the new settings immediately
		tcsetattr(STDIN_FILENO, TCSANOW, &new);
		// set the new state
		enabled = false;
	}
}

int test()
{
	uint8_t array[SIZE];
	// these are exponents with base 2 (1=2 2=4 3=8)
	uint8_t data[] = {
		0, 0, 0, 1, 1, 0, 0, 0,
		0, 0, 1, 1, 2, 0, 0, 0,
		0, 1, 0, 1, 2, 0, 0, 0,
		1, 0, 0, 1, 2, 0, 0, 0,
		1, 0, 1, 0, 2, 0, 0, 0,
		1, 1, 1, 0, 2, 1, 0, 0,
		1, 0, 1, 1, 2, 1, 0, 0,
		1, 1, 0, 1, 2, 1, 0, 0,
		1, 1, 1, 1, 2, 2, 0, 0,
		2, 2, 1, 1, 3, 2, 0, 0,
		1, 1, 2, 2, 2, 3, 0, 0,
		3, 0, 1, 1, 3, 2, 0, 0,
		2, 0, 1, 1, 2, 2, 0, 0};
	uint8_t *in, *out;
	uint8_t t, tests;
	uint8_t i;
	bool success = true;

	tests = (sizeof(data) / sizeof(data[0])) / (2 * SIZE);
	for (t = 0; t < tests; t++)
	{
		in = data + t * 2 * SIZE;
		out = in + SIZE;
		for (i = 0; i < SIZE; i++)
		{
			array[i] = in[i];
		}
		slideArray(array);
		for (i = 0; i < SIZE; i++)
		{
			if (array[i] != out[i])
			{
				success = false;
			}
		}
		if (success == false)
		{
			for (i = 0; i < SIZE; i++)
			{
				printf("%d ", in[i]);
			}
			printf("=> ");
			for (i = 0; i < SIZE; i++)
			{
				printf("%d ", array[i]);
			}
			printf("expected ");
			for (i = 0; i < SIZE; i++)
			{
				printf("%d ", in[i]);
			}
			printf("=> ");
			for (i = 0; i < SIZE; i++)
			{
				printf("%d ", out[i]);
			}
			printf("\n");
			break;
		}
	}
	if (success)
	{
		printf("All %u tests executed successfully\n", tests);
	}
	return !success;
}

void signal_callback_handler(int signum)
{
	printf("         TERMINATED         \n");
	setBufferedInput(true);
	printf("\033[?25h\033[m");
	exit(signum);
}

int main(int argc, char *argv[])
{
	uint8_t board[SIZE][SIZE];
	uint8_t previousBoard[SIZE][SIZE];
	// uint8_t*** previousBoardList = (uint8_t***)malloc(SIZE * SIZE * sizeof(uint8_t***));
	uint8_t bufferBoard[SIZE][SIZE];

	char c;
	bool success;

	if (argc == 2 && strcmp(argv[1], "test") == 0)
	{
		return test();
	}
	if (argc == 2 && strcmp(argv[1], "blackwhite") == 0)
	{
		scheme = 1;
	}
	if (argc == 2 && strcmp(argv[1], "bluered") == 0)
	{
		scheme = 2;
	}

	printf("\033[?25l\033[2J");

	// register signal handler for when ctrl-c is pressed
	signal(SIGINT, signal_callback_handler);

	state = inGame;
	initBoard(board);

	uint8_t i;

	copyBoard(board, previousBoard);
	copyBoard(board, bufferBoard);

	setBufferedInput(false);
	while (true)
	{
		c = getchar();
		if (c == -1)
		{
			puts("\nError! Cannot read keyboard input!");
			break;
		}
		switch (c)
		{
		case 97:  // 'a' key
		case 104: // 'h' key
		case 68:  // left arrow
			copyBoard(board, bufferBoard);
			success = moveLeft(board);
			if(!boardsAreSimilar(board, bufferBoard))
			{
				copyBoard(bufferBoard, previousBoard);
				moveNumber++;
			}
			break;
		case 100: // 'd' key
		case 108: // 'l' key
		case 67:  // right arrow
			copyBoard(board, bufferBoard);
			success = moveRight(board);
			if(!boardsAreSimilar(board, bufferBoard))
			{
				copyBoard(bufferBoard, previousBoard);
				moveNumber++;
			}
			break;
		case 119: // 'w' key
		case 107: // 'k' key
		case 65:  // up arrow
			copyBoard(board, bufferBoard);
			success = moveUp(board);
			if(!boardsAreSimilar(board, bufferBoard))
			{
				copyBoard(bufferBoard, previousBoard);
				moveNumber++;
			}
			break;
		case 115: // 's' key
		case 106: // 'j' key
		case 66:  // down arrow
			copyBoard(board, bufferBoard);
			success = moveDown(board);
			if(!boardsAreSimilar(board, bufferBoard))
			{
				copyBoard(bufferBoard, previousBoard);
				moveNumber++;
			}
			break;
		case 47: // "/"key
			undoMove(board, previousBoard);
			drawBoard(board);
		default:
			success = false;
		}
		if (success)
		{
			drawBoard(board);
			usleep(150000);
			addRandom(board);
			drawBoard(board);
			if (gameEnded(board))
			{
				state = gameOver;
				drawBoard(board);
				while (true)
				{
					c = getchar();
					if (c == 'q')
					{
						break;
					}
					else if(c == '/')
					{
						state = inGame;
						undoMove(board, previousBoard);
						drawBoard(board);
						break;
					}
				}
			}
		}
		if (c == 'q')
		{
			state = wantQuit;
			drawBoard(board);
			c = getchar();
			if (c == 'y')
			{
				break;
			}
			state = inGame;
			drawBoard(board);
		}
		if (c == 'r')
		{
			state = restartGame;
			drawBoard(board);
			c = getchar();
			if (c == 'y')
			{
				state = inGame;
				initBoard(board);
			}
			state = inGame;
			drawBoard(board);
		}
	}
	setBufferedInput(true);

	printf("\033[?25h\033[m");

	return EXIT_SUCCESS;
}
