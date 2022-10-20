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

/**
 * given a tile `value` and a color-`scheme` index,
 * returns `foreground` and `background` colors as a "2-tuple".
*/
__attribute__((pure)) uint8_t *getColors(uint8_t value, const uint8_t scheme)
{
	static uint8_t out[2] = {0, 0};
	const uint8_t original[] = {8, 255, 1, 255, 2, 255, 3, 255, 4, 255, 5, 255, 6, 255, 7, 255, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 255, 0, 255, 0};
	const uint8_t blackwhite[] = {232, 255, 234, 255, 236, 255, 238, 255, 240, 255, 242, 255, 244, 255, 246, 0, 248, 0, 249, 0, 250, 0, 251, 0, 252, 0, 253, 0, 254, 0, 255, 0};
	const uint8_t bluered[] = {235, 255, 63, 255, 57, 255, 93, 255, 129, 255, 165, 255, 201, 255, 200, 255, 199, 255, 198, 255, 197, 255, 196, 255, 196, 255, 196, 255, 196, 255, 196, 255};
	const uint8_t *schemes[] = {original, blackwhite, bluered};
	const uint8_t *s = schemes[scheme];
	value *= 2;
	const unsigned long len = sizeof(original);
	const uint8_t fg = *(s + (1 + value) % len);
	const uint8_t bg = *(s + (0 + value) % len);
	out[0] = fg;
	out[1] = bg;
	return out;
}

/** calculates `floor(log_10(n) + 1)` */
__attribute__((pure)) uint8_t digitCount(uint32_t n)
{
	uint8_t c = 0;
	do
	{
		n /= 10;
		c += 1;
	} while (n);
	return c;
}

void drawBoard(const uint8_t board[SIZE][SIZE], const uint8_t scheme)
{
	uint8_t x, y, fg, bg;
	printf("\033[H");
	printf("2048.c %17d pts\n\n", score);
	for (y = 0; y < SIZE; y++)
	{
		for (x = 0; x < SIZE; x++)
		{
			uint8_t *colors = getColors(board[x][y], scheme);
			fg = colors[0];
			bg = colors[1];
			printf("\033[38;5;%d;48;5;%dm", fg, bg); // set color
			printf("       ");
			printf("\033[m"); // reset
		}
		printf("\n");
		for (x = 0; x < SIZE; x++)
		{
			uint8_t *colors = getColors(board[x][y], scheme);
			fg = colors[0];
			bg = colors[1];
			printf("\033[38;5;%d;48;5;%dm", fg, bg); // set color
			if (board[x][y] != 0)
			{
				uint32_t number = 1 << board[x][y];
				uint8_t t = 7 - digitCount(number);
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
			uint8_t *colors = getColors(board[x][y], scheme);
			fg = colors[0];
			bg = colors[1];
			printf("\033[38;5;%d;48;5;%dm", fg, bg); // set color
			printf("       ");
			printf("\033[m"); // reset
		}
		printf("\n");
	}
	printf("\n");
	printf("        ←,↑,→,↓ or q        \n");
	printf("\033[A"); // one line up
}

uint8_t findTarget(const uint8_t array[SIZE], const uint8_t x, const uint8_t stop)
{
	// if the position is already on the first, don't evaluate
	if (x == 0)
	{
		return x;
	}
	for (uint8_t t = x - 1;; t--)
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

/** call `rotateBoard` N times */
void rotateBoardN(const uint8_t board[SIZE][SIZE], uint8_t n){
	while (n--)
		rotateBoard(board);
}

bool moveUp(const uint8_t board[SIZE][SIZE])
{
	bool success = false;
	for (uint8_t x = 0; x < SIZE; x++)
		success |= slideArray(board[x]);

	return success;
}

bool moveLeft(const uint8_t board[SIZE][SIZE])
{
	rotateBoard(board);
	const bool success = moveUp(board);
	rotateBoardN(board, 3);
	return success;
}

bool moveDown(const uint8_t board[SIZE][SIZE])
{
	rotateBoardN(board, 2);
	const bool success = moveUp(board);
	rotateBoardN(board, 2);
	return success;
}

bool moveRight(const uint8_t board[SIZE][SIZE])
{
	rotateBoardN(board, 3);
	const bool success = moveUp(board);
	rotateBoard(board);
	return success;
}

/** returns `bool success` */
bool findPairDown(const uint8_t board[SIZE][SIZE])
{
	uint8_t x, y;
	for (x = 0; x < SIZE; x++)
	{
		for (y = 0; y < SIZE - 1; y++)
		{
			if (board[x][y] == board[x][y + 1])
				return true;
		}
	}
	return false;
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
	rotateBoardN(board, 3);
	return ended;
}

void addRandom(uint8_t board[SIZE][SIZE])
{
	static bool initialized = false;
	uint8_t x, y, r, len = 0;
	uint8_t n, list[SIZE * SIZE][2];

	if (!initialized)
	{
		srand(time(NULL));
		initialized = true;
	}

	for (x = 0; x < SIZE; x++)
		for (y = 0; y < SIZE; y++)
			if (board[x][y] == 0)
			{
				list[len][0] = x;
				list[len][1] = y;
				len++;
			}

	if (len > 0)
	{
		const uint8_t r = rand() % len;
		x = list[r][0];
		y = list[r][1];
		n = (rand() % 10) / 9 + 1;
		board[x][y] = n;
	}
}

void initBoard(uint8_t board[SIZE][SIZE], const uint8_t scheme)
{
	for (uint8_t x = 0; x < SIZE; x++)
		for (uint8_t y = 0; y < SIZE; y++)
			board[x][y] = 0;

	addRandom(board);
	addRandom(board);
	drawBoard(board, scheme);
	score = 0;
}

void setBufferedInput(const bool enable)
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

unsigned long len(const a[]){
	return sizeof(a) / sizeof(a[0]);
}

int test()
{
	uint8_t array[SIZE];
	/** these are exponents with base 2 (1->2 2->4 3->8) */
	const uint8_t data[] = {
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

	tests = len(data) / (2 * SIZE);
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
	uint8_t scheme = 0;
	uint8_t board[SIZE][SIZE];
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

	initBoard(board, scheme);
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
			success = moveLeft(board);
			break;
		case 100: // 'd' key
		case 108: // 'l' key
		case 67:  // right arrow
			success = moveRight(board);
			break;
		case 119: // 'w' key
		case 107: // 'k' key
		case 65:  // up arrow
			success = moveUp(board);
			break;
		case 115: // 's' key
		case 106: // 'j' key
		case 66:  // down arrow
			success = moveDown(board);
			break;
		default:
			success = false;
		}
		if (success)
		{
			drawBoard(board, scheme);
			usleep(150000);
			addRandom(board);
			drawBoard(board, scheme);
			if (gameEnded(board))
			{
				printf("         GAME OVER          \n");
				break;
			}
		}
		if (c == 'q')
		{
			printf("        QUIT? (y/n)         \n");
			c = getchar();
			if (c == 'y')
			{
				break;
			}
			drawBoard(board, scheme);
		}
		if (c == 'r')
		{
			printf("       RESTART? (y/n)       \n");
			c = getchar();
			if (c == 'y')
			{
				initBoard(board, scheme);
			}
			drawBoard(board, scheme);
		}
	}
	setBufferedInput(true);

	printf("\033[?25h\033[m");

	return EXIT_SUCCESS;
}
