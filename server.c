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

int rollDie()
{
    // Hello
    return 1 + rand() % 6;
}

void gameInit(PlayerData *data, int playerCount)
{
    srand(time(NULL));
    *data = (PlayerData) {
            playerCount, -1,
            {
                    {
                            (Pawn) {playerPos[0][0][0], PLAYER_1, 0, '1', false},
                            (Pawn) {playerPos[0][0][1], PLAYER_1, 0, '2', false},
                            (Pawn) {playerPos[0][0][2], PLAYER_1, 0, '3', false},
                            (Pawn) {playerPos[0][0][3], PLAYER_1, 0, '4', false}
                    },
                    {
                            (Pawn) {playerPos[1][0][0], PLAYER_2, 0, '1', false},
                            (Pawn) {playerPos[1][0][1], PLAYER_2, 0, '2', false},
                            (Pawn) {playerPos[1][0][2], PLAYER_2, 0, '3', false},
                            (Pawn) {playerPos[1][0][3], PLAYER_2, 0, '4', false}
                    },
                    {
                            (Pawn) {playerPos[2][0][0], PLAYER_3, 0, '1', false},
                            (Pawn) {playerPos[2][0][1], PLAYER_3, 0, '2', false},
                            (Pawn) {playerPos[2][0][2], PLAYER_3, 0, '3', false},
                            (Pawn) {playerPos[2][0][3], PLAYER_3, 0, '4', false}
                    },
                    {
                            (Pawn) {playerPos[3][0][0], PLAYER_4, 0, '1', false},
                            (Pawn) {playerPos[3][0][1], PLAYER_4, 0, '2', false},
                            (Pawn) {playerPos[3][0][2], PLAYER_4, 0, '3', false},
                            (Pawn) {playerPos[3][0][3], PLAYER_4, 0, '4', false}
                    }
            }
    };

    for (int player = 0; player < playerCount; ++player) {
        for (int pawn = 0; pawn < PAWN_COUNT; ++pawn) {
            pawnsStartArea[player][pawn] = &data->pawns[player][pawn];
        }
    }
}

void nextPlayer(PlayerData* playerData)
{
    playerData->activePlayer = (playerData->activePlayer + 1) % playerData->count;
}

// TODO remove
void previousPlayer(PlayerData* playerData)
{
    playerData->activePlayer = (playerData->activePlayer + playerData->count - 1) % playerData->count;
}

void writeToActivePlayer(ThreadData* data, void* buffer, size_t size)
{
    int n = write(data->clSockFD[data->players->activePlayer], buffer, size);
    if (n < 0) {
        perror("Error writing to socket in method writeToActivePlayer\n");
    }
}

bool checkGameEnd(PlayerData* playerData)
{
    bool allPawnsOnEnd;

    // Go through each players end area
    for (int player = 0; player < playerData->count; ++player) {
        allPawnsOnEnd = true;

        // Check each end tile
        for (int pawn = 0; pawn < PAWN_COUNT; ++pawn) {

            // If a pawn is missing from end area - false
            if (pawnsEndArea[player][pawn] == null) {
                allPawnsOnEnd = false;
                break;
            }
        }

        // If all pawns were on end tiles
        if (allPawnsOnEnd) {
            return true;
        }
    }

    // No player has all his pawns on end tiles
    return false;
}

void sendDiceRoll(ThreadData *data, int rolledNum)
{
    Descriptor descriptor;
    memset(&descriptor, 0, sizeof(Descriptor));
    descriptor = (Descriptor) { DICE_ROLL, sizeof(int) };

    writeToActivePlayer(data, &descriptor, sizeof(Descriptor));

    int num;
    memset(&num, 0, sizeof(int));
    num = rolledNum;
    writeToActivePlayer(data, &num, descriptor.size);

    printf("Sent to Player %d, rolled %d\n", data->players->activePlayer, rolledNum);
    //sleep(1); // TODO removing sometimes breaks it
}

void sendChoice(ThreadData *data, Pawn *choices, int choiceCount)
{
    Descriptor descriptor;
    memset(&descriptor, 0, sizeof(Descriptor));
    descriptor = (Descriptor) {AVAILABLE_PAWNS, choiceCount * sizeof(Pawn)};

    writeToActivePlayer(data, &descriptor, sizeof(Descriptor));

    writeToActivePlayer(data, choices, descriptor.size);
}

