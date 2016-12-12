2048.c
======

![screenshot](http://www.leaseweblabs.com/wp-content/uploads/2014/03/20481.png)

Console version of the game "2048" for GNU/Linux

### Gameplay

You can move the tiles in four directions using the arrow keys: up, down, left, and right. All numbers on the board will slide into that direction until they hit the wall and if they bump into each other then two numbers will be combined into one if they have the same value. Each number will only be combined once per move. Every move a new number 2 or 4 appears. If you have a 2048 on the board you have won, but you lose once the board is full and you cannot make a move. 

### Requirements

- C compiler
- GNU/Linux

### Installation

```
wget https://raw.githubusercontent.com/mevdschee/2048.c/master/2048.c
gcc -o 2048 2048.c
./2048
```

### Running

The game supports different color schemes. This depends on ANSI support for 88 or 256 colors. If there are not enough colors supported the game will fallback to black and white (still very much playable). For the original color scheme run:

```
./2048
```
For the black-to-white color scheme (requires 256 colors):

```
./2048 blackwhite
```

For the blue-to-red color scheme (requires 256 colors):

```
./2048 bluered
```

### Implemention on multi-user machines

To use this game on multi-user machines (to make all users able to play the game and add records to high scores):
1- login as root

2- change SCORE\_FILE in 2048.c to "/etc/2048\_score\_file"

3- copy score\_file shipped with the game to /etc/2048\_score\_file 

4- `gcc 2048.c -o 2048`

5- copy 2048 to /usr/local/games/2048

6- `chown root /usr/local/games/2048; chmod 6111 /usr/local/games/2048; chown root /etc/2048_score_file; chmod 600 /etc/2048_score_file`


### Contributing

Contributions are very welcome. Always run the tests before committing using:

```
$ ./2048 test
All 13 tests executed successfully
```
