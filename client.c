#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>
#include "client.h"

bool receive(int socketfd, void* buffer, size_t size) {
    if (read(socketfd, buffer, size) < 0) {
        clearPrintw(0, 21, "Error on receive(int socketfd, void* buffer, size_t size)");
    }
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    if (argc < 3)
    {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        return 1;
    }

    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "Error, no such host\n");
        return 2;
    }

    bzero((char*)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(
            (char*)server->h_addr,
            (char*)&serv_addr.sin_addr.s_addr,
            server->h_length
    );
    serv_addr.sin_port = htons(atoi(argv[2]));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error creating socket");
        return 3;
    }

    if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error connecting to socket");
        return 4;
    }

    initscr();
    start_color();
    init_pair(1,COLOR_YELLOW, COLOR_BLACK);
    init_pair(2,COLOR_GREEN, COLOR_BLACK);
    init_pair(3,COLOR_CYAN, COLOR_BLACK);
    init_pair(4,COLOR_RED, COLOR_BLACK);

    /*
     * SEM PRIDE FUNKCIONALITA HRY
     */

    gameLogic(sockfd);

    /*
     * TU KONCI FUNKCIONALITA A ZATVARA SA SOCKET
     */
    endwin();
    close(sockfd);
    return 0;
}

void clearLine(int x, int y) {
    move(y,x);
    clrtoeol();
}

void clearLines(int x, int y, int count)
{
    for (int i = 0; i < count; ++i) {
        clearLine(x, y + i);
    }
}

void clearPrintw(int x, int y, const char* str)
{
    clearLine(x, y);
    mvprintw(y, x, str);
}

void handleEndGame(Descriptor descriptor, int sockfd, enum Player id)
{
    clearLines(0, 12, 9);

    int winner = -1;
    receive(sockfd, &winner, descriptor.size);

    if (id == winner) {
        mvprintw(12, 0, "Congratulations, you won!");
    } else {
        mvprintw(12, 0, "Game Over!");
    }

    attron(COLOR_PAIR(getColor(winner)));
    mvprintw(13, 0, "Player %d ", winner + 1);
    attroff(COLOR_PAIR(getColor(winner)));
    printw("won!");
    refresh();
}

enum Player handleId(Descriptor descriptor, int sockfd)
{
    enum Player id;
    receive(sockfd, &id, descriptor.size);

    mvprintw(12, 0, "You are ");
    attron(COLOR_PAIR(getColor(id)));
    printw("Player %d", id + 1);
    attroff(COLOR_PAIR(getColor(id)));

    return id;
}

void gameLogic(int sockfd) {
    int n;
    enum Player id = -1;
    Descriptor descriptor = {0,0};

    do {
        mvprintw(20, 0, "Waiting for descriptor");
        refresh();
        receive(sockfd, &descriptor, sizeof(Descriptor));
        clearLine(0, 20);
        refresh();

        switch (descriptor.code) {  // TODO Descriptor is copied each time
            case DICE_ROLL:
                handleDiceRoll(descriptor,sockfd);  // TODO Descriptor used only for size_t
                break;
            case SKIP_TURN:
                handleSkipTurn(descriptor,sockfd);
                break;
            case AVAILABLE_PAWNS:
                clearLine(0, 19);
                handlePawns(descriptor,sockfd);
                clearLine(0, 13);
                break;
            case REDRAW:
                redrawBoard(descriptor, sockfd);
                break;
            case END_GAME:
                handleEndGame(descriptor, sockfd, id);
                break;
            case START_GAME:
                id = handleId(descriptor, sockfd);
                break;
            default:
                clearLine(0,21);
                mvprintw(21, 0, "Descriptor method not recognized %d", descriptor.code);
        }
        refresh();
    } while (descriptor.code != END_GAME);

    getch();
}

void handleDiceRoll(Descriptor descriptor, int sockfd){
    int buffer = 0;
    receive(sockfd, &buffer, descriptor.size);

    char str[13] = { 0 };
    sprintf(str, "You rolled %d", buffer);
    clearPrintw(0, 13, str);

    refresh();
}

