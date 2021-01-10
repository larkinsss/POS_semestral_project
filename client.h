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
 *  Handles receiving of Descriptors and runs a method specified to handling received codes
 */
void gameLogic(int sockfd);

/**
 * Method which handles dice roll input from server.
 * Prints out rolled dice.
 */
void handleDiceRoll(int sockfd, size_t size);

/**
 * Handles the skip turn code
 */
void handleSkipTurn();

/**
 * Handles received choices and sends user selected pawn
 */
void handlePawns(int sockfd, size_t size);

/**
 * Clears a line from given coordinates until eol
 */
void clearLine(int x, int y);

/**
 * Reads from the given socketfd into the given buffer size-count of bytes
 */
void receive(int socketfd, void* buffer, size_t size);

/**
 * Clears the given number of lines from the given coordinates
 * @param count how many lines to clear
 */
void clearLines(int x, int y, int count);

/**
 * Handles end game code and displays the winner
 * @param id Player ID
 */
void handleEndGame(int sockfd, size_t size, enum Player id);

#endif //POS_SEMESTRAL_PROJECT_CLIENT_H
