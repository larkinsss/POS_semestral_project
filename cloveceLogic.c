#include "cloveceLogic.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <curses.h>
#include <unistd.h>
#include "server.c"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    Data gameData;

    int sockfd, newsockfd;
    socklen_t cli_len;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    //char buffer[256];
    Data *dataToSend;

    if (argc < 2) {
        fprintf(stderr, "usage %s port\n", argv[0]);
        return 1;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(atoi(argv[1]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return 1;
    }

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding socket address");
        return 2;
    }

    listen(sockfd, 5);
    cli_len = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len);
    if (newsockfd < 0) {
        perror("ERROR on accept");
        return 3;
    }



    //bzero(dataToSend->buffer, 256);
    //n = read(newsockfd, dataToSend, 255);
//    if (n < 0) {
//        perror("Error reading from socket");
//        return 4;
//    }
    //printf("Here is the message: %s\n", dataToSend->buffer);

    //const char *msg = "I got your message";

    //void *gamedata = (void *)&gameData;

    startGame(&gameData);
    n = write(newsockfd, &gameData, sizeof(Data));
    if (n < 0) {
        perror("Error writing to socket");
        return 5;
    }

    close(newsockfd);
    close(sockfd);
}

void init(Data *data)
{
    /*
    initscr();
    start_color();
    init_pair(1,COLOR_YELLOW, COLOR_BLACK);
    init_pair(2,COLOR_GREEN, COLOR_BLACK);
    init_pair(3,COLOR_BLUE, COLOR_BLACK);
    init_pair(4,COLOR_RED, COLOR_BLACK);
    */

    *data = (Data) {
        {
            {
                (Pawn) {playerPos[0][0][0], 0, '1', false},
                (Pawn) {playerPos[0][0][1], 0, '2', false},
                (Pawn) {playerPos[0][0][2], 0, '3', false},
                (Pawn) {playerPos[0][0][3], 0, '4', false}
            },
            {
                (Pawn) {playerPos[1][0][0], 0, '1', false},
                (Pawn) {playerPos[1][0][1], 0, '2', false},
                (Pawn) {playerPos[1][0][2], 0, '3', false},
                (Pawn) {playerPos[1][0][3], 0, '4', false}
            },
            {
                (Pawn) {playerPos[2][0][0], 0, '1', false},
                (Pawn) {playerPos[2][0][1], 0, '2', false},
                (Pawn) {playerPos[2][0][2], 0, '3', false},
                (Pawn) {playerPos[2][0][3], 0, '4', false}
            },
            {
                (Pawn) {playerPos[3][0][0], 0, '1', false},
                (Pawn) {playerPos[3][0][1], 0, '2', false},
                (Pawn) {playerPos[3][0][2], 0, '3', false},
                (Pawn) {playerPos[3][0][3], 0, '4', false}
             }
        }
    };
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

void startGame(Data *gameData) {
    srand(time(NULL));
    init(gameData);
}

int gameLogic(Data *gameData)
{

    bool end = false;

    int panacik;
    int generovaneCislo;
    move(11,0);

    /*while(!end) {
        generovaneCislo = 1 + rand() % (6);

        mvprintw(11,0,"Hodil si cislo: %d \n", generovaneCislo);

        bool playerAhasActive = false;
        for (int i = 0; i < 4; ++i) {
            if (playerA[i].isActive) {
                printw("%d", i+1);
                playerAhasActive = true;
            }
        }
        printw("\n");

        if (playerAhasActive) {
            mvprintw(13,0,"Zadaj cislo panacika:");
            scanw("%d",&panacik);

            if (playerA[panacik-1].isActive) {
                playerA[panacik-1].travelled += generovaneCislo;
                playerA[panacik-1].pos = gameArea[33+generovaneCislo];
                end = true;
            }
        }
    }*/

    getch();
    endwin();
    return 0;
}