/*
 ============================================================================
 Name        : 2048.c
 Author      : Maurits van der Schee
 Description : Console version of the game "2048" for GNU/Linux
 ============================================================================
 */

#define VERSION "1.0.3"

#define _XOPEN_SOURCE 500 // for: usleep
#include <stdio.h>		  // defines: printf, puts, getchar
#include <stdlib.h>		  // defines: EXIT_SUCCESS
#include <string.h>		  // defines: strcmp
#include <unistd.h>		  // defines: STDIN_FILENO, usleep
#include <termios.h>	  // defines: termios, TCSANOW, ICANON, ECHO
#include <stdbool.h>	  // defines: true, false
#include <stdint.h>		  // defines: uint8_t, uint32_t
#include <time.h>		  // defines: time
#include <signal.h>		  // defines: signal, SIGINT
#include <libintl.h>	  // defines: number format
#include <locale.h>

#if defined(SIZE) && ((SIZE < 4) || (SIZE > 9))
#warning "The board ranges from 4x4 to 9x9, default to 4x4"
#undef SIZE
#endif
#ifndef SIZE
#define SIZE 4
#endif

#define ROW_WIDTH (SIZE * 7)

#define PRINT_HEAD(score)                                    \
	do                                                       \
	{                                                        \
		char fmt[48];                                        \
		uint8_t len = ROW_WIDTH - 11;                        \
		len = sprintf(fmt, "\n2048.c %%\'%du pts\n\n", len); \
		fmt[len] = '\0';                                     \
		printf(fmt, score);                                  \
	} while (0)

#define PRINT_FOOT(str, len)                   \
	do                                         \
	{                                          \
		char pad[ROW_WIDTH];                   \
		memset(pad, ' ', ROW_WIDTH);           \
		pad[(ROW_WIDTH - len + 1) / 2] = '\0'; \
		printf("\n%s%s%s", pad, str, pad);     \
	} while (0)

// this function receives 2 pointers (indicated by *) so it can set their values
void getColors(uint8_t value, uint8_t scheme, uint8_t *foreground, uint8_t *background)
{
	uint8_t original[] = {8, 255, 1, 255, 2, 255, 3, 255, 4, 255, 5, 255, 6, 255, 7, 0, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 255, 0, 255, 0};
	uint8_t blackwhite[] = {232, 255, 234, 255, 236, 255, 238, 255, 240, 255, 242, 255, 244, 255, 246, 0, 248, 0, 249, 0, 250, 0, 251, 0, 252, 0, 253, 0, 254, 0, 255, 0};
	uint8_t bluered[] = {235, 255, 63, 255, 57, 255, 93, 255, 129, 255, 165, 255, 201, 255, 200, 255, 199, 255, 198, 255, 197, 255, 196, 255, 196, 255, 196, 255, 196, 255, 196, 255};
	uint8_t *schemes[] = {original, blackwhite, bluered};
	// modify the 'pointed to' variables (using a * on the left hand of the assignment)
	*foreground = *(schemes[scheme] + (1 + value * 2) % sizeof(original));
	*background = *(schemes[scheme] + (0 + value * 2) % sizeof(original));
	// alternatively we could have returned a struct with two variables
}

uint8_t getDigitCount(uint32_t number)
{
	uint8_t count = 0;
	do
	{
		number /= 10;
		count += 1;
	} while (number);
	return count;
}

void drawBoard(uint8_t board[SIZE][SIZE], uint8_t scheme, uint32_t score)
{
	uint8_t x, y, fg, bg;
	printf("\033[?25l"); // hide cursor again due to reset vty
	printf("\033[H");	 // move cursor to 0,0
	PRINT_HEAD(score);
	for (y = 0; y < SIZE; y++)
	{
		for (x = 0; x < SIZE; x++)
		{
			// send the addresses of the foreground and background variables,
			// so that they can be modified by the getColors function
			getColors(board[x][y], scheme, &fg, &bg);
			printf("\033[38;5;%u;48;5;%um", fg, bg); // set color
			printf("       ");
			printf("\033[m"); // reset all modes
		}
		printf("\n");
		for (x = 0; x < SIZE; x++)
		{
			getColors(board[x][y], scheme, &fg, &bg);
			printf("\033[38;5;%u;48;5;%um", fg, bg); // set color
			if (board[x][y] != 0)
			{
				uint32_t number = 1 << board[x][y];
				uint8_t t = 7 - getDigitCount(number);
				printf("%*s%u%*s", t - t / 2, "", number, t / 2, "");
			}
			else
			{
				printf("   ·   ");
			}
			printf("\033[m"); // reset all modes
		}
		printf("\n");
		for (x = 0; x < SIZE; x++)
		{
			getColors(board[x][y], scheme, &fg, &bg);
			printf("\033[38;5;%u;48;5;%um", fg, bg); // set color
			printf("       ");
			printf("\033[m"); // reset all modes
		}
		printf("\n");
	}
	PRINT_FOOT("←,↑,→,↓,r or q", 14);
	printf("\033[A"); // one line up
}

