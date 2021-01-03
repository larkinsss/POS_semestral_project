#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <curses.h>
#include <unistd.h>

#define null NULL
#define SIZE 11

typedef enum hrac{
    p1,
    p2,
    p3,
    p4
} HRAC_ENUM;

typedef struct suradnice{
    int surA;
    int surB;
} SUR;

typedef struct{
    struct suradnice surky;

} PAWN;

SUR gameArea[40] = {
        {0, 4},
        {0, 5},
        {0, 6},
        {1, 6},
        {2, 6},
        {3, 6},
        {4, 6},
        {4, 7},
        {4, 8},
        {4, 9},
        {4, 10},
        {5, 10},
        {6, 10},
        {6, 9},
        {6, 8},
        {6, 7},
        {6, 6},
        {7, 6},
        {8, 6},
        {9, 6},
        {10, 6},
        {10, 5},
        {10, 4},
        {9, 4},
        {8, 4},
        {7, 4},
        {6, 4},
        {6, 3},
        {6, 2},
        {6, 1},
        {6, 0},
        {5, 0},
        {4, 0},
        {4, 1},
        {4, 2},
        {4, 3},
        {4, 4},
        {3, 4},
        {2, 4},
        {1, 4},
};

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
    initscr();

    start_color();
    init_pair(1,COLOR_YELLOW, COLOR_BLACK);
    init_pair(2,COLOR_GREEN, COLOR_BLACK);
    init_pair(3,COLOR_BLUE, COLOR_BLACK);
    init_pair(4,COLOR_RED, COLOR_BLACK);

//    for (int i = 0; i < strlen(gameArea); ++i) {
//        gameArea[i] = 0;
//    }

    for (int i = 0; i < 4; ++i) {
        playerA[i] = 1;
        playerB[i] = 2;
        playerC[i] = 3;
        playerD[i] = 4;
    }
}

void draw()
{

    printw("A A     . . .     B B\n"
           "A A     . | .     B B\n"
           "        . | .        \n"
           "        . | .        \n"
           ". . . . . | . . . . .\n"
           ". - - - -   - - - - .\n"
           ". . . . . | . . . . .\n"
           "        . | .        \n"
           "        . | .        \n"
           "D D     . | .     C C\n"
           "D D     . . .     C C\n");

    attron(COLOR_PAIR(1));
    mvprintw(0,0,"1");
    mvprintw(0,2,"2");
    mvprintw(1,0,"3");
    mvprintw(1,2,"4");
    mvprintw(5,2,"_");
    mvprintw(5,4,"_");
    mvprintw(5,6,"_");
    mvprintw(5,8,"_");
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(2));
    mvprintw(0,18,"1");
    mvprintw(0,20,"2");
    mvprintw(1,18,"3");
    mvprintw(1,20,"4");
    mvprintw(1,10,"|");
    mvprintw(2,10,"|");
    mvprintw(3,10,"|");
    mvprintw(4,10,"|");
    attroff(COLOR_PAIR(2));

    attron(COLOR_PAIR(3));
    mvprintw(9,0,"1");
    mvprintw(9,2,"2");
    mvprintw(10,0,"3");
    mvprintw(10,2,"4");
    mvprintw(6,10,"|");
    mvprintw(7,10,"|");
    mvprintw(8,10,"|");
    mvprintw(9,10,"|");
    attroff(COLOR_PAIR(3));

    attron(COLOR_PAIR(4));
    mvprintw(9,18,"1");
    mvprintw(9,20,"2");
    mvprintw(10,18,"3");
    mvprintw(10,20,"4");
    mvprintw(5,12,"_");
    mvprintw(5,14,"_");
    mvprintw(5,16,"_");
    mvprintw(5,18,"_");
    attroff(COLOR_PAIR(4));
    refresh();
}

int main()
{

#define END false
    srand(time(null));

    init();

    draw();

//    for (int i = 0; i < 40; ++i) {
//        mvprintw(gameArea[i].surA,gameArea[i].surB*2, ".");
//        mvprintw(gameArea[(i+1) % 40].surA,gameArea[(i+1) % 40].surB*2, "0");
//        //getch();
//        //sleep(1);
//        move(gameArea[17].surA,gameArea[17].surB*2);
//        refresh();
//
//    }

    int panacik;
    int generovaneCislo;

    while(!END) {
        generovaneCislo = 1 + rand() % (6);

        scanw("%d", panacik);

    }





    getch();
    endwin();
#undef END
    return 0;
}

