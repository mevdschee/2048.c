/*
 ============================================================================
 Name        : 2048.c
 Author      : Maurits van der Schee
 Description : Console version of the game "2048" for GNU/Linux
 ============================================================================
 */

#define _XOPEN_SOURCE 500 // for: usleep
#include <stdio.h>	  // defines: printf, puts, getchar
#include <stdlib.h>	  // defines: EXIT_SUCCESS
#include <string.h>	  // defines: strcmp
#include <unistd.h>	  // defines: STDIN_FILENO, usleep
#include <termios.h>	  // defines: termios, TCSANOW, ICANON, ECHO
#include <stdbool.h>	  // defines: true, false
#include <stdint.h>	  // defines: uint8_t, uint32_t
#include <time.h>	  // defines: time
#include <signal.h>	  // defines: signal, SIGINT

#define SIZE 4
#define VERSION "1.0"

// this function receives 2 pointers (indicated by *) so it can set their values
void getColors(uint8_t value, uint8_t scheme, uint8_t *foreground, uint8_t *background)
{
	uint8_t original[] = {8, 255, 1, 255, 2, 255, 3, 255, 4, 255, 5, 255, 6, 255, 7, 255, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 255, 0, 255, 0};
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
	printf("\033[H"); // move cursor to 0,0
	printf("2048.c %17d pts\n\n", score);
	for (y = 0; y < SIZE; y++)
	{
		for (x = 0; x < SIZE; x++)
		{
			// send the addresses of the foreground and background variables,
			// so that they can be modified by the getColors function
			getColors(board[x][y], scheme, &fg, &bg);
			printf("\033[38;5;%d;48;5;%dm", fg, bg); // set color
			printf("       ");
			printf("\033[m"); // reset all modes
		}
		printf("\n");
		for (x = 0; x < SIZE; x++)
		{
			getColors(board[x][y], scheme, &fg, &bg);
			printf("\033[38;5;%d;48;5;%dm", fg, bg); // set color
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
			printf("\033[38;5;%d;48;5;%dm", fg, bg); // set color
			printf("       ");
			printf("\033[m"); // reset all modes
		}
		printf("\n");
	}
	printf("\n");
	printf("        ←,↑,→,↓ or q        \n");
	printf("\033[A"); // one line up
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

bool slideArray(uint8_t array[SIZE], uint32_t *score)
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
					*score += (uint32_t)1 << array[t];
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
		success |= slideArray(board[x], score);
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

int test()
{
	uint8_t array[SIZE];
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

	tests = (sizeof(data) / sizeof(data[0])) / (2 * SIZE + 1);
	for (t = 0; t < tests; t++)
	{
		in = data + t * (2 * SIZE + 1);
		out = in + SIZE;
		points = in + 2 * SIZE;
		for (i = 0; i < SIZE; i++)
		{
			array[i] = in[i];
		}
		score = 0;
		slideArray(array, &score);
		for (i = 0; i < SIZE; i++)
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
			for (i = 0; i < SIZE; i++)
			{
				printf("%d ", in[i]);
			}
			printf("=> ");
			for (i = 0; i < SIZE; i++)
			{
				printf("%d ", array[i]);
			}
			printf("(%d points) expected ", score);
			for (i = 0; i < SIZE; i++)
			{
				printf("%d ", in[i]);
			}
			printf("=> ");
			for (i = 0; i < SIZE; i++)
			{
				printf("%d ", out[i]);
			}
			printf("(%d points)\n", *points);
			break;
		}
	}
	if (success)
	{
		printf("All %u tests executed successfully\n", tests);
	}
	return !success;
}

int version()
{
    printf("2048.c version %s\n", VERSION);
    return EXIT_SUCCESS;
}

void signal_callback_handler(int signum)
{
	printf("         TERMINATED         \n");
	setBufferedInput(true);
	// make cursor visible, reset all modes
	printf("\033[?25h\033[m");
	exit(signum);
}

struct CliOption
{
    char* name;
    char* description;
};
struct CliOption cli_options[] = {
    {"--help", "Show this help message."},
    {"--version", "Show version number."},
    {"bluered", "Use a blue-to-red color scheme (requires 256-color terminal support). If unsupported, it will fall back to black-to-white."},
    {"blackwhite", "The default color scheme is black-to-white (requires 256-color terminal support)."},
    {"test", "Run the test suite."},
};

char* parseArgs(int argc, char* argv[], int* status_code)
{
    if (argc == 1)
    {
        return "blackwhite";      // default color scheme
    }
    else if (argc == 2)
    {
        return argv[1];
    }
    else
    {
        printf("Invalid number of arguments\n");
        *status_code = EXIT_FAILURE;
        return "--help";
    }
}

int help(int success) {
    printf("Usage: 2048 [OPTION]\n");
    printf("Play the game 2048 in the console\n\n");
    printf("Options:\n");
    for (int i = 0; i < sizeof(cli_options) / sizeof(cli_options[0]); i++)
    {
        printf("  %-12s %s\n", cli_options[i].name, cli_options[i].description);
    }
    return success;
}

int main(int argc, char *argv[])
{
	uint8_t board[SIZE][SIZE];
	uint8_t scheme = 0;
	uint32_t score = 0;
	char c;
	bool success;

    int status_code = EXIT_SUCCESS;
    char* option = parseArgs(argc, argv, &status_code);
    if (strcmp(option, "--help") == 0 || strcmp(option, "-h") == 0)
    {
        return help(status_code);
    }
    else if (strcmp(option, "--version") == 0 || strcmp(option, "-v") == 0)
    {
        return version();
    }
	else if (strcmp(option, "test") == 0)
	{
		return test();
	}
	else if (strcmp(option, "blackwhite") == 0)
	{
		scheme = 1;
	}
	else if (strcmp(option, "bluered") == 0)
	{
		scheme = 2;
	} else {
        printf("Invalid option: %s\n", option);
        return help(EXIT_FAILURE);
    }

	// make cursor invisible, erase entire screen
	printf("\033[?25l\033[2J");

	// register signal handler for when ctrl-c is pressed
	signal(SIGINT, signal_callback_handler);

	initBoard(board);
	setBufferedInput(false);
	drawBoard(board, scheme, score);
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
			success = moveLeft(board, &score);
			break;
		case 100: // 'd' key
		case 108: // 'l' key
		case 67:  // right arrow
			success = moveRight(board, &score);
			break;
		case 119: // 'w' key
		case 107: // 'k' key
		case 65:  // up arrow
			success = moveUp(board, &score);
			break;
		case 115: // 's' key
		case 106: // 'j' key
		case 66:  // down arrow
			success = moveDown(board, &score);
			break;
		default:
			success = false;
		}
		if (success)
		{
			drawBoard(board, scheme, score);
			usleep(150 * 1000); // 150 ms
			addRandom(board);
			drawBoard(board, scheme, score);
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
			drawBoard(board, scheme, score);
		}
		if (c == 'r')
		{
			printf("       RESTART? (y/n)       \n");
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
	printf("\033[?25h\033[m");

	return EXIT_SUCCESS;
}
