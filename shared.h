#ifndef POS_SEMESTRAL_PROJECT_SHARED_H
#define POS_SEMESTRAL_PROJECT_SHARED_H

#include <stdbool.h>
#include <stddef.h>

enum Player { PLAYER_1, PLAYER_2, PLAYER_3, PLAYER_4 };
enum Command { START_GAME, END_GAME, DICE_ROLL, SKIP_TURN, AVAILABLE_PAWNS, SELECTED_PAWN, CONFIRM, REDRAW};
enum PawnArea { AREA_START, AREA_END, AREA_GAME };
enum StartTileIndex { START_TILE_P1 = 33, START_TILE_P2 = 3, START_TILE_P3 = 13, START_TILE_P4 = 23};
const int MAX_PLAYER_COUNT = 4;
const int PAWN_COUNT = 4;
const int SPACING = 2;
const int SIZE = 11;
const int GAME_TILE_COUNT = 40;

/**
 * Descriptor - describes the type of next network write
 */
typedef struct descriptor {
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

typedef struct choice {
    char choice;
} PlayerChoice;

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
 * Checks if the given positions match
 * @return true if @code a.x == b.x && a.y == b.y
 */
bool positionEquals(Position a, Position b);

#endif //POS_SEMESTRAL_PROJECT_SHARED_H