uint8_t findTarget(uint8_t *array, uint8_t x, uint8_t stop)
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

bool slideArray(uint8_t *array, uint32_t *score, uint8_t size)
{
	bool success = false;
	uint8_t x, t, stop = 0;

	for (x = 0; x < size; x++)
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
					*score += 1 << array[t];
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

bool moveUp(uint8_t board[SIZE][SIZE], uint32_t *score)
{
	bool success = false;
	uint8_t x;
	for (x = 0; x < SIZE; x++)
	{
		success |= slideArray(board[x], score, SIZE);
	}
	return success;
}

bool moveLeft(uint8_t board[SIZE][SIZE], uint32_t *score)
{
	bool success;
	rotateBoard(board);
	success = moveUp(board, score);
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveDown(uint8_t board[SIZE][SIZE], uint32_t *score)
{
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board, score);
	rotateBoard(board);
	rotateBoard(board);
	return success;
}

bool moveRight(uint8_t board[SIZE][SIZE], uint32_t *score)
{
	bool success;
	rotateBoard(board);
	rotateBoard(board);
	rotateBoard(board);
	success = moveUp(board, score);
	rotateBoard(board);
	return success;
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

#define TEST_SIZE 4
bool testSucceed()
{
	uint8_t array[TEST_SIZE];
	// these are exponents with base 2 (1=2 2=4 3=8)
	// data holds per line: 4x IN, 4x OUT, 1x POINTS
	uint8_t data[] = {
		0, 0, 0, 1, 1, 0, 0, 0, 0,
		0, 0, 1, 1, 2, 0, 0, 0, 4,
		0, 1, 0, 1, 2, 0, 0, 0, 4,
		1, 0, 0, 1, 2, 0, 0, 0, 4,
		1, 0, 1, 0, 2, 0, 0, 0, 4,
		1, 1, 1, 0, 2, 1, 0, 0, 4,
		1, 0, 1, 1, 2, 1, 0, 0, 4,
		1, 1, 0, 1, 2, 1, 0, 0, 4,
		1, 1, 1, 1, 2, 2, 0, 0, 8,
		2, 2, 1, 1, 3, 2, 0, 0, 12,
		1, 1, 2, 2, 2, 3, 0, 0, 12,
		3, 0, 1, 1, 3, 2, 0, 0, 4,
		2, 0, 1, 1, 2, 2, 0, 0, 4};
	uint8_t *in, *out, *points;
	uint8_t t, tests;
	uint8_t i;
	bool success = true;
	uint32_t score;

	tests = (sizeof(data) / sizeof(data[0])) / (2 * TEST_SIZE + 1);
	for (t = 0; t < tests; t++)
	{
		in = data + t * (2 * TEST_SIZE + 1);
		out = in + TEST_SIZE;
		points = in + 2 * TEST_SIZE;
		for (i = 0; i < TEST_SIZE; i++)
		{
			array[i] = in[i];
		}
		score = 0;
		slideArray(array, &score, TEST_SIZE);
		for (i = 0; i < TEST_SIZE; i++)
		{
			if (array[i] != out[i])
			{
				success = false;
			}
		}
		if (score != *points)
		{
			success = false;
		}
		if (success == false)
		{
			for (i = 0; i < TEST_SIZE; i++)
			{
				printf("%u ", in[i]);
			}
			printf("=> ");
			for (i = 0; i < TEST_SIZE; i++)
			{
				printf("%u ", array[i]);
			}
			printf("(%u points) expected ", score);
			for (i = 0; i < TEST_SIZE; i++)
			{
				printf("%u ", in[i]);
			}
			printf("=> ");
			for (i = 0; i < TEST_SIZE; i++)
			{
				printf("%u ", out[i]);
			}
			printf("(%u points)\n", *points);
			break;
		}
	}
	if (success)
	{
		printf("All %u tests executed successfully\n", tests);
	}
	return success;
}

void signal_callback_handler(int signum)
{
	PRINT_FOOT("TERMINATED", 10);
	setBufferedInput(true);
	// make cursor visible, reset all modes
	printf("\033[?25h\033[m\n");
	exit(signum);
}

typedef enum
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	ROTATE,
	OTHER
} key_map_t;

key_map_t key_map(char c, bool termux)
{
	switch (c)
	{
	case 52:  // '4' key
	case 97:  // 'a' key
	case 104: // 'h' key
	case 68:  // left arrow
		return LEFT;

	case 54:  // '6' key
	case 100: // 'd' key
	case 108: // 'l' key
	case 67:  // right arrow
		return RIGHT;

	case 56: // '8' key
		if (termux)
			return DOWN;
	case 119: // 'w' key
	case 107: // 'k' key
	case 65:  // up arrow
		return UP;

	case 50: // '2' key
		if (termux)
			return UP;
	case 115: // 's' key
	case 106: // 'j' key
	case 66:  // down arrow
		return DOWN;

	case 53:  // '5' key
	case 114: // 'r' key
	case 117: // 'u' key
		return ROTATE;
	default:
		return OTHER;
	}
}
char rand_key()
{
	return "asd"[rand() % 3]; // left down right
}

int main(int argc, char *argv[])
{
	uint8_t board[SIZE][SIZE];
	uint8_t scheme = 0;
	uint32_t score = 0;
	int c;
	bool success;
	bool termux;
	bool demo;

	// handle the command line options
	if (argc > 1)
	{
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
		{
			printf("Usage: 2048 [OPTION] | [MODE]\n");
			printf("Play the game 2048 in the console\n\n");
			printf("Options:\n");
			printf("  -h,  --help       Show this help message.\n");
			printf("  -v,  --version    Show version number.\n\n");
			printf("Modes:\n");
			printf("  bluered      Use a blue-to-red color scheme (requires 256-color terminal support).\n");
			printf("  blackwhite   The black-to-white color scheme (requires 256-color terminal support).\n");
			return EXIT_SUCCESS;
		}
		else if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
		{
			printf("2048.c version %s\n", VERSION);
			return EXIT_SUCCESS;
		}
		else if (strcmp(argv[1], "blackwhite") == 0)
		{
			scheme = 1;
		}
		else if (strcmp(argv[1], "bluered") == 0)
		{
			scheme = 2;
		}
		else if (strcmp(argv[1], "test") == 0)
		{
			return testSucceed() ? EXIT_SUCCESS : EXIT_FAILURE;
		}
		else if (strcmp(argv[1], "demo") == 0)
		{
			demo = true;
		}
		else
		{
			printf("Invalid option: %s\n\nTry '%s --help' for more options.\n", argv[1], argv[0]);
			return EXIT_FAILURE;
		}
	}

	termux = getenv("TERMUX__PREFIX") != NULL ? true : false;
	setlocale(LC_NUMERIC, "en_US");
	// make cursor invisible, erase entire screen
	printf("\033[?25l\033[2J");

	// register signal handler for when ctrl-c is pressed
	signal(SIGINT, signal_callback_handler);

	initBoard(board);
	setBufferedInput(false);
	drawBoard(board, scheme, score);

	while (true)
	{
		c = demo ? rand_key() : getchar();
		if (c == EOF)
		{
			puts("\nError! Cannot read keyboard input!");
			break;
		}
		switch (key_map(c, termux))
		{
		case LEFT:
			success = moveLeft(board, &score);
			break;
		case RIGHT:
			success = moveRight(board, &score);
			break;
		case UP:
			success = moveUp(board, &score);
			break;
		case DOWN:
			success = moveDown(board, &score);
			break;
		case ROTATE:
			rotateBoard(board);
			drawBoard(board, scheme, score);
		case OTHER:
			success = false;
		}
		if (success)
		{
			drawBoard(board, scheme, score);
			usleep((demo ? 1 : 150) * 1000); // 1 or 150 ms
			addRandom(board);
			drawBoard(board, scheme, score);
			if (gameEnded(board))
			{
				PRINT_FOOT("GAME OVER", 9);
				break;
			}
		}
		if (c == 'q')
		{
			PRINT_FOOT("QUIT? (y/n)", 11);
			c = getchar();
			if (c == 'y')
			{
				break;
			}
			drawBoard(board, scheme, score);
		}
		else if (c == 'n')
		{
			PRINT_FOOT("RESTART? (y/n)", 14);
			c = getchar();
			if (c == 'y')
			{
				initBoard(board);
				score = 0;
			}
			drawBoard(board, scheme, score);
		}
	}
	setBufferedInput(true);

	// make cursor visible, reset all modes
	printf("\033[?25h\033[m\n");

	return EXIT_SUCCESS;
}
