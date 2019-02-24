#ifndef H2048H
#define H2048H
/*
 ============================================================================
 Name        : 2048.h
 Author      : Maurits van der Schee
 Description : Header File for 2048.c
 ============================================================================
 */
#define SIZE 4

/*
=============================================================================


                             SYSTEM DEFENITIONS


=============================================================================
*/
void signal_callback_handler(int signum);
void setBufferedInput(bool enable);
void getColor(uint8_t value, char *color, size_t length);
void drawBoard(uint8_t board[SIZE][SIZE]);
/*
=============================================================================


                            TEST DEFENITIONS


=============================================================================
*/
int test();

/*
=============================================================================


                        GAME LOGIC DEFENITIONS


=============================================================================
*/
void initBoard(uint8_t board[SIZE][SIZE]);
void addRandom(uint8_t board[SIZE][SIZE]);
bool gameEnded(uint8_t board[SIZE][SIZE]);
uint8_t countEmpty(uint8_t board[SIZE][SIZE]);
bool findPairDown(uint8_t board[SIZE][SIZE]);
bool moveUp(uint8_t board[SIZE][SIZE]);
bool moveLeft(uint8_t board[SIZE][SIZE]);
bool moveDown(uint8_t board[SIZE][SIZE]);
bool moveRight(uint8_t board[SIZE][SIZE]);
void rotateBoard(uint8_t board[SIZE][SIZE]);
bool slideArray(uint8_t array[SIZE]);
uint8_t findTarget(uint8_t array[SIZE],uint8_t x,uint8_t stop);


#endif /* end of include guard: 2048.H */
