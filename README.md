2048.c
======

[Spanish](README_es.md)

![screenshot](screenshot.png)

Console version of the game "2048" for GNU/Linux

### Gameplay

You can move the tiles in four directions using the arrow keys: up, down, left, and right. All numbers on the board will slide into that direction until they hit the wall and if they bump into each other then two numbers will be combined into one if they have the same value. Each number will only be combined once per move. Every move a new number 2 or 4 appears. If you have a 2048 on the board you have won, but you lose once the board is full and you cannot make a move. 

### Building
Make sure you have a C compiler. The project uses `gcc` by default, though you can specify that through the command line to use your preferred C compiler.

This project has been tested on GNU/Linux, FreeBSD, and OpenBSD, though it should really build on any system.

To build 2048, do the following

    make

To install 2048 on your system, do the following. This will install the 2048 binary to `/usr/bin`

    sudo make install

Check out the [Makefile](./Makefile) for other functions you can use as well as other variables you might want to set up.

### Running

The game supports different color schemes. This depends on ANSI support for 88 or 256 colors. If there are not enough colors supported the game will fallback to black and white (still very much playable).

For the original color scheme run

    ./2048

For the black-to-white color scheme (requires 256 colors)

    ./2048 blackwhite

For the blue-to-red color scheme (requires 256 colors)

    ./2048 bluered

### Contributing

Contributions are very welcome. Always run the tests before committing

    ./2048 test
    All 13 tests executed successfully
