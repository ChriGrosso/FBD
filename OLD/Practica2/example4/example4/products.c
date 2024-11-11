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
    char x[BufferLength] = "\0";  // Para almacenar el código de producto (cadena)
    char y[BufferLength] = "\0";  // Buffer para almacenar la cantidad en stock
    int finished = 0;
    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Error in connect\n");
        return FINISHED;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Error in AllocHandle\n");
        return FINISHED;
    }

    /* Preparar la consulta SQL con un parámetro */
    ret = SQLPrepare(stmt, (SQLCHAR*) "select quantityinstock from products where productcode = ?", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Error in SQLPrepare\n");
        odbc_extract_error("Error preparing statement", stmt, SQL_HANDLE_ENV);
        return FINISHED;
    }

    printf("Enter productcode > ");
    (void) fflush(stdout);

    while (finished == 0 && fgets(x, (int) sizeof(x), stdin) != NULL) {  
        remove_space(x);  


        ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR, 0, 0, x, 0, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding parameter\n");
            odbc_extract_error("Error binding parameter", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

        ret = SQLExecute(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error executing query\n");
            odbc_extract_error("Error executing query", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

  
        ret = SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) y, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

    
        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("quantityinstock = %s\n", y);
            finished = 1;
        }

        if (finished == 0) {
            printf("Enter productcode > ");
            (void) fflush(stdout);
        }
    }

    printf("\n");

    /* Free up statement handle */
    ret2 = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (!SQL_SUCCEEDED(ret2)) {
        printf("ERROR in FreeHandle\n");
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
    char x[BufferLength] = "\0";  // Para almacenar el código de producto (cadena)
    char y[BufferLength] = "\0";  // Buffer para almacenar la cantidad en stock
    char z[BufferLength] = "\0";
    int finished = 0;
    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Error in connect\n");
        return FINISHED;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Error in AllocHandle\n");
        return FINISHED;
    }

    /* Preparar la consulta SQL con un parámetro */
    ret = SQLPrepare(stmt, (SQLCHAR*) "select productcode, productname from products where productname LIKE ?", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Error in SQLPrepare\n");
        odbc_extract_error("Error preparing statement", stmt, SQL_HANDLE_ENV);
        return FINISHED;
    }

    printf("Enter productname > ");
    (void) fflush(stdout);

    while (finished == 0 && fgets(x, (int) sizeof(x), stdin) != NULL) {  
        remove_space(x);  
        char query[BufferLength+3] = "\0";
        snprintf(query,sizeof(query),"%%%s%%",x);

        ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR, 0, 0, query, 0, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding parameter\n");
            odbc_extract_error("Error binding parameter", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

        ret = SQLExecute(stmt);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error executing query\n");
            odbc_extract_error("Error executing query", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

  
        ret = SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) y, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

        ret = SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*) z, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }
        
    
        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s\n",z,y);
            finished = 1;
        }


        if(finished == 0) {
            printf("Enter productname > ");
            ret = fflush(stdout);
        }
    }

    printf("\n");

    /* Free up statement handle */
    ret2 = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (!SQL_SUCCEEDED(ret2)) {
        printf("ERROR in FreeHandle\n");
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
