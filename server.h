#ifndef POS_SEMESTRAL_PROJECT_SERVER_H
#define POS_SEMESTRAL_PROJECT_SERVER_H

#include "shared.h"
#include <stdbool.h>
#include <pthread.h>

#define null NULL
#define Mutex pthread_mutex_t
#define mutex_init pthread_mutex_init
#define mutex_lock pthread_mutex_lock
#define mutex_unlock pthread_mutex_unlock
#define mutex_destroy pthread_mutex_destroy
#define Cond pthread_cond_t
#define cond_init pthread_cond_init
#define cond_wait pthread_cond_wait
#define cond_signal pthread_cond_signal
#define cond_broadcast pthread_cond_broadcast
#define cond_destroy pthread_cond_destroy
#define Thread pthread_t
#define thread_create pthread_create
#define thread_join pthread_join

static enum Player playerCounter = PLAYER_1;

/**
 * ThreadData
 */
typedef struct thread_data {
    PlayerData* players;
    bool end;

    int* clSockFD;
    int svSockFD;

    Mutex* mutex;
    Cond* wakeServer;
    Cond* wakeClient;
} ThreadData;

/**
 * Array of Pawn*, where each index represents a single tile of end area.
 * pawnsEndArea[playerIndex][tileIndex]
 * The size of the array [playerCount][PAWN_COUNT]
 */
Pawn* pawnsEndArea[4][4] = { {null}, {null}, {null}, {null} };

/**
 * Array of Pawn*, where each index represents a single tile of start area.
 * pawnsStartArea[playerIndex][tileIndex]
 * The size of the array is [playerCount][PAWN_COUNT]
 */
Pawn* pawnsStartArea[4][4] = { {null}, {null}, {null}, {null} };

/**
 * Rolls a six-sided die
 * @return integer from 1 to 6
 */
int rollDie();

/**
 * Moves given pawn, into the given area at the given tile index
 * @param pawn to move
 * @param player owner of the given pawn
 * @param area AREA_START, AREA_END, AREA_GAME
 * @param index tile index in the given area, where the pawn shall be moved
 */
void movePawn(Pawn *pawn, enum Player player, enum PawnArea area, int index);

/**
 * Checks if the given pawn can advance game board by the given number of tiles
 * @param pawn
 * @param data
 * @param tileCount by how many tiles should the given pawn move
 * @return true only if the pawn can move the given number of tiles
 */
bool canPawnAdvance(Pawn pawn, PlayerData* data, int tileCount);

/**
 * Advance the pawn by the given amount of tiles.
 * @param pawn
 * @param data
 * @param tileCount by how many tiles should the given pawn move
 */
void advancePawn(Pawn *pawn, PlayerData* data, int tileCount);

/**
 * Checks if the given position is occupied by a pawn
 * @param data
 * @param position to check
 * @return a pawn with the given position, if not found returns null
 */
Pawn* isPawnOnPos(PlayerData* data, Position position);

/**
 * Write 'size' bytes of buffer to the active player using 'write()'.
 * @param data socket and active player will be retrieved from here
 * @param buffer what to send
 * @param size number of bytes to send from buffer
 * @return true only if the buffer was written successfully, false otherwise
 */
void writeToActivePlayer(ThreadData* data, void* buffer, size_t size);

/**
 * Initializes player data
 * @param data
 * @param playerCount
 */
void init(PlayerData *data, int playerCount);

/**
 * Changes the member activePlayer to the next player
 * @param playerData
 */
void nextPlayer(PlayerData* playerData);

/**
 * Changes the member activePlayer to the previous player
 * @param playerData
 */
void previousPlayer(PlayerData* playerData);

/**
 * Checks the ending condition of 4 pawns in the end area
 * @param playerData
 * @return true, if there are 4 pawns in any of the players end areas
 */
bool checkGameEnd(PlayerData* playerData);

/**
 * Checks if any of the pawns in the given array are on their spawnpoint
 * @param pawns
 * @return true, if there is no pawn on the spawnpoint
 */
bool canSpawn(Pawn *pawns);

/**
 * Calculates index of the next position of a given pawn, but does not actually modify its position
 * @param pawn to be 'moved'
 * @param player whose pawn is it
 * @param tileCount how many tiles should the pawn 'move'
 * @return index of the next position, -1 if pawn will move into the players end area
 */
int nextPositionIndex(Pawn pawn, int tileCount);

/**
 * Spawns given pawn onto the game area
 * @param pawn to be spawned
 * @param playerData
 */
void spawnPawn(Pawn *pawn, PlayerData *playerData);

/**
 * Executes the pawn action - spawns pawn or advances its position.
 * Action is determined based on pawn.isActive
 * @param pawn
 * @param data
 * @param rolledNum
 */
void actOnPawn(Pawn *pawn, PlayerData *data, int rolledNum);

/**
 * Returns given pawn home to the start area
 * @param pawn
 * @param data
 */
void pawnReturnHome(Pawn *pawn);

void sendGameStart(ThreadData *data);
void sendDiceRoll(ThreadData *data, int rolledNum);
void sendSkipTurn(ThreadData *threadData);
void sendChoice(ThreadData *data, Pawn *choices, int choiceCount);
char receiveChoice(ThreadData *data);
Pawn* getPawnChoice(ThreadData *data, int die);
void sendRedraw(ThreadData *data);
void* gameThread(void *args);
void* playerThread(void *args);

#endif //POS_SEMESTRAL_PROJECT_SERVER_H
