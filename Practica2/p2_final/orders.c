#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "orders.h"
#include "types.h"
#include <stdio.h> 

enum{OPEN = 1, RANGE, DETAIL, BACK_O};

static int orders_open(void);
static int orders_range(void);
static int orders_detail(void);


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
        case OPEN:
            finished = orders_open();
            break;
        case RANGE:
            finished = orders_range();
            break;
        case DETAIL:
            finished = orders_detail();
            break;
        case BACK_O:
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
        return FINISHED;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return FINISHED;
    }
    /* simple query */
    ret = SQLExecDirect(stmt, (SQLCHAR*) "select ordernumber from orders where shippeddate IS NULL order by ordernumber", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return FINISHED;
    }
    /* How many columns are there */
    ret = SQLNumResultCols(stmt, &columns);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("Error getting number of columns", stmt, SQL_HANDLE_STMT);
        return FINISHED;
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
        (void)SQLFreeHandle(SQL_HANDLE_STMT, stmt);

    /* DISCONNECT */
    ret = odbc_disconnect(env, dbc);
    if (!SQL_SUCCEEDED(ret)) {
        return FINISHED;
    }

    return NO_FINISHED;
}



static int orders_range(void) {
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
    ret = SQLPrepare(stmt, (SQLCHAR*) "select ordernumber, orderdate, shippeddate from orders where orderdate >= ? and orderdate <= ?", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Error in SQLPrepare\n");
        odbc_extract_error("Error preparing statement", stmt, SQL_HANDLE_ENV);
        return FINISHED;
    }

    printf("Enter dates (YYYY-MM-DD - YYYY-MM-DD) > ");
    (void) fflush(stdout);

    while (finished == 0 && fgets(q, (int) sizeof(q), stdin) != NULL) {  

        /*to separate both dates*/
        toks = strtok(q, " ");
        strcpy(x1,toks);
        toks = strtok(NULL, " ");
        toks = strtok(NULL, "\n");
        strcpy(x2,toks);


        ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR, 0, 0, x1, 0, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding parameter\n");
            odbc_extract_error("Error binding parameter", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

        ret = SQLBindParameter(stmt, 2, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR, 0, 0, x2, 0, NULL);
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

  
        ret = SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) orderNumber, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

        ret = SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*) orderDate, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

        ret = SQLBindCol(stmt, 3, SQL_C_CHAR, (SQLCHAR*) shippedDate, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }
        
    
        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s %s\n", orderNumber, orderDate, shippedDate);
            finished=1;
        }


        if(finished == 0) {
            printf("Enter dates (YYYY-MM-DD - YYYY-MM-DD) > ");
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

 
static int orders_detail(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    int finished=0,print = 0;
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
    ret = SQLPrepare(stmt, (SQLCHAR*) "Select ordernumber,orderdate, status, productcode,quantityordered,priceeach,orderlinenumber "
                                                            "from orders natural join orderdetails "
                                                            "where ordernumber = ? order by orderlinenumber", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Error in SQLPrepare\n");
        odbc_extract_error("Error preparing statement", stmt, SQL_HANDLE_ENV);
        return FINISHED;
    }

    printf("Enter ordernumber > ");
    (void) fflush(stdout);

    while (finished == 0 && (fgets(x1, (int) sizeof(x1), stdin) != NULL)) {  


        ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR, 0, 0, x1, 0, NULL);
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


        ret = SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) orderNumber, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

        ret = SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*) orderDate, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

        ret = SQLBindCol(stmt, 3, SQL_C_CHAR, (SQLCHAR*) status, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }
        ret = SQLBindCol(stmt, 4, SQL_C_CHAR, (SQLCHAR*) productCode, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }
        ret = SQLBindCol(stmt, 5, SQL_C_CHAR, (SQLCHAR*) quantityOrdered, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }
        ret = SQLBindCol(stmt, 6, SQL_C_CHAR, (SQLCHAR*) priceEach, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }
        ret = SQLBindCol(stmt, 7, SQL_C_CHAR, (SQLCHAR*) orderLineNumber, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }
        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            if(print == 0) {
                printf("OrderDate: %s\nStatus: %s\n\n",orderDate, status);
                print = finished = 1;
            }
            printf("%s %s %s\n",productCode, quantityOrdered, priceEach);
        }


        if(finished == 0) {
            printf("Enter ordernumber > ");
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