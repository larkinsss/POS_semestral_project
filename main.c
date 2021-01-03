#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define null NULL
#define SIZE 11

int gameArea[40];
int playerA[4];
int playerB[4];
int playerC[4];
int playerD[4];

int drawArea[SIZE][SIZE] = {
        {41, 42, 0, 0, 1, 2, 3, 0, 0, 51, 52},
        {43, 44, 0, 0, 40, 55, 4, 0, 0, 53, 54},
        {0, 0, 0, 0, 39, 56, 5, 0, 0, 0, 0},
        {0, 0, 0, 0, 38, 57, 6, 0, 0, 0, 0},
        {33, 34, 35, 36, 37, 58, 7, 8, 9, 10, 11},
        {32, 45, 46, 47, 48, 0, 68, 67, 66, 65, 12},
        {31, 30, 29, 28, 27, 78, 17, 16, 15, 14, 13},
        {0, 0, 0, 0, 26, 77, 18, 0, 0, 0, 0},
        {0, 0, 0, 0, 25, 76, 19, 0, 0, 0, 0},
        {71, 72, 0, 0, 24, 75, 20, 0, 0, 61, 62},
        {73, 74, 0, 0, 23, 22, 21, 0, 0, 63, 64},
};

int diceroll()
{
    return 1 + rand() % 6;
}

void init()
{
    for (int i = 0; i < strlen(gameArea); ++i) {
        gameArea[i] = 0;
    }

    for (int i = 0; i < 4; ++i) {
        playerA[i] = 1;
        playerB[i] = 2;
        playerC[i] = 3;
        playerD[i] = 4;
    }
}

void draw()
{
    int element;
    char str[24] = "                       ";

    for (int row = 0; row < SIZE; ++row) {
        for (int col = 0; col < SIZE; ++col) {

            element = drawArea[row][col];

            if (element == 0) {
                str[col*2] = ' ';
            } else if (element <= 40) {
                str[col*2] = gameArea[element - 1] == 0 ? 'o' : 'x';
                //strcpy(str[col], &gameArea[element]);
            } else if (element <= 50) {
                str[col*2] = playerA[(element % 40) - 1] == 1 ? '1' : '_';
                //strcpy(str[col], &playerA[element % 40]);
            }else if (element <= 60) {
                str[col*2] = playerB[(element % 50) - 1] == 2 ? '2' : '_';
                //strcpy(str[col], &playerB[element % 50]);
            }else if (element <= 70) {
                str[col*2] = playerC[(element % 60) - 1] == 3 ? '3' : '_';
                //strcpy(str[col], &playerC[element % 60]);
            }else {
                str[col*2] = playerD[(element % 70) - 1] == 4 ? '4' : '_';
                //strcpy(str[col], &playerD[element % 70]);
            }

        }
        printf("%s\n", str);
    }
}

int main()
{


    srand(time(null));

    init();

    draw();
    return 0;
}

