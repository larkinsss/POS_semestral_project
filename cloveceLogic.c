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

static enum Player p = PLAYER_1;

bool writeToActivePlayer(ThreadData* data, void* buffer, size_t size)
{
    if ( write(data->clSockFD[data->players->activePlayer], buffer, size) ) {
        perror("Error writing to socket in method writeToActivePlayer(ThreadData, void*, size_t)\n");
        return false;
    }
    return true;
}

void init(PlayerData *data, int playerCount)
{
    srand(time(NULL));
    *data = (PlayerData) {
        playerCount, -1, // TODO
        { 0, 0, 0, 0 },
        {
            {
                (Pawn) {playerPos[0][0][0], START_TILE_P1, 0, '1', false},
                (Pawn) {playerPos[0][0][1], START_TILE_P1, 0, '2', false},
                (Pawn) {playerPos[0][0][2], START_TILE_P1, 0, '3', false},
                (Pawn) {playerPos[0][0][3], START_TILE_P1, 0, '4', false}
            },
            {
                (Pawn) {playerPos[1][0][0], START_TILE_P2, 0, '1', false},
                (Pawn) {playerPos[1][0][1], START_TILE_P2, 0, '2', false},
                (Pawn) {playerPos[1][0][2], START_TILE_P2, 0, '3', false},
                (Pawn) {playerPos[1][0][3], START_TILE_P2, 0, '4', false}
            },
            {
                (Pawn) {playerPos[2][0][0], START_TILE_P3, 0, '1', false},
                (Pawn) {playerPos[2][0][1], START_TILE_P3, 0, '2', false},
                (Pawn) {playerPos[2][0][2], START_TILE_P3, 0, '3', false},
                (Pawn) {playerPos[2][0][3], START_TILE_P3, 0, '4', false}
            },
            {
                (Pawn) {playerPos[3][0][0], START_TILE_P4, 0, '1', false},
                (Pawn) {playerPos[3][0][1], START_TILE_P4, 0, '2', false},
                (Pawn) {playerPos[3][0][2], START_TILE_P4, 0, '3', false},
                (Pawn) {playerPos[3][0][3], START_TILE_P4, 0, '4', false}
            }
        }
    };
}

void startGame(ThreadData *data) {
    int n;
    Descriptor descriptor = {START_GAME, 255};
    for (int i = 0; i < data->players->count; ++i) {
        n = write(data->clSockFD[i], &descriptor, sizeof(Descriptor));
        if (n < 0){
            perror("Writing to socket on START_GAME went wrong. \n");
        }
    }
}

int rollDie() {
    return rand() % 6 + 1;
}

void nextPlayer(PlayerData* playerData) {
    printf("Player change from %d", playerData->activePlayer);
    playerData->activePlayer = (playerData->activePlayer + 1) % playerData->count;
    printf(" to %d\n", playerData->activePlayer);
}

