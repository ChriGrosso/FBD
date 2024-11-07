#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "customers.h"
#include <stdio.h> 

enum {NO_FINISHED,FINISHED};
static int customers_find(void);
static int customers_listProd(void);
static customers_balance(void);
static void remove_space(char *s);
void remove_space(char *s) {
    size_t n = strlen(s);
    s[n-1] = '\0';
}
void customersMenu() {
    int nSelected, finished = 0;
    char buf[16];
    while (finished == 0) {
        do {
            printf("Products menu:\n"
                    "\t(1) Find.\n"
                    "\t(2) List Products.\n"
                    "\t(3) Balance\n"
                    "\t(4) Back\n"
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


        switch (nSelected)
        {
        case 1:
            finished = customers_find();
            break;
        case 2:
            finished = customers_listProd();
            break;
        case 3:
            finished = customers_balance();
            break;
        case 4:
            finished =  FINISHED;
            break;
        }
    }
    return;
}

static int customers_find(void){
    return 0;
}
static int customers_listProd(void){
    return 0;
}
static customers_balance(void){
    return 0;
}