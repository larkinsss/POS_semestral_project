#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

int main() {

    char *hernePole[2];
    char riadok1[3] = {'.','.','.'};
    char riadok2[3] = {'.','_','.'};


    strcpy(hernePole[0], riadok1);
    strcpy(hernePole[1], riadok2);

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 3; ++j) {
            printf("%c", *hernePole[i][j]);
        }
        printf("\n");
    }
    printf("Hello, World!\n");
    return 0;
}
