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
static int orders_detail(void);
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
            printf("Orders menu:\n"
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

static int orders_open(void){
    SQLHENV env = NULL; /* enviroment handle */
    SQLHDBC dbc = NULL; /* connection handle */
    SQLHSTMT stmt = NULL; /* connection handle */
    int ret; /* ODBC API return status */
    SQLSMALLINT columns = 0; /* number of columns in result-set */
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
    ret = SQLExecDirect(stmt, (SQLCHAR*) "select ordernumber from orders where shippeddate IS NULL order by ordernumber", SQL_NTS);
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
    int finished=0;
    SQLRETURN ret2; /* ODBC API return status */
    #define BufferLength 512
    char *toks;
    char q[BufferLength] = "\0";
    char x1[11] = "\0";
    char x2[11] = "\0";
    char orderNumber[BufferLength] = "\0";
    char orderDate[BufferLength] = "\0";
    char shippedDate[BufferLength] = "\0";

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

    printf("Enter dates (YYYY-MM-DD - YYYY-MM-DD) > ");
    (void) fflush(stdout);
    while (!finished && (fgets(q, (int) sizeof(q), stdin) != NULL)) {
        /*to separate both dates*/
        toks = strtok(q, " ");
        strcpy(x1,toks);
        toks = strtok(NULL, " ");
        toks = strtok(NULL, "\n");
        strcpy(x2,toks);

        char query[BufferLength + 28];
        /* snprintf is not defined if ansi flag is enabled */
        (void) snprintf(query, (size_t)(BufferLength + 40), "select ordernumber, orderdate, shippeddate from orders where orderdate >= '%s' and orderdate <= '%s';", x1,x2);

        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);

        /* Vincula las columnas de la consulta a variables */
        SQLBindCol(stmt, 1, SQL_C_CHAR, orderNumber, BufferLength, NULL);
        SQLBindCol(stmt, 2, SQL_C_CHAR, orderDate, BufferLength, NULL);
        SQLBindCol(stmt, 3, SQL_C_CHAR, shippedDate, BufferLength, NULL);

        /* Recorre y muestra los resultados */
        printf("Order Number\tOrder Date\tShipped Date:\n");
        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s %s\n", orderNumber, orderDate, shippedDate);
            finished=1;
        }

        ret2 = SQLCloseCursor(stmt);
        if (!SQL_SUCCEEDED(ret2)) {
            odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return ret;
        }
        
        if (finished == 0) {
            printf("Enter dates (YYYY-MM-DD - YYYY-MM-DD) > ");
            (void) fflush(stdout);
        }
    }
    
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
static int orders_detail(void){
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    int finished=0;
    SQLRETURN ret2; /* ODBC API return status */
    #define BufferLength 512
    char x1[BufferLength] = "\0";
    char orderNumber[BufferLength] = "\0";
    char orderDate[BufferLength] = "\0";
    char status[BufferLength] = "\0";
    char productCode[BufferLength] = "\0";
    char quantityOrdered[BufferLength] = "\0";
    char priceEach[BufferLength] = "\0";
    char orderLineNumber[BufferLength] = "\0";

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

    printf("Enter ordernumber > ");
    (void) fflush(stdout);
    while (!finished &&(fgets(x1, (int) sizeof(x1), stdin) != NULL)) {
        remove_space(x1);
        char query[BufferLength + 100];
        /* snprintf is not defined if ansi flag is enabled */
        (void) snprintf(query, (size_t)(BufferLength + 150), "Select ordernumber,orderdate, status, productcode,quantityordered,priceeach,orderlinenumber "
                                                            "from orders natural join orderdetails "
                                                            "where ordernumber = %d order by orderlinenumber;",atoi(x1));
        //group by ordernumber
        //order by orderlinenumber

        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);

        /* Vincula las columnas de la consulta a variables */
        SQLBindCol(stmt, 1, SQL_C_CHAR, orderNumber, BufferLength, NULL);
        SQLBindCol(stmt, 2, SQL_C_CHAR, orderDate, BufferLength, NULL);
        SQLBindCol(stmt, 3, SQL_C_CHAR, status, BufferLength, NULL);
        SQLBindCol(stmt, 4, SQL_C_CHAR, productCode, BufferLength, NULL);
        SQLBindCol(stmt, 5, SQL_C_CHAR, quantityOrdered, BufferLength, NULL);
        SQLBindCol(stmt, 6, SQL_C_CHAR, priceEach, BufferLength, NULL);
        SQLBindCol(stmt, 7, SQL_C_CHAR, orderLineNumber, BufferLength, NULL);

        /* Recorre y muestra los resultados */
        if (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            finished = 1;
            printf("OrderDate: %s\n"
                "Status: %s\n\n",orderDate, status);
        }

        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s %s\n",productCode, quantityOrdered, priceEach);
        }

        ret2 = SQLCloseCursor(stmt);
        if (!SQL_SUCCEEDED(ret2)) {
            odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return ret;
        }
        if (finished == 0) {
            printf("Enter ordernumber > ");
            (void) fflush(stdout);
        }
    }
    
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