void sendSkipTurn(ThreadData *threadData)
{
    Descriptor descriptor;
    memset(&descriptor, 0, sizeof(Descriptor));
    descriptor = (Descriptor) {SKIP_TURN, 0};

    writeToActivePlayer(threadData, &descriptor, sizeof(Descriptor));

    //sleep(1); // TODO removing sometimes breaks it
}

void sendGameStart(ThreadData *data)
{
    int n;
    Descriptor descriptor;
    memset(&descriptor, 0, sizeof(Descriptor));
    descriptor = (Descriptor) {START_GAME, sizeof(enum Player)};

    for (enum Player i = PLAYER_1; i < data->players->count; ++i) {

        n = write(data->clSockFD[i], &descriptor, sizeof(Descriptor));
        if (n < 0){
            perror("Error writing descriptor on sendGameStart(ThreadData *data)");
        }

        n = write(data->clSockFD[i], &i, descriptor.size);
        if (n < 0) {
            perror("Error sending data on sendGameStart(ThreadData *data)");
        }
    }
}

void sendGameEnd(ThreadData *data, enum Player winner)
{
    int n;
    int win = winner;   // Didn't work with sending just 'winner'
    Descriptor descriptor;
    memset(&descriptor, 0, sizeof(Descriptor));
    descriptor = (Descriptor) { END_GAME, sizeof(win) };

    for (int i = 0; i < data->players->count; ++i) {
        n = write(data->clSockFD[i], &descriptor, sizeof(Descriptor));
        if (n < 0) {
            perror("Error sending descriptor in sendGameEnd(ThreadData *data)\n");
        }

        n = write(data->clSockFD[i], &win, descriptor.size);
        if (n < 0) {
            perror("Error sending message in sendGameEnd(ThreadData *data)\n");
        }
    }
}

void sendRedraw(ThreadData *data)
{
    int n;
    Descriptor descriptor;
    memset(&descriptor, 0, sizeof(Descriptor));
    descriptor = (Descriptor) {REDRAW, sizeof(PlayerData)};

    for (int i = 0; i < data->players->count; ++i) {
        n = write(data->clSockFD[i], &descriptor, sizeof(Descriptor));
        if (n < 0) {
            perror("Error sending descriptor on redraw in method sendRedraw(ThreadData *data)");
        }

        n = write(data->clSockFD[i], data->players, descriptor.size);
        if (n < 0) {
            perror("Error sending player data on redraw in method sendRedraw(ThreadData *data)");
        }
    }
}

bool canPawnAdvance(Pawn pawn, PlayerData* data, int tileCount)
{
    // When the pawn is on home tile
    if (!pawn.isActive) {
        return false;
    }

    pawn.travelled += tileCount;

    if (pawn.travelled >= GAME_TILE_COUNT) {
        // If pawn made a whole round and is heading to end area
        pawn.pos = playerPos[data->activePlayer][1][pawn.travelled % GAME_TILE_COUNT];
    } else {
        // If pawn is progressing through game area
        pawn.pos = gamePos[(startPosIndex[pawn.player] + pawn.travelled) % GAME_TILE_COUNT];
    }

    // If the pawn has already made a round and is going to the end area
    if (pawn.travelled >= GAME_TILE_COUNT) {

        // If he rolled too high of a number to get into the end area
        if (pawn.travelled % GAME_TILE_COUNT >= PAWN_COUNT) {
            printf("Pawn %c rolled too high\n", pawn.symbol);   // TODO remove
            return false;
        }
    }

    // Makes sure that the pawn doesn't jump on a pawn of the same player
    for (int i = 0; i < PAWN_COUNT; ++i) {
        if (positionEquals(data->pawns[data->activePlayer][i].pos, pawn.pos)) {
            printf("Pawn %c would jump on Pawn %c\n", pawn.symbol, data->pawns[data->activePlayer][i].symbol);  // TODO remove
            return false;
        }
    }

    return true;
}

