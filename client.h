#ifndef POS_SEMESTRAL_PROJECT_CLIENT_H
#define POS_SEMESTRAL_PROJECT_CLIENT_H

#include <stdbool.h>
#include "shared.h"

typedef struct gameData {
    PlayerData *playerData;
    int playerId;
    volatile bool endGame;
    int whosTurn;
    int numberOfPlayers;
} GAME_DATA;

/**
 * Prints given string at the given position using curses.h
 * Automatically adds spacing to the x-axis
 * @param position where should the string be printed
 * @param string what should be printed
 * @return upon successful completion, returns OK. Otherwise, returns ERR
 */
int movePrintSpacing(Position position, const char* string);

/**
 * Gives color pair number for the given players.
 * @param player
 * @return color pair number
 */
int getColor(enum Player player);

/**
 * Gives color pair number for the given players.
 * @param player number of the players [0 -> 3]
 * @return color pair number
 */
int colorFromPlayerNum(int player);

/**
 * Draws the initial game board.
 */
void drawBoard();

/**
 * Draws board with changes
 * @param data
 */
void redrawBoard(Descriptor descriptor, int sockfd);

/**
 *
 */
void gameLogic(Descriptor descriptor, int sockfd);

/**
 * Method which handles dice roll input from server.
 * Prints out rolled dice.
 */
void handleDiceRoll(Descriptor descriptor, int sockfd);

void handleSkipTurn(Descriptor descriptor, int sockfd);

void handlePawns(Descriptor descriptor, int sockfd);

void clearLine(int x, int y);

#endif //POS_SEMESTRAL_PROJECT_CLIENT_H
