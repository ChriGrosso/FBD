#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
int main(void) {
    int nChoice = 0;

    do {
        nChoice = ShowMainMenu();
        switch (nChoice) {
            case 1: 
                createTable("test.");
                break;
            
            case 2: 
                break;
            
            case 3: 
                printTree();
                break;
            case 4: 
                printf("Bye Bye\n\n");
                break;
           
        }
    } while (nChoice != 4);
    return 0;
}



int ShowMainMenu() {
    int nSelected = 0;
    char buf[16]; /* buffer for reading user input
                     in theory we only need 1 char
                     but a large buffer will handle
                     more gracefully wrong inputs
                     as 'qwerty' */

    do {
        printf(" (1) use\n"
               " (2) insert\n"
               " (3) print\n"
               " (4) exit\n\n"
               "Enter a number that corresponds to your choice > ");
        if (!fgets(buf, 16, stdin))
            /* reading input failed, give up: */
            nSelected =0;
        else
            /* have some input, convert it to integer: */
            nSelected = atoi(buf);
        printf("\n");

        if ((nSelected < 1) || (nSelected > 4)) {
            printf("You have entered an invalid choice. Please try again\n\n\n");
        }
    } while ((nSelected < 1) || (nSelected > 4));

    return nSelected;
}
