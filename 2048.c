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

void getColor(uint16_t value, char *color, size_t length)
{
    uint8_t original[] = {8, 255, 1, 255, 2, 255, 3, 255, 4, 255, 5, 255, 6, 255, 7, 255, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 255, 0, 255, 0};
    //uint8_t blackwhite[] = {232,255,234,255,236,255,238,255,240,255,242,255,244,255,246,0,248,0,249,0,250,0,251,0,252,0,253,0,254,0,255,0};
    //uint8_t bluered[] = {235,255,63,255,57,255,93,255,129,255,165,255,201,255,200,255,199,255,198,255,197,255,196,255,196,255,196,255,196,255,196,255};
    uint8_t *scheme = original;
    uint8_t *background = scheme + 0;
    uint8_t *foreground = scheme + 1;
    if (value > 0)
    {
        while (value >>= 1)
        {
            if (background+2<scheme+sizeof(original))
            {
                background+=2;
                foreground+=2;
            }
        }
    }
    snprintf(color,length,"\033[38;5;%d;48;5;%dm",*foreground,*background);
}

void drawBoard(uint16_t board[SIZE][SIZE])
{
    int8_t x, y;
    char color[40], reset[] = "\033[m";
    printf("\033[H");

    printf("2048.c %17d pts\n\n",score);

    for (y = 0; y < SIZE; y++)
    {
        for (x = 0; x < SIZE; x++)
        {
            getColor(board[x][y], color, 40);
            printf("%s       %s", color, reset);
        }
        printf("\n");
        for (x = 0; x < SIZE; x++)
        {
            getColor(board[x][y], color, 40);
            printf("%s", color);
            if (board[x][y]!=0)
            {
                char s[8];
                snprintf(s, 8, "%u", board[x][y]);
                int8_t t = 7 - strlen(s);
                printf("%*s%s%*s", t/2, "", s, t/2, "");
            }
            else
            {
                printf("   ·   ");
            }
            printf("%s", reset);
        }
        printf("\n");
        for (x = 0; x < SIZE; x++)
        {
            getColor(board[x][y], color, 40);
            printf("%s       %s", color, reset);
        }
        printf("\n");
    }
    printf("\n");
    printf("        ←,↑,→,↓ or q        \n");
    printf("\033[A");
}

int8_t findTarget(uint16_t array[SIZE],int8_t x, int8_t stop)
{
    int8_t t;
    // if the position is already on the first, don't evaluate
    if (x == 0)
    {
        return x;
    }
    for(t = x-1; t >= 0; t--)
    {
        if (array[t] != 0)
        {
            if (array[t] != array[x])
            {
                // merge is not possible, take next position
                return t+1;
            }
            return t;
        }
        else if (t == stop)
        {
            // we should not slide further, return this one
            return t;
        }
    }
    // we did not find one
    return x;
}

bool slideArray(uint16_t array[SIZE])
{
    bool success = false;
    int8_t x, t, stop = 0;

    for (x = 0; x < SIZE; x++)
    {
        if (array[x] != 0)
        {
            // if target is not original position, then move or merge
            if ((t = findTarget(array, x, stop)) != x)
            {
                // if target is not zero, set stop to avoid double merge
                if (array[t] != 0)
                {
                    score += array[t] + array[x];
                    stop = t+1;
                }
                array[t] += array[x];
                array[x] = 0;
                success = true;
            }
        }
    }
    return success;
}

void rotateBoard(uint16_t board[SIZE][SIZE])
{
    int8_t i, j, n = SIZE;
    uint16_t tmp;
    for (i = 0; i < n/2; i++)
    {
        for (j = i; j < n-i-1; j++)
        {
            tmp = board[i][j];
            board[i][j] = board[j][n-i-1];
            board[j][n-i-1] = board[n-i-1][n-j-1];
            board[n-i-1][n-j-1] = board[n-j-1][i];
            board[n-j-1][i] = tmp;
        }
    }
}

bool moveUp(uint16_t board[SIZE][SIZE])
{
    bool success = false;
    int8_t x;
    for (x = 0; x < SIZE; x++)
    {
        success |= slideArray(board[x]);
    }
    return success;
}

bool moveLeft(uint16_t board[SIZE][SIZE])
{
    bool success;
    rotateBoard(board);
    success = moveUp(board);
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    return success;
}

