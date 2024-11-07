#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "orders.h"
#include <stdio.h> 

enum {NO_FINISHED,FINISHED};
static int orders_open(void);
static int orders_range(void);
static orders_detail(void);
static void remove_space(char *s);
void remove_space(char *s) {
    size_t n = strlen(s);
    s[n-1] = '\0';
}
void ordersMenu() {
    int nSelected, finished = 0;
    char buf[16];
    while (finished == 0) {
        do {
            printf("Products menu:\n"
                    "\t(1) Open.\n"
                    "\t(2) Range.\n"
                    "\t(3) Detail\n"
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
            finished = orders_open();
            break;
        case 2:
            finished = orders_range();
            break;
        case 3:
            finished = orders_detail();
            break;
        case 4:
            finished =  FINISHED;
            break;
        }
    }
    return;
}

int products_find(void) {
     
}


static int orders_open(void){
    SQLHENV env = NULL; /* enviroment handle */
    SQLHDBC dbc = NULL; /* connection handle */
    SQLHSTMT stmt = NULL; /* connection handle */
    int ret; /* ODBC API return status */
    SQLSMALLINT columns = 0; /* number of columns in result-set */
    SQLUSMALLINT ucolumns = 0;
    SQLUSMALLINT i = 0;
    SQLINTEGER x = 0;
    #define BufferLength 512
    char y[BufferLength] = "\0";

    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }
    /* simple query */
    ret = SQLExecDirect(stmt, (SQLCHAR*) "select * from orders where shippeddate=NULL order by ordernumber", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return ret;
    }
    /* How many columns are there */
    ret = SQLNumResultCols(stmt, &columns);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("Error getting number of columns", stmt, SQL_HANDLE_STMT);
        return ret;
    }

    /* print the name of each column */
    ucolumns = (SQLUSMALLINT)columns;
    for (i = 1; i <= ucolumns; i++) {
        char buf[BufferLength] = "\0";
        ret = SQLDescribeCol(stmt, i, (SQLCHAR *) buf, BufferLength, NULL, NULL, NULL, NULL, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            odbc_extract_error("Error in SQLDescribeCol", stmt, SQL_HANDLE_STMT);
            return ret;
        }
        printf("%s\t", buf);
    }
    printf("\n");

    /* Loop through the rows in the result-set */
    while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
        (void) SQLGetData(stmt, 1, SQL_C_SLONG, &x, 0, NULL);
        (void) SQLGetData(stmt, 2, SQL_C_CHAR, (SQLCHAR *)y, BufferLength, NULL);
        printf("%d\t%s\n", x, y);
    }
 
    // Free handles
    // Statement
    if (stmt != SQL_NULL_HSTMT)
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
static int orders_range(void){
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    SQLRETURN ret2; /* ODBC API return status */
    #define BufferLength 512
    char x[BufferLength] = "\0";
    char y[BufferLength] = "\0";

    /* CONNECT */
    ret = odbc_connect(&env, &dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    printf("From Date [AAAA-MM-DD] = ");
    (void) fflush(stdout);
    while (fgets(x, sizeof(x), stdin) != NULL) {
        printf("To Date [AAAA-MM-DD] = ");
        (void) fflush(stdout);
        while(fgets(y, sizeof(y), stdin) != NULL) {
            char query[BufferLength + 28];
            /* snprintf is not defined if ansi flag is enabled */
            (void) snprintf(query, (size_t)(BufferLength + 27), "select ordernumber, orderdate, shippeddate from orders where orderdate > '%s' and orderdate < '%s';", x,y);

            (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);

            (void) SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) y, BufferLength , NULL);

            /* Loop through the rows in the result-set */
            while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
                printf("y = %s\n", y);
            }

            ret2 = SQLCloseCursor(stmt);
            if (!SQL_SUCCEEDED(ret2)) {
                odbc_extract_error("", stmt, SQL_HANDLE_STMT);
            return ret;
            }

            printf("x = ");
            ret = fflush(stdout);
        }
    }
    printf("\n");
    
    /* free up statement handle */
    ret2 = SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    if (!SQL_SUCCEEDED(ret2)) {
        odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return ret;
    }

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
static orders_detail(void){
    return 0;
}