bool checkPawns(PlayerData* playerData)
{
    for (int i = 0; i < playerData->count; ++i) {
        if (playerData->pawnsOnEnd[i] >= playerData->count * 2) // TODO change data->count * 2
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

    // Sending the type of data
    Descriptor descriptor = {DICE_ROLL, 255};

    writeToActivePlayer(data, &descriptor, sizeof(Descriptor));

    bzero(msg, 255);
    sprintf(msg, "You #%d rolled a %d", data->players->activePlayer, die);
    //sleep(1);

    writeToActivePlayer(data, msg, strlen(msg));

    printf("Sent to Player %d, rolled %d\n", data->players->activePlayer, die);
    sleep(1);
    resolvePawnMovement(data, die);
}

bool awaitConfirmation(int sockfd)
{
    Descriptor descriptor = {0 , 0};

    if (read(sockfd, &descriptor, sizeof(Descriptor)) < 0) {
        perror("Error reading from socket on awaitConfirmation()\n");
        return false;
    }

    return descriptor.code == CONFIRM;
}

bool positionEquals(Position a, Position b)
{
    return a.x == b.x && a.y == b.y;
}

void skipTurn(ThreadData *threadData, int die) {
    int n;
    char msg[255];
    Descriptor descriptor = {SKIP_TURN, 255};

    writeToActivePlayer(threadData, &descriptor, sizeof(Descriptor));

    bzero(msg, 255);
    sprintf(msg, "You rolled a %d but you cannot move.", die);
    sleep(1);

    writeToActivePlayer(threadData, msg, strlen(msg));
    sleep(1);
}

bool canPawnAdvance(Pawn pawn, PlayerData* playerData, int tileCount)
{
    //Pawn pwn = *pawn;
    pawn.travelled += tileCount;

    // If the pawn has already made a round and is going to the end area
    if (pawn.travelled >= GAME_TILE_COUNT) {

        // If he rolled too high of a number to get into the end area
        if (pawn.travelled % GAME_TILE_COUNT >= PAWN_COUNT) {
            return false;
        } else {
            // If he can get into the end area -- update his position
            pawn.pos = playerPos[playerData->activePlayer][1][pawn.travelled % GAME_TILE_COUNT];
        }
    } else {
        // If the pawn stays in the game area, update his position
        pawn.pos = gamePos[pawn.startIndex + pawn.travelled % GAME_TILE_COUNT];
    }

    // Makes sure that the pawn doesn't jump on a pawn of the same player
    for (int i = 0; i < PAWN_COUNT; ++i) {
        if (positionEquals(playerData->pawns[playerData->activePlayer][i].pos, pawn.pos)) {
            return false;
        }
    }

    return true;
}

void advancePawn(Pawn *pawn, PlayerData* data, int tileCount)
{
    pawn->travelled += tileCount;

    if (pawn->travelled >= GAME_TILE_COUNT) {
        pawn->pos = playerPos[data->activePlayer][1][pawn->travelled % GAME_TILE_COUNT];
    } else {
        pawn->pos = gamePos[pawn->startIndex + pawn->travelled % GAME_TILE_COUNT];
    }
}

void movePawn(Pawn *pawn, enum Player player, enum PawnArea area, int index)
{
    if (area == AREA_GAME) {
        pawn->pos = gamePos[index % GAME_TILE_COUNT];
    } else {
        pawn->pos = playerPos[player][area][index];
    }
}

//TODO change name
void spawnPawn(PlayerData *playerData, int pawn)
{
    if (false) {

    }
}

bool checkCanPawnSpawn(Pawn *pawnField)
{
    for (int i = 0; i < PAWN_COUNT; ++i) {
        if (positionEquals(gamePos[pawnField[i].startIndex], pawnField[i].pos)) {                                       // Kontroluje vsetky spawn pointy, nie len ten, na ktory sa spawne dany hrac
            return false;
        }
    }
    return true;
}

void resolvePawnMovement(ThreadData *threadData, int die)                                                               // TODO too complicated of a method
{
    PlayerData *playerData = threadData->players;
    //TODO: Maybe adresses
    Pawn pawnField[4] = {playerData->pawns[playerData->activePlayer][0],
                         playerData->pawns[playerData->activePlayer][1],
                         playerData->pawns[playerData->activePlayer][2],
                         playerData->pawns[playerData->activePlayer][3]};


    int n = 0;
    bool noPawnsInField = true;                                                                                         // TODO negated name of var bool -> pawnsInField
    for (int i = 0; i < PAWN_COUNT; ++i) {
        if (!positionEquals(pawnField[i].pos,playerPos[playerData->activePlayer][0][i])){
            noPawnsInField = false;
        }
    }

    int moves = 0;
    Pawn possibleMoves[4] = {0};
    if (noPawnsInField) {
        //No pawns of field
        if(die == 6) {
            //Rolled 6 while no pawns on field, can select one pawn to put on field
            for (int i = 0; i < PAWN_COUNT; ++i) {
                possibleMoves[i] = pawnField[i];
            }
        } else {
            //Rolled other than 6 while no pawns on field, cannot move
            skipTurn(threadData, die);
        }
    } else {
        //Some pawns on field
        if (die == 6) {
            //Rolled 6 while some pawns on field
            for (int i = 0; i < PAWN_COUNT; ++i) {
                if(pawnField[i].isActive) {
                    //Checks if pawn on field can move 6
                    if (canPawnAdvance(pawnField[i],playerData,die)) {
                        possibleMoves[moves] = pawnField[i];
                        moves++;
                    }
                } else {
                    //Check if pawn that is not active is on start
                    if (positionEquals(pawnField[i].pos, gamePos[pawnField[i].startIndex])) {
                        if (checkCanPawnSpawn(pawnField)) {
                            possibleMoves[moves] = pawnField[i];
                            moves++;
                        }
                    }
                }
            }
        } else {
            //Some pawns on field and rolled under 6
            for (int i = 0; i < 4; ++i) {
                if (pawnField[i].isActive) {
                    if(canPawnAdvance(pawnField[i],playerData,die)) {
                        possibleMoves[moves] = pawnField[i];
                        moves++;
                    }
                }
            }
        }
    }
    //TODO: Send available moves
    Pawn* possibleMovesFinal = (Pawn *)calloc(moves,sizeof(Pawn));
    sendAvailableMoves(threadData, possibleMovesFinal,moves);
    free(possibleMovesFinal);
}

void sendAvailableMoves(ThreadData *threadData, Pawn *possibleMoves, int numberOfMoves) {
    int n;
    char msg[255];
    Descriptor descriptor = {AVAILABLE_PAWNS, numberOfMoves * sizeof(Pawn)};

    writeToActivePlayer(threadData, &descriptor, sizeof(Descriptor));

    sleep(1);

    writeToActivePlayer(threadData, possibleMoves, descriptor.size);
    sleep(1);

    char choice;
    n = read(threadData->svSockFD, &choice, sizeof(char));
    if (n < 0) {
        perror("Error writing to socket");
    }
    //TODO: Do choice

    int pawn = atoi(&choice)-1;
    spawnPawn(threadData->players, pawn);

}

Pawn* checkForPawn(PlayerData* data, Position position)
{
    Pawn* pawn;

    for (int player = 0; player < data->count; ++player) {
        for (int i = 0; i < PAWN_COUNT; ++i) {
            pawn = &data->pawns[player][i];
            if (positionEquals(pawn->pos, position)) {
                return pawn;
            }
        }
    }

    return null;
}

void* gameThread(void *args)
{
    ThreadData *data = (ThreadData *) args;
    mutex_lock(data->mutex);
    printf("Server init\n");
//    startGame(data);
//    sleep(5);

    int n = 0;
    int die = 0;
    char msg[256];

    // First time init
    data->players->activePlayer = PLAYER_1;
    mutex_unlock(data->mutex);
    cond_broadcast(data->wakeClient);

    for (int i = data->players->activePlayer; checkPawns(data->players); i = (i + 1) % data->players->count) {
        mutex_lock(data->mutex);

        while (i == data->players->activePlayer) {
            printf("Game thread sleep... i = %d, a = %d\n", i, data->players->activePlayer);
            cond_wait(data->wakeServer, data->mutex);
            nextPlayer(data->players);
        }
        printf("Game thread woken... i = %d, a = %d\n", i, data->players->activePlayer);

        // TODO check and/or update pawnsOnEnd

        mutex_unlock(data->mutex);
        cond_broadcast(data->wakeClient);
    }

    printf("GameThread end!\n");
    mutex_lock(data->mutex);
    data->end = true;
    mutex_unlock(data->mutex);
    cond_broadcast(data->wakeClient);

    return null;
}

void* playerThread(void *args)
{
    ThreadData *data = (ThreadData *) args;
    enum Player id = p++;
    printf("Player %d start!\n", id);
    bool goToSleep = false;

    while (!data->end) {
        mutex_lock(data->mutex);

        while(goToSleep || data->players->activePlayer != id) {
            cond_wait(data->wakeClient, data->mutex);
            if (data->end) {
                mutex_unlock(data->mutex);
                cond_signal(data->wakeServer);
                return null;
            }
            goToSleep = false;
        }

        sendDie(data);
        /*
         * TODO calculate if player can play a pawn
         * if he can then retreive all playable pawns
         *      send pawns to player and wait for his choice
         *      Play the chosen pawn, then update pawnsOnEnd if needed.
         * if he can't play a pawn, send CODE to skip the turn.
         */
        data->players->pawnsOnEnd[0] += 1; // TODO update pawnsOnEnd[id]

        goToSleep = true;
        mutex_unlock(data->mutex);
        cond_signal(data->wakeServer);
    }

    printf("Player %d end\n", id);
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
    if (playerCount < 1 || playerCount > 4) {
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

        port++;
        serv_addr.sin_port = htons(port);
        printf("New port: %d\n", port); // TODO

        if (bind(svSockFD, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            return 5;
        }
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

    int* clSockFD = (int*) calloc(playerCount, sizeof(int));

    ThreadData thData = (ThreadData) {
            &playerData, false,
            clSockFD,
            svSockFD,
            &mutex, &wakeServer, &wakeClient
    };

    // Connecting with clients
    struct sockaddr_in clSockAddr[playerCount];
    socklen_t clSockLen[playerCount];

    printf("Waiting for players...\n");
    for (int i = 0; i < playerCount; ++i) {
        clSockLen[i] = sizeof( clSockAddr[i] );
        thData.clSockFD[i] = accept( svSockFD, (struct sockaddr *) &clSockAddr[i], &clSockLen[i] );
        if (thData.clSockFD[i] < 0) {
            perror("Error on accept");
            return 6;
        }
        printf("Connected player %d\n", i + 1);
    }


    init(&playerData, playerCount);

    thread_create(&threadServer, null, &gameThread, &thData);
    for (int i = 0; i < playerCount; ++i) {
        thread_create(&threadPlayer[i], null, &playerThread, &thData);
    }

    // Program is running

    thread_join(threadServer, NULL);
    for (int i = 0; i < playerCount; ++i) {
        thread_join(threadPlayer[i], NULL);
    }

    // Closing
    free(clSockFD);

    mutex_destroy(&mutex);
    cond_destroy(&wakeClient);
    cond_destroy(&wakeServer);

    close(svSockFD);
    for (int i = 0; i < playerCount; ++i) {
        close(thData.clSockFD[i]);
    }
}
