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

enum Player p = PLAYER_1;

void init(PlayerData *data)
{
    srand(time(NULL));
    *data = (PlayerData) {
        2, -1, // TODO
        { 0, 0, 0, 0 },
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

void startGame(PlayerData *data)
{
    // TODO
}

int rollDie() {
    return rand() % 6 + 1;
}

void nextPlayer(PlayerData* data) {
    data->activePlayer = data->activePlayer <= 1 ? 0 : data->activePlayer + 1;
}

bool checkPawns(PlayerData* data)
{
    for (int i = 0; i < data->count; ++i) {
        if (data->pawnsOnEnd[i] >= 8) // TODO
        {
            return false;
        }
    }
    return true;
}

void sendDie(ThreadData* data)
{
    char msg[256];
    int die = rollDie();
    int n = 0;

    bzero(msg, 255);
    sprintf(msg, "You #%d rolled a %d", data->players->activePlayer, die);
    sleep(1);

    n = write(data->clSockFD[data->players->activePlayer], msg, strlen(msg));
    if (n < 0) {
        perror("Error writing to socket");
    }
}

void* gameThread(void *args)
{
    ThreadData *data = (ThreadData *) args;
    mutex_lock(data->mutex);
    printf("Server init\n");
    startGame(data->players);

    int n = 0;
    int die = 0;
    char msg[256];

    // First time init

    data->players->activePlayer = PLAYER_1;
    sendDie(data);
    /*die = rollDie();
    sprintf(msg, "You rolled a %d", die);
    n = write(data->clSockFD[PLAYER_1], msg, strlen(msg));
    if (n < 0) {
        perror("Error writing to socket");
    }*/
    data->players->activePlayer = PLAYER_2;
    mutex_unlock(data->mutex);
    cond_broadcast(data->wakeClient);
    printf("First time rolled %d\n", die);

    for (int i = data->players->activePlayer; checkPawns(data->players); i = (i + 1) % data->players->count) {
        mutex_lock(data->mutex);

        while (i == data->players->activePlayer) {
            cond_wait(data->wakeServer, data->mutex);
        }

        sendDie(data);
        data->players->pawnsOnEnd[0] += 1;

        mutex_unlock(data->mutex);
        cond_broadcast(data->wakeClient);
    }

    printf("GameThread end!\n");
    mutex_lock(data->mutex);
    data->end = true;
    mutex_unlock(data->mutex);
    cond_broadcast(data->wakeClient);

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
    return null;
}

void* playerThread(void *args)
{
    ThreadData *data = (ThreadData *) args;
    enum Player id = p++;
    printf("Player %d commence!\n", id);

    for (int i = 0; i < 2; ++i) { // TODO condition
        mutex_lock(data->mutex);

        while(data->players->activePlayer != id) {
            cond_wait(data->wakeClient, data->mutex);
            if (data->end) {
                mutex_unlock(data->mutex);
                return null;
            }
        }

        printf("Player %d woke up!\n", id);

        nextPlayer(data->players);
        printf("Player %d is next!\n", data->players->activePlayer);

        mutex_unlock(data->mutex);
        cond_broadcast(data->wakeServer);
    }

    return null;
}

int main(int argc, char *argv[])
{
    // Input arguments
    if (argc < 3) {
        fprintf(stderr, "usage %s port number_of_players\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);
    if (port <= 1024) {
        fprintf(stderr, "Port must be > 1024\n");
        return 2;
    }

    int playerCount = atoi(argv[2]);
    if (playerCount < 2 || playerCount > 4) {
        fprintf(stderr, "Nubmer of players must be 2, 3 or 4\n");
        return 3;
    }

    // Variable init
    PlayerData playerData;
    int svSockFD;
    struct sockaddr_in serv_addr;

    // Server address init
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // Server socket init
    svSockFD = socket(AF_INET, SOCK_STREAM, 0);
    if (svSockFD < 0) {
        perror("Error creating socket");
        return 4;
    }

    if (bind(svSockFD, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding socket address");
        return 5;
    }

    listen(svSockFD, playerCount);

    // Thread init
    Thread threadServer;
    Thread threadPlayer[playerCount];
    Mutex mutex;
    Cond wakeServer, wakeClient;

    mutex_init(&mutex, NULL);
    cond_init(&wakeServer, NULL);
    cond_init(&wakeClient, NULL);

    ThreadData thData = (ThreadData) {
            &playerData, false,
            {0, 0},
            {0, 0},
            svSockFD,
            &mutex, &wakeServer, &wakeClient
    };

    // Connecting with clients
    struct sockaddr_in clSockAddr[playerCount];
    socklen_t clSockLen[playerCount];

    for (int i = 0; i < playerCount; ++i) {
        clSockLen[i] = sizeof( clSockAddr[i] );
        thData.clSockFD[i] = accept( svSockFD, (struct sockaddr *) &clSockAddr[i], &clSockLen[i] );
        if (thData.clSockFD[i] < 0) {
            perror("Error on accept");
            return 6;
        }
        printf("Connected player %d\n", i + 1);
    }

    init(&playerData); // TODO

    // TODO neviem preco playerThread sa spusti skor nez gameThread
    thread_create(&threadServer, null, &gameThread, &thData);
    for (int i = 0; i < playerCount; ++i) {
        thread_create(&threadPlayer[i], null, &playerThread, &thData);
    }

    // Program is running

    thread_join(threadServer, NULL);
    for (int i = 0; i < playerCount; ++i) {
        printf("Kicking %d\n", i);
        thread_join(threadPlayer[i], NULL);
    }

    mutex_destroy(&mutex);
    cond_destroy(&wakeClient);
    cond_destroy(&wakeServer);

    close(svSockFD);
    for (int i = 0; i < playerCount; ++i) {
        close(thData.clSockFD[i]);
    }
}
