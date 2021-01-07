//
// Created by Moyo on 1/4/2021.
//

#ifndef POS_SEMESTRAL_PROJECT_CLOVECELOGIC_H
#define POS_SEMESTRAL_PROJECT_CLOVECELOGIC_H

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

enum Player { PLAYER_1, PLAYER_2, PLAYER_3, PLAYER_4 };
enum Command { START_GAME, END_GAME, DICE_ROLL, SKIP_TURN, AVAILABLE_PAWNS, SELECTED_PAWN, CONFIRM };
enum PawnArea { AREA_START, AREA_END, AREA_GAME };
enum StartTileIndex { START_TILE_P1 = 33, START_TILE_P2 = 3, START_TILE_P3 = 13, START_TILE_P4 = 23};
const int MAX_PLAYER_COUNT = 4;
const int PAWN_COUNT = 4;
const int SPACING = 2;
const int SIZE = 11;
const int GAME_TILE_COUNT = 40;

typedef struct {
    enum Command code;
    size_t size;
} Descriptor;

/**
 * Stores the y, x coordinates
 */
typedef struct coordinates {
    int y;
    int x;
} Position;

/**
 * Pawn - Stores information about a pawn.
 * That includes: position, number of tiles travelled, its char representation
 */
typedef struct gamepawn {
    Position pos;
    int startIndex;
    int travelled;
    char symbol;
    bool isActive;
} Pawn;

/**
 * PlayerData
 */
typedef struct player_data {
    int count;
    enum Player activePlayer;
    int pawnsOnEnd[4];  // TODO calloc
    Pawn pawns[4][4];   // TODO calloc
} PlayerData;

/**
 * ThreadData
 */
typedef struct thread_data{
    PlayerData* players;
    bool end;

    int* clSockFD;
    int svSockFD;

    Mutex* mutex;
    Cond* wakeServer;
    Cond* wakeClient;
} ThreadData;

/**
 * Coordinates for each tile in the active gameThread area.
 * Tiles are indexed 0 -> 39 with 0 being the leftmost tile at the top moving to the right
 */
const Position gamePos[40] = {
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

/**
 * Coordinates for each player area in the gameThread for all players.
 * [playerIndex][areaIndex][tileIndex]
 * \playerIndex index of a player 0 -> 3
 * \areaIndex index of an area 0 -> 1 where 0 is startingArea and 1 is Home or ending area
 * \tileIndex index of a tile 0 -> 3, when addressing the ending area, the tile closest to the entrance from gameArea has index=0
 */
const Position playerPos[4][2][4] = {
        {
                { {0, 0}, {0, 1}, {1, 0}, {1, 1} },
                { {5, 1}, {5, 2}, {5, 3}, {5, 4} }
        },
        {
                { {0, 9}, {0, 10}, {1, 9}, {1, 10} },
                { {1, 5}, {2, 5}, {3, 5}, {4, 5} }
        },
        {
                { {9, 9}, {9, 10}, {10, 9}, {10, 10} },
                { {5, 9}, {5, 8}, {5, 7}, {5, 6} }
        },
        {
                { {9, 0}, {9, 1}, {10, 0}, {10, 1} },
                { {9, 5}, {8, 5}, {7, 5}, {6, 5} }
        }
};

/**
 * Array of Pawn*, where each index represents a single tile of game area.
 * The size of the array is GAME_TILE_COUNT
 */
Pawn* pawnsGameArea[GAME_TILE_COUNT] = {null};

/**
 * Array of Pawn*, where each index represents a single tile of end area.
 * pawnsEndArea[playerIndex][tileIndex]
 * The size of the array [playerCount][PAWN_COUNT]
 */
Pawn* pawnsEndArea[4][PAWN_COUNT] = { {null}, {null}, {null}, {null} };

/**
 * Array of Pawn*, where each index represents a single tile of start area.
 * pawnsStartArea[playerIndex][tileIndex]
 * The size of the array is [playerCount][PAWN_COUNT]
 */
Pawn* pawnsStartArea[4][PAWN_COUNT] = { {null}, {null}, {null}, {null} };

/**
 * Rolls a six-sided die
 * @return integer from 1 to 6
 */
int rollDie();

/**
 * Awaits a descriptor to arrive at the given socket file descriptor.
 * @param sockfd is a socket file descriptor, where the descriptor will arrive
 * @return true only when the descriptor arrived, and its code == CONFIRM
 */
bool awaitConfirmation(int sockfd);

/**
 * Moves given pawn, into the given area at the given tile index
 * @param pawn to move
 * @param player owner of the given pawn
 * @param area AREA_START, AREA_END, AREA_GAME
 * @param index tile index in the given area, where the pawn shall be moved
 */
void movePawn(Pawn *pawn, enum Player player, enum PawnArea area, int index);

/**
 * Checks if the given positions match
 * @return true if @code a.x == b.x && a.y == b.y
 */
bool positionEquals(Position a, Position b);

/**
 * Checks if the given pawn can advance game board by the given number of tiles
 * @param pawn
 * @param playerData
 * @param tileCount by how many tiles should the given pawn move
 * @return true only if the pawn can move the given number of tiles
 */
bool canPawnAdvance(Pawn pawn, PlayerData* playerData, int tileCount);

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
Pawn* checkForPawn(PlayerData* data, Position position);

/**
 * Write 'size' bytes of buffer to the active player using 'write()'.
 * @param data socket and active player will be retrieved from here
 * @param buffer what to send
 * @param size number of bytes to send from buffer
 * @return true only if the buffer was written successfully, false otherwise
 */
bool writeToActivePlayer(ThreadData* data, void* buffer, size_t size);

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
 * Checks the ending condition of 4 pawns in the end area
 * @param playerData
 * @return true, if there are 4 pawns in any of the players end areas
 */
bool checkPawns(PlayerData* playerData);

/**
 * Checks if any of the pawns in the given array are on their spawnpoint
 * @param pawns
 * @return true, if there is no pawn on the spawnpoint
 */
bool canSpawn(Pawn *pawns);

/**
 * Calculates index of the next position index of a given pawn, but does not actually modify its position
 * @param pawn to be 'moved'
 * @param player whose pawn is it
 * @param tileCount how many tiles should the pawn 'move'
 * @return
 */
int nextPositionIndex(Pawn pawn, enum Player player, int tileCount);

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
void pawnReturnHome(Pawn *pawn, PlayerData *data);

void startGame(ThreadData *data);
int gameLogic(PlayerData *gameData);
void sendDiceRoll(ThreadData *data, int rolledNum);
void sendSkipTurn(ThreadData *threadData, int die);
void sendChoice(ThreadData *data, Pawn *choices, int choiceCount);
char receiveChoice(ThreadData *data);
void* gameThread(void *args);
void* playerThread(void *args);
Pawn* resolvePawnMovement(ThreadData *data, int die);

#endif //POS_SEMESTRAL_PROJECT_CLOVECELOGIC_H
