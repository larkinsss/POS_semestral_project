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
enum Command { START_GAME, END_GAME, DICE_ROLL, SKIP_TURN, AVAILABLE_PAWNS, SELECTED_PAWN };
enum PawnPosition { START, GAME_AREA, END };
const int MAX_PLAYER_COUNT = 4;
const int PAWN_COUNT = 4;
const int SPACING = 2;
const int SIZE = 11;

/**
 * Stores the y, x coordinates
 */
typedef struct coordinates {
    int y;
    int x;
} Position;

/**
 * Stores information about a pawn.
 * That includes: position, number of tiles travelled, its char representation
 */
typedef struct gamepawn {
    Position pos;
    int travelled;
    char symbol;
    bool isActive;
} Pawn;

/**
 * Player Data
 */
typedef struct playerData {
    int count;
    enum Player activePlayer;
    int pawnsOnEnd[4];
    Pawn player[4][4];
} PlayerData;

typedef struct {
    PlayerData* players;
    bool end;

    socklen_t clSockLen[2]; // TODO
    int clSockFD[2];
    int svSockFD;

    Mutex* mutex;
    Cond* wakeServer;
    Cond* wakeClient;
} ThreadData;

typedef struct {

} GameData;

typedef struct {
    PlayerData playerData;
    int playerId;
    volatile bool endGame;
    int whosTurn;
    char *option;
    int numberOfPlayers;
} OldgameData;

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
 * \tileIndex index of a tile 0 -> 3, when addressing the ending area, the tile closest to the gameArea has index=0
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

Pawn* pawnsGameArea[40];
Pawn* pawnsEndArea[4][4];

void init(PlayerData *data);
void startGame(PlayerData *data);
int gameLogic(PlayerData *gameData);
void draw();
void* gameThread(void *args);
void* playerThread(void *args);

#endif //POS_SEMESTRAL_PROJECT_CLOVECELOGIC_H
