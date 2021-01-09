#ifndef POS_SEMESTRAL_PROJECT_CLIENT_H
#define POS_SEMESTRAL_PROJECT_CLIENT_H

#include "shared.h"
#include <stdbool.h>

const int SPACING = 2;

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
 * Draws board with changes
 * @param data
 */
void redrawBoard(int sockfd, size_t size);

/**
 *
 */
void gameLogic(int sockfd);

/**
 * Method which handles dice roll input from server.
 * Prints out rolled dice.
 */
void handleDiceRoll(int sockfd, size_t size);

void handleSkipTurn();

void handlePawns(int sockfd, size_t size);

void clearLine(int x, int y);

void clearPrintw(int x, int y, const char* str);

bool receive(int socketfd, void* buffer, size_t size);

void clearLines(int x, int y, int count);

void handleEndGame(int sockfd, size_t size, enum Player id);

#endif //POS_SEMESTRAL_PROJECT_CLIENT_H
