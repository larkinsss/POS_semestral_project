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

typedef struct coordinates{
    int surA;
    int surB;
} SUR;

typedef struct{
    struct coordinates coords;
    int traveled;
    bool active;
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

PAWN playerA[4];
PAWN playerB[4];
PAWN playerC[4];
PAWN playerD[4];

void init()
{
    initscr();

    start_color();
    init_pair(1,COLOR_YELLOW, COLOR_BLACK);
    init_pair(2,COLOR_GREEN, COLOR_BLACK);
    init_pair(3,COLOR_BLUE, COLOR_BLACK);
    init_pair(4,COLOR_RED, COLOR_BLACK);

    playerA[0] = (PAWN){{0,0},0,false};
    playerA[1] = (PAWN){{0,2},0,false};
    playerA[2] = (PAWN){{1,0},0,false} ;
    playerA[3] = (PAWN){{1,2},0,false};

    playerB[0] = (PAWN){{0,18},0,false};
    playerB[1] = (PAWN){{0,20},0,false};
    playerB[2] = (PAWN){{1,18},0,false};
    playerB[3] = (PAWN){{1,20},0,false};

    playerC[0] = (PAWN){{9,0},0,false};
    playerC[1] = (PAWN){{9,2},0,false};
    playerC[2] = (PAWN){{10,0},0,false};
    playerC[3] = (PAWN){{10,2},0,false};

    playerD[0] = (PAWN){{9,18},0,false};
    playerD[1] = (PAWN){{9,20},0,false};
    playerD[2] = (PAWN){{10,18},0,false};
    playerD[3] = (PAWN){{10,20},0,false};
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

    bool end = false;
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

    move(11,0);

    while(!end) {
        generovaneCislo = 1 + rand() % (6);

        mvprintw(11,0,"Hodil si cislo: %d \n", generovaneCislo);

        bool playerAhasActive = false;
        for (int i = 0; i < 4; ++i) {
            if (playerA[i].active) {
                printw("%d", i+1);
                playerAhasActive = true;
            }
        }
        printw("\n");

        if (playerAhasActive) {
            mvprintw(13,0,"Zadaj cislo panacika:");
            scanw("%d",&panacik);

            if (playerA[panacik-1].active) {
                playerA[panacik-1].traveled += generovaneCislo;
                playerA[panacik-1].coords = gameArea[33+generovaneCislo];
            }
        }
    }





    getch();
    endwin();
    return 0;
}

