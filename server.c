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
#include "server.h"
#include <pthread.h>

static enum Player p = PLAYER_1;

bool writeToActivePlayer(ThreadData* data, void* buffer, size_t size)
{
    int n;
    n = write(data->clSockFD[data->players->activePlayer], buffer, size);
    if (n < 0) {
        perror("Error writing to socket in method writeToActivePlayer\n");

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

    for (int player = 0; player < playerCount; ++player) {
        for (int pawn = 0; pawn < PAWN_COUNT; ++pawn) {
            pawnsStartArea[player][pawn] = &data->pawns[player][pawn];
        }
    }
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

void callRedraw(ThreadData *data) {
    int n;
    Descriptor descriptor = {REDRAW, sizeof(PlayerData)};
    for (int i = 0; i < data->players->count; ++i) {
        n = write(data->clSockFD[i], &descriptor, sizeof(Descriptor));
        if (n < 0) {
            perror("Error sending descriptor on redraw in method callRedraw(ThreadData *data)");
        }

        sleep(1);
        n = write(data->clSockFD[i], data->players, descriptor.size);
        if (n < 0) {
            perror("Error sending player data on redraw in method callRedraw(ThreadData *data)");
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

void sendDiceRoll(ThreadData *data, int rolledNum)
{
    char msg[256];

    // Sending the type of data
    Descriptor descriptor = {DICE_ROLL, 255};

    writeToActivePlayer(data, &descriptor, sizeof(Descriptor));

    bzero(msg, 255);
    sprintf(msg, "You #%d rolled a %d", data->players->activePlayer, rolledNum);
    sleep(1);

    writeToActivePlayer(data, msg, strlen(msg));

    printf("Sent to Player %d, rolled %d\n", data->players->activePlayer, rolledNum);
    sleep(1);
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

void sendSkipTurn(ThreadData *threadData, int die) {
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
    // When the pawn is on home tile
    if (!pawn.isActive) {
        return false;
    }

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

// TODO advancePawn <-> nextPositionIndex
int nextPositionIndex(Pawn pawn, enum Player player, int tileCount)
{
    pawn.travelled += tileCount;

    if (pawn.travelled >= GAME_TILE_COUNT) {
        return pawn.travelled % GAME_TILE_COUNT;
    } else {
        return pawn.startIndex + pawn.travelled % GAME_TILE_COUNT;
    }
}

// TODO nextPositionIndex <-> advancePawn
void advancePawn(Pawn *pawn, PlayerData* data, int tileCount)
{
    pawn->travelled += tileCount;

    if (pawn->travelled >= GAME_TILE_COUNT) {
        pawn->pos = playerPos[data->activePlayer][1][pawn->travelled % GAME_TILE_COUNT];
        data->pawnsOnEnd[data->activePlayer]++;
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

void spawnPawn(Pawn *pawn, PlayerData *playerData)
{
    movePawn(pawn, playerData->activePlayer, AREA_GAME, pawn->startIndex);
    pawnsStartArea[playerData->activePlayer][pawn->symbol - '1'] = null;
    pawn->isActive = true;
}

bool canSpawn(Pawn *pawns)
{
    for (int i = 0; i < PAWN_COUNT; ++i) {
        if (positionEquals(gamePos[pawns[i].startIndex], pawns[i].pos)) {   // TODO couldn't this use pawn.travelled == 0
            return false;
        }
    }
    return true;
}

Pawn* resolvePawnMovement(ThreadData *data, int die)
{
    PlayerData *playerData = data->players;
    enum Player player = data->players->activePlayer;
    Pawn* pwn = &data->players->pawns[player][0];

    Pawn* choices[4] = {0};
    int count = 0;

    if (die == 6) {

        // Spawning from home to start
        if (canSpawn(pwn)) {

            // If there is a pawn on a home tile
            for (int i = 0; i < PAWN_COUNT; ++i) {
                if (pawnsStartArea[player][i] != null) {
                    choices[count++] = pawnsStartArea[player][i];
                }
            }
        }
    }

    // Check each pawn, if he can advance the given number of tiles
    for (int i = 0; i < PAWN_COUNT; ++i) {
        if (canPawnAdvance(pwn[i], data->players, die)) {
            choices[count++] = &pwn[i];
        }
    }

    // If there are no choices return, player must skip a turn
    if (count == 0) {
        return null;
    }

    // Create array to send
    Pawn* pawnChoices = (Pawn *) calloc(count, sizeof(Pawn));
    for (int i = 0; i < count; ++i) {
        pawnChoices[i] = *choices[i];
    }

    // Send choice to player
    sendChoice(data, pawnChoices, count);
    free(pawnChoices);

    // Receive players choice
    char choice = receiveChoice(data);
    return choices[choice - '1'];
}

void sendChoice(ThreadData *data, Pawn *choices, int choiceCount)
{
    Descriptor descriptor = {AVAILABLE_PAWNS, choiceCount * sizeof(Pawn)};
    writeToActivePlayer(data, &descriptor, sizeof(Descriptor));

    sleep(1);

    writeToActivePlayer(data, choices, descriptor.size);
}

char receiveChoice(ThreadData *data)
{
    int n;
    PlayerChoice choice;

    n = read(data->clSockFD[data->players->activePlayer], &choice, sizeof(PlayerChoice));
    if (n < 0) {
        perror("Error reading choice from player in method receiveChoice(ThreadData *data)");
    }
    return choice.choice;
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

void pawnReturnHome(Pawn *pawn, PlayerData *data)
{
    switch (pawn->startIndex) { // TODO weak - make better
        case START_TILE_P1:
            movePawn(pawn, PLAYER_1, AREA_START, pawn->symbol - '1');
            pawnsStartArea[PLAYER_1][pawn->symbol - '1'] = pawn;
            break;
        case START_TILE_P2:
            movePawn(pawn, PLAYER_2, AREA_START, pawn->symbol - '1');
            pawnsStartArea[PLAYER_2][pawn->symbol - '1'] = pawn;
            break;
        case START_TILE_P3:
            movePawn(pawn, PLAYER_3, AREA_START, pawn->symbol - '1');
            pawnsStartArea[PLAYER_3][pawn->symbol - '1'] = pawn;
            break;
        default:
            movePawn(pawn, PLAYER_4, AREA_START, pawn->symbol - '1');
            pawnsStartArea[PLAYER_4][pawn->symbol - '1'] = pawn;
    }
    pawn->isActive = false;
}

void actOnPawn(Pawn *pawn, PlayerData *data, int rolledNum)
{
    Pawn* kickedPawn = null;

    // Pawn is in the game area
    if (pawn->isActive) {
        kickedPawn = checkForPawn(data, gamePos[ nextPositionIndex(*pawn, data->activePlayer, rolledNum) ]);
        advancePawn(pawn, data, rolledNum);
    } else {    // Pawn is home - to spawn
        kickedPawn = checkForPawn(data, pawn->pos);
        spawnPawn(pawn, data);
    }

    if (kickedPawn != null) {
        pawnReturnHome(kickedPawn, data);
    }
}

void* gameThread(void *args)
{
    ThreadData *data = (ThreadData *) args;
    mutex_lock(data->mutex);
    printf("Server init\n");
    startGame(data);
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

        callRedraw(data);

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
    int rolledNum = 0;

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

        rolledNum = rollDie();
        sendDiceRoll(data, rolledNum);

        Pawn* chosenPawn = resolvePawnMovement(data, rolledNum);

        if (chosenPawn == null) {
            sendSkipTurn(data, rolledNum);
        } else {
            actOnPawn(chosenPawn, data->players, rolledNum);
        }

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
