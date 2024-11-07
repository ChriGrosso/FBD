#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "products.h"
#include <stdio.h> 

enum {STOCKS = 1, FIND, BACK};
enum {NO_FINISHED,FINISHED};
static int products_stock(void);
static int products_find(void);
static void remove_space(char *s);
void remove_space(char *s) {
    size_t n = strlen(s);
    s[n-1] = '\0';
}
void productsMenu() {
    int nSelected, finished = 0;
    char buf[16];
    while (finished == 0) {
        do {
            printf("Products menu:\n"
                    "\t(1) Stocks.\n"
                    "\t(2) Find.\n"
                    "\t(3) Back\n"
                    "Enter a number that corresponds to your choice > ");
            if (!fgets(buf, 16, stdin))
                /* reading input failed, give up: */
                nSelected =0;
            else
                /* have some input, convert it to integer: */
                nSelected = atoi(buf);
            printf("\n");

            if ((nSelected < 1) || (nSelected > 3)) {
                printf("You have entered an invalid choice. Please try again\n\n\n");
            }
        } while ((nSelected < 1) || (nSelected > 3));


        switch (nSelected )
        {
        case STOCKS:
            finished = products_stock();
            break;
        case FIND:
            finished = products_find();
            break;
        case BACK:
            finished =  FINISHED;
            break;
        }
    }
    return;
}

int products_stock(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    SQLRETURN ret2; /* ODBC API return status */
    #define BufferLength 512
    char x[BufferLength] = "\0";
    char y[BufferLength] = "\0";
    int finished = 0;

    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Errpr in connect");
        return FINISHED;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Error in AllocHandle\n");
        return FINISHED;
    }

    printf("Enter productcode > ");
    (void) fflush(stdout);
    while (finished == 0 && fgets(x, (int) sizeof(x), stdin) != NULL) {
        remove_space(x);
        char query[BufferLength + 28];
        /* snprintf is not defined if ansi flag is enabled */
        (void) snprintf(query, (size_t)(BufferLength + 100), "select quantityinstock from products where productcode = '%s';", x);
        printf("query %s\n",query);

        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);
	
        (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) y, BufferLength , NULL);
        /* Loop through the rows in the result-set */
        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("quantityinstock = %s\n", y);
            finished = 1;
        }

        ret2 = SQLCloseCursor(stmt);
        if (!SQL_SUCCEEDED(ret2)) {
            odbc_extract_error("", stmt, SQL_HANDLE_STMT);
            printf("Error in CLoseCursor");
           return FINISHED;
        }
        if (finished == 0) {
            printf("Enter productcode > ");
            (void) fflush(stdout);
        }
    }
    printf("\n");
    
    /* free up statement handle */
    ret2 = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (!SQL_SUCCEEDED(ret2)) {
        printf("\nERROR in FreeHandle\n");
        return FINISHED;
    }

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Error in disconnect\n");
        return FINISHED;
    }

    return NO_FINISHED;
}

int products_find(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    SQLRETURN ret2; /* ODBC API return status */
    #define BufferLength 512
    char x[BufferLength] = "\0";
    char y[BufferLength] = "\0";
    char z[BufferLength] = "\0";
    int finished = 0;
    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        printf("\nError in odbc_connect\n");
        return FINISHED;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        printf("\nError in SQLAllocHandle\n");
        return FINISHED;
    }

    printf("Enter productname > ");
    (void) fflush(stdout);
    while (finished == 0 &&((fgets(x, (int) sizeof(x), stdin)) != NULL)) {
        remove_space(x);
        char query[BufferLength + 28];
        /* snprintf is not defined if ansi flag is enabled */
        (void) snprintf(query, (size_t)(BufferLength + 100), "select productcode, productname from products where productname LIKE '%%%s%%';", x);
        printf("query %s\n",query);

        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);

        (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) z, BufferLength , NULL);
        (void) SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*) y, BufferLength , NULL);
        /* Loop through the rows in the result-set */
        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s\n",z,y);
            finished = 1;
        }

        ret2 = SQLCloseCursor(stmt);
        if (!SQL_SUCCEEDED(ret2)) {
            printf("\nError in SQLCloseCursor\n");
            return FINISHED;
        }
        if(finished == 0) {
            printf("Enter productname > ");
            ret = fflush(stdout);
        }
        
    }
    printf("\n");
    
    /* free up statement handle */
    ret2 = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (!SQL_SUCCEEDED(ret2)) {
        odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        printf("\nError in SQLFreeHandle\n");
        return FINISHED;
    }

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        printf("\nError in obdb_disconnect\n");
        return FINISHED;
    }

    return NO_FINISHED;
}