bool moveDown(uint16_t board[SIZE][SIZE])
{
    bool success;
    rotateBoard(board);
    rotateBoard(board);
    success = moveUp(board);
    rotateBoard(board);
    rotateBoard(board);
    return success;
}

bool moveRight(uint16_t board[SIZE][SIZE])
{
    bool success;
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    success = moveUp(board);
    rotateBoard(board);
    return success;
}

bool findPairDown(uint16_t board[SIZE][SIZE])
{
    int8_t x, y;
    for (x = 0; x < SIZE; x++)
    {
        for (y = 0; y < SIZE-1; y++)
        {
            if (board[x][y] == board[x][y+1])
                return true;
        }
    }
    return false;
}

int16_t countEmpty(uint16_t board[SIZE][SIZE])
{
    int8_t x, y;
    int16_t count = 0;
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

bool gameEnded(uint16_t board[SIZE][SIZE])
{
    bool ended = true;
    if (countEmpty(board)>0)
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

void addRandom(uint16_t board[SIZE][SIZE])
{
    static bool initialized = false;
    int8_t x, y;
    int16_t r, len = 0;
    uint16_t n,list[SIZE*SIZE][2];

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
        r = rand()%len;
        x = list[r][0];
        y = list[r][1];
        n = ((rand()%10) / 9+1) * 2;
        board[x][y] = n;
    }
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

int test(void)
{
    uint16_t array[SIZE];
    uint16_t data[] =
    {
        0,0,0,2,	2,0,0,0,
        0,0,2,2,	4,0,0,0,
        0,2,0,2,	4,0,0,0,
        2,0,0,2,	4,0,0,0,
        2,0,2,0,	4,0,0,0,
        2,2,2,0,	4,2,0,0,
        2,0,2,2,	4,2,0,0,
        2,2,0,2,	4,2,0,0,
        2,2,2,2,	4,4,0,0,
        4,4,2,2,	8,4,0,0,
        2,2,4,4,	4,8,0,0,
        8,0,2,2,	8,4,0,0,
        4,0,2,2,	4,4,0,0
    };
    uint16_t *in, *out;
    uint16_t t, tests;
    uint8_t i;
    bool success = true;

    tests = (sizeof(data) / sizeof(data[0]))/(2*SIZE);
    for (t = 0; t < tests; t++)
    {
        in = data + t*2*SIZE;
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
    printf("\033[?25h");
    exit(signum);
}

int quit(void)
{
    printf("        QUIT? (y/n)         \n");
    if (getchar() == 'y')
    {
        setBufferedInput(true);
        printf("\033[?25h");
        return 1;
    }
    else
    {
        drawBoard(board);
        return 0;
    }
}

void restart(void)
{
    printf("       RESTART? (y/n)       \n");
    if (getchar() == 'y')
    {
        memset(board, 0, sizeof(board));
        addRandom(board);
        addRandom(board);
        drawBoard(board);
    }
    else
    {
        drawBoard(board);
    }
}

int main(int argc, char *argv[])
{
    uint16_t board[SIZE][SIZE];
    int c;
    bool success, _loop = true;

    if (argc >= 2 && strcmp(argv[1], "test") == 0)
    {
        return test();
    }

    printf("\033[?25l\033[2J\033[H");

    // register signal handler for when ctrl-c is pressed
    signal(SIGINT, signal_callback_handler);

    memset(board, 0, sizeof(board));
    addRandom(board);
    addRandom(board);
    drawBoard(board);
    setBufferedInput(false);
    while (_loop)
    {
        /*I recommand to use defines instead of numbers, it would be easier to understand and there wouldn't be commentaries*/
        c = getchar();
        switch(c)
        {
        case 97:	// 'a' key
        case 104:	// 'h' key
        case 68:	// left arrow
            success = moveLeft(board);
            break;
        case 100:	// 'd' key
        case 108:	// 'l' key
        case 67:	// right arrow
            success = moveRight(board);
            break;
        case 119:	// 'w' key
        case 107:	// 'k' key
        case 65:	// up arrow
            success = moveUp(board);
            break;
        case 115:	// 's' key
        case 106:	// 'j' key
        case 66:	// down arrow
            success = moveDown(board);
            break;
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
                printf("         GAME OVER          \n");
                break;
            }
        }
        if (c == 'q')
        {
            if(quit() == 1)
                _loop = false;
            break;
        }
        if (c == 'r')
        {
            restart();
            break;
        }
    }
    setBufferedInput(true);

    printf("\033[?25h");

    return EXIT_SUCCESS;
}