void handleSkipTurn(Descriptor descriptor, int sockfd){
    mvprintw(19, 0, "No possible moves - turn skipped");
    refresh();
    sleep(1);
}

void handlePawns(Descriptor descriptor, int sockfd) {
    Pawn *possibleMoves = (Pawn *)malloc(descriptor.size);
    int count = (int) (descriptor.size / sizeof(Pawn));

    receive(sockfd, possibleMoves, descriptor.size);

    mvprintw(15, 0, "You can move pawns: ");
    for (int i = 0; i < count; ++i) {
        printw("%c ", possibleMoves[i].symbol);
    }

    char choice;
    bool correctInput = false;

    while (!correctInput) {
        clearLine(0,14);
        mvprintw(14, 0, "Choose a pawn that you want to move: ");
        refresh();
        scanw("%c", &choice);
        for (int i = 0; i < count; ++i) {
            if (choice == possibleMoves[i].symbol) {
                correctInput = true;
                count = write(sockfd, &choice, sizeof(choice));
                if(count < 0) {
                    mvprintw(21,0,"Error writing choice to server");
                }
            }
        }
    }

    clearLines(0, 14, 5);
    free(possibleMoves);
    refresh();
}

void drawBoard()
{
    mvprintw(0, 0, "        . . .        \n"
           "        .   .        \n"
           "        .   .        \n"
           "        .   .        \n"
           ". . . . .   . . . . .\n"
           ".                   .\n"
           ". . . . .   . . . . .\n"
           "        .   .        \n"
           "        .   .        \n"
           "        .   .        \n"
           "        . . .        \n");

    for (int player = 0; player < MAX_PLAYER_COUNT; ++player) {
        attron(COLOR_PAIR(getColor(player)));

        // Printout pawns
        movePrintSpacing(playerPos[player][0][0], "1");
        movePrintSpacing(playerPos[player][0][1], "2");
        movePrintSpacing(playerPos[player][0][2], "3");
        movePrintSpacing(playerPos[player][0][3], "4");

        // Printout finish tiles
        for (int tile = 0; tile < PAWN_COUNT; ++tile) {
            movePrintSpacing(playerPos[player][1][tile], (const char *) (player % 2 == 0 ? "-" : "|"));
        }
        attroff(COLOR_PAIR(getColor(player)));
    }

    refresh();
}

void redrawBoard(Descriptor descriptor, int sockfd) {
    PlayerData *data = (PlayerData *)malloc(descriptor.size);
    receive(sockfd, data, descriptor.size);

    mvprintw(0, 0,
        "        . . .        \n"
        "        .   .        \n"
        "        .   .        \n"
        "        .   .        \n"
        ". . . . .   . . . . .\n"
        ".                   .\n"
        ". . . . .   . . . . .\n"
        "        .   .        \n"
        "        .   .        \n"
        "        .   .        \n"
        "        . . .        \n"
    );

    for (int player = 0; player < MAX_PLAYER_COUNT; ++player) {
        attron(COLOR_PAIR(getColor(player)));
        // Printout finish tiles and start tiles
        for (int tile = 0; tile < PAWN_COUNT; ++tile) {
            movePrintSpacing(playerPos[player][0][tile], (const char *) ".");
            movePrintSpacing(playerPos[player][1][tile], (const char *) (player % 2 == 0 ? "-" : "|"));
        }
        attroff(COLOR_PAIR(getColor(player)));
    }

    for (int player = 0; player < data->count; ++player) {
        attron(COLOR_PAIR(getColor(player)));

        // Printout pawns
        movePrintSpacing(data->pawns[player][0].pos, "1");
        movePrintSpacing(data->pawns[player][1].pos, "2");
        movePrintSpacing(data->pawns[player][2].pos, "3");
        movePrintSpacing(data->pawns[player][3].pos, "4");

        attroff(COLOR_PAIR(getColor(player)));
    }
    refresh();
    free(data);
}

int movePrintSpacing(Position position, const char* string)
{
    return mvprintw(position.y, position.x * SPACING, string);
}

int getColor(enum Player player)
{
    return (int) player + 1;
}

