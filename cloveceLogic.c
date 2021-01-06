#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <curses.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include "cloveceLogic.h"
#include <pthread.h>

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

void* game(void *args) {

    GAME_DATA * data = (GAME_DATA *) args;
    bool itsGameTime = false;

    int currentPlayer = 1;

    while(!data->endGame) {

        pthread_mutex_lock(data->mutex);

        while (data->whosTurn == 0) {
            pthread_cond_wait(data->doMove,data->mutex);
        }

        int panacik = 13548616;
        int generovaneCislo = 1 + rand() % 6;
        currentPlayer++;


        if (currentPlayer > data->numberOfPlayers) {
            currentPlayer = 1;
        }

        data->whosTurn = currentPlayer;

        sprintf(data->option, "%d", generovaneCislo);

        panacik = write(data->newsocfd, data, sizeof(GAME_DATA));
        if (panacik < 0) {
            perror("Error writing to socket");
        }

        pthread_cond_broadcast(data->giveMove);
        pthread_mutex_unlock(data->mutex);


    }








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

void* playerLogic(void *args){

    GAME_DATA * data = (GAME_DATA *) args;
    DATA_FOR_PLAYER dataToSend = (DATA_FOR_PLAYER){data->playerData,data->playerId, data->endGame, data->whosTurn, data->option, data->numberOfPlayers };


    int n;

    while(!data->endGame) {

        pthread_mutex_lock(data->mutex);

        n = write(data->newsocfd, &dataToSend, sizeof(DATA_FOR_PLAYER));
        if (n < 0) {
            perror("Error writing to socket");
            //return 5;
        }

        while(data->whosTurn != data->playerId) {
            pthread_cond_wait(data->giveMove, data->mutex);
        }

        int msg;
        //while(atoi(data->option) == 0) {
        n = read(data->newsocfd, &data->option, 255);
        //}

        pthread_cond_signal(data->doMove);
        pthread_mutex_unlock(data->mutex);

    }
    return NULL;
}


int main(int argc, char *argv[]) {

    Data playerData;

    int sockfd;
    struct sockaddr_in serv_addr;
    int n;
    //char buffer[256];
    //Data *dataToSend;

    if (argc < 3) {
        fprintf(stderr, "usage %s port\n", argv[0]);
        return 1;
    }

    int toConnect = atoi(argv[2]);
    int connected = 0;
    int newsockfd[toConnect];
    struct sockaddr_in cli_addr[toConnect];
    socklen_t cli_len[toConnect];

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

    listen(sockfd, toConnect);

    pthread_t gameLogic;
    pthread_t playerThread[toConnect];
    pthread_mutex_t mutex;
    pthread_cond_t giveMove, doMove;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&doMove, NULL);
    pthread_cond_init(&giveMove, NULL);

    while(connected < toConnect) {
        cli_len[connected] = sizeof(cli_addr[connected]);
        newsockfd[connected] = accept(sockfd, (struct sockaddr *) &cli_addr[connected], &cli_len[connected]);
        if (newsockfd[connected] < 0) {
            perror("ERROR on accept");
            return 3;
        } else {
            connected++;
        }
    }

    startGame(&playerData);

    GAME_DATA gameData[toConnect];

    int firstPlayer = 0;

    for (int i = 0; i < connected; ++i) {
        gameData[i].playerData = playerData;
        gameData[i].playerId = i+1;
        gameData[i].newsocfd = newsockfd[i];
        gameData[i].endGame = false;
        gameData[i].doMove = &doMove;
        gameData[i].giveMove = &giveMove;
        gameData[i].mutex = &mutex;
        gameData[i].whosTurn = firstPlayer;
        gameData[i].numberOfPlayers = connected;
        pthread_create(&playerThread[i], NULL, &playerLogic, &gameData);
    }

    pthread_create(&gameLogic, NULL, &game, &gameData);

    mvprintw(13,0,"Pripojili sa %d hraci \n", connected);

    for (int i = 0; i < connected; ++i) {
        pthread_join(playerThread[i], NULL);
    }
    pthread_join(gameLogic, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&doMove);
    pthread_cond_destroy(&giveMove);
    for (int i = 0; i < connected; ++i) {
        close(newsockfd[i]);
    }
    close(sockfd);
}