int nextPositionIndex(Pawn pawn, int tileCount)
{
    pawn.travelled += tileCount;

    if (pawn.travelled >= GAME_TILE_COUNT) {
        // If pawn is going to the end area
        return -1;
    } else {
        return (startPosIndex[pawn.player] + pawn.travelled) % GAME_TILE_COUNT;
    }
}

void advancePawn(Pawn *pawn, PlayerData* data, int tileCount)
{
    pawn->travelled += tileCount;

    if (pawn->travelled >= GAME_TILE_COUNT) {
        // If pawn made a whole round and is heading to end area
        pawn->pos = playerPos[data->activePlayer][1][pawn->travelled % GAME_TILE_COUNT];

        // Check if the pawn wasn't already in the end area
        for (int i = 0; i < PAWN_COUNT; ++i) {
            if (pawnsEndArea[data->activePlayer][i] == pawn) {
                // If he was - remove him from old position
                pawnsEndArea[data->activePlayer][i] = null;
                break;
            }
        }
        // Add pawn to a position in end area
        pawnsEndArea[data->activePlayer][pawn->travelled % GAME_TILE_COUNT] = pawn;
    } else {
        // If pawn is progressing through game area
        pawn->pos = gamePos[(startPosIndex[pawn->player] + pawn->travelled) % GAME_TILE_COUNT];
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
    movePawn(pawn, playerData->activePlayer, AREA_GAME, startPosIndex[pawn->player]);
    pawnsStartArea[playerData->activePlayer][pawn->symbol - '1'] = null;
    pawn->isActive = true;
}

bool canSpawn(Pawn *pawns)
{
    for (int i = 0; i < PAWN_COUNT; ++i) {
        if (pawns[i].isActive && pawns[i].travelled == 0) {
            return false;
        }
    }
    return true;
}

Pawn* handlePawnChoice(ThreadData *data, int die)
{
    enum Player player = data->players->activePlayer;

    // First calculate which pawns can the player choose
    Pawn* choices[4] = { null };
    int count = 0;

    if (die == 6) {

        // Spawning from home to start
        if (canSpawn(data->players->pawns[player])) {

            // If there is a pawn on a home tile
            for (int i = 0; i < PAWN_COUNT; ++i) {
                if (pawnsStartArea[player][i] != null) {
                    choices[count++] = pawnsStartArea[player][i];
                }
            }
        }
    }

    // If there is at least one pawn that is not on a home tile
    if (count != 4) {

        // Check each pawn, if he can advance the given number of tiles
        for (int i = 0; i < PAWN_COUNT; ++i) {
            if (canPawnAdvance(data->players->pawns[player][i], data->players, die)) {
                choices[count++] = &data->players->pawns[player][i];
            }
        }
    }

    // Second send choices
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

    // Return the chosen pawn*
    for (int i = 0; i < count; ++i) {
        if (choices[i]->symbol == choice) {
            return choices[i];
        }
    }
}

char receiveChoice(ThreadData *data)
{
    int n;
    char choice = 'X';

    printf("Waiting for choice from player %d\n", data->players->activePlayer);
    n = read(data->clSockFD[data->players->activePlayer], &choice, sizeof(choice));
    if (n < 0) {
        perror("Error reading choice from player in method receiveChoice(ThreadData *data)");
    }
    return choice;
}

Pawn* isPawnOnPos(PlayerData* data, Position position)
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

void pawnReturnHome(Pawn *pawn)
{
    movePawn(pawn, pawn->player, AREA_START, pawn->symbol - '1');
    pawnsStartArea[pawn->player][pawn->symbol - '1'] = pawn;
    pawn->isActive = false;
    pawn->travelled = 0;
}

void actOnPawn(Pawn *pawn, PlayerData *data, int rolledNum)
{
    Pawn* kickedPawn = null;

    if (pawn->isActive) {
        // Pawn is in the game area
        int nextIndex = nextPositionIndex(*pawn, rolledNum);
        if (nextIndex > 0) {
            kickedPawn = isPawnOnPos(data, gamePos[nextIndex]);
        }
        advancePawn(pawn, data, rolledNum);
    } else {
        // Pawn is home - to spawn
        kickedPawn = isPawnOnPos(data, gamePos[startPosIndex[pawn->player]]);
        spawnPawn(pawn, data);
    }

    if (kickedPawn != null) {
        printf("%c Kicking pawn %c [%d]\n", pawn->symbol, kickedPawn->symbol, startPosIndex[pawn->player] + kickedPawn->travelled);  // TODO remove
        pawnReturnHome(kickedPawn);
    }
}

void* gameThread(void *args)
{
    ThreadData *data = (ThreadData *) args;
    printf("GameThread start\n");

    mutex_lock(data->mutex);

    // First time gameInit
    sendGameStart(data);
    sendRedraw(data);
    data->players->activePlayer = PLAYER_1;

    cond_broadcast(data->wakeClient);
    mutex_unlock(data->mutex);

    while (!checkGameEnd(data->players)) {
        mutex_lock(data->mutex);

        cond_wait(data->wakeServer, data->mutex);

        // If player rolled a six, he goes again
        if (data->lastRolledNum != 6 && !checkGameEnd(data->players))
        {
            nextPlayer(data->players);
        }

        cond_broadcast(data->wakeClient);
        mutex_unlock(data->mutex);
    }

    sendGameEnd(data, data->players->activePlayer);
    
    mutex_lock(data->mutex);
    data->gameEnd = true;
    cond_broadcast(data->wakeClient);
    mutex_unlock(data->mutex);

    printf("GameThread end\n");
    return null;
}

void* playerThread(void *args)
{
    ThreadData *data = (ThreadData *) args;

    mutex_lock(data->mutex);
    enum Player id = data->playerCounter++;
    mutex_unlock(data->mutex);

    printf("PlayerTread %d start!\n", id);
    bool goToSleep = false;
    Pawn* chosenPawn;

    while (!data->gameEnd) {
        mutex_lock(data->mutex);

        while(goToSleep || data->players->activePlayer != id) {
            cond_wait(data->wakeClient, data->mutex);
            goToSleep = false;

            // If the game ended, stop this thread
            if (data->gameEnd) {
                cond_signal(data->wakeServer);
                mutex_unlock(data->mutex);
                return null;
            }
        }

        data->lastRolledNum = rollDie();
        sendDiceRoll(data, data->lastRolledNum);
        chosenPawn = handlePawnChoice(data, data->lastRolledNum);

        if (chosenPawn == null) {
            sendSkipTurn(data);
        } else {
            actOnPawn(chosenPawn, data->players, data->lastRolledNum);
            sendRedraw(data);
        }

        goToSleep = true;
        printf("PlayerThread %d sleep\n", id);
        cond_signal(data->wakeServer);
        mutex_unlock(data->mutex);
    }

    printf("PlayerThread %d end\n", id);
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
    if (port <= 1024 || port >= 65535) {
        fprintf(stderr, "Port must be > 1024 and < 65535\n");
        return 2;
    }

    int playerCount = atoi(argv[2]);
    if (playerCount < 1 || playerCount > 4) { // TODO
        fprintf(stderr, "Nubmer of players must be 2, 3 or 4\n");
        return 3;
    }

    // Variable gameInit
    PlayerData playerData;
    memset(&playerData, 0, sizeof(playerData));
    int svSockFD;
    struct sockaddr_in serv_addr;

    // Server address gameInit
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // Server socket gameInit
    svSockFD = socket(AF_INET, SOCK_STREAM, 0);
    if (svSockFD < 0) {
        perror("Error creating socket");
        return 4;
    }

    if (bind(svSockFD, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error binding socket address");

        port++;
        serv_addr.sin_port = htons(port);
        printf("New port: %d\n", port);

        if (bind(svSockFD, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            return 5;
        }
    }

    listen(svSockFD, playerCount);

    // Thread gameInit
    Thread threadServer;
    Thread threadPlayer[playerCount];
    Mutex mutex;
    Cond wakeServer, wakeClient;

    mutex_init(&mutex, NULL);
    cond_init(&wakeServer, NULL);
    cond_init(&wakeClient, NULL);

    int* clSockFD = (int*) calloc(playerCount, sizeof(int));

    ThreadData thData = (ThreadData) {
            &playerData,
            false,
            0,
            PLAYER_1,
            clSockFD,
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


    gameInit(&playerData, playerCount);

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
