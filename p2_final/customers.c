#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"
#include "customers.h"
#include "types.h"
#include <stdio.h> 

enum {FIND_OPTIONS = 1,LIST,BALANCE,BACK_C};

static int customers_find(void);
static int customers_listProd(void);
static int customers_balance(void);
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
            printf("Customers menu:\n"
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
        case FIND_OPTIONS:
            finished = customers_find();
            break;
        case LIST:
            finished = customers_listProd();
            break;
        case BALANCE:
            finished = customers_balance();
            break;
        case BACK_C:
            finished =  FINISHED;
            break;
        }
    }
    return;
}



static int customers_find(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    int finished=0;
    SQLRETURN ret2; /* ODBC API return customerName */
    #define BufferLength 512
    char x1[BufferLength] = "\0";
    char contactFirstName[BufferLength] = "\0";
    char contactLastName[BufferLength] = "\0";
    char customerName[BufferLength] = "\0";
    char customerNumber[BufferLength] = "\0";

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
    ret = SQLPrepare(stmt, (SQLCHAR*) "Select customername, contactfirstname, contactlastname, customernumber from customers where contactfirstname LIKE ? or contactlastname LIKE ? order by customernumber", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Error in SQLPrepare\n");
        odbc_extract_error("Error preparing statement", stmt, SQL_HANDLE_ENV);
        return FINISHED;
    }

    printf("Enter customer name > ");
    (void) fflush(stdout);

    while (finished == 0 && fgets(x1, (int) sizeof(x1), stdin) != NULL) {  
        remove_space(x1);  
        char query[BufferLength+3] = "\0";
        (void)snprintf(query,sizeof(query),"%%%s%%",x1);

        ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR, 0, 0, query, 0, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding parameter\n");
            odbc_extract_error("Error binding parameter", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

        ret = SQLBindParameter(stmt, 2, SQL_PARAM_INPUT,
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

        ret = SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) customerName, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

        ret = SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*) contactFirstName, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }
        
        ret = SQLBindCol(stmt, 3, SQL_C_CHAR, (SQLCHAR*) contactLastName, BufferLength, NULL);

        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }
        ret = SQLBindCol(stmt, 4, SQL_C_CHAR, (SQLCHAR*) customerNumber, BufferLength, NULL);

        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }
        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s %s %s\n", customerNumber, customerName, contactFirstName,contactLastName);
            finished=1;
        }


        if(finished == 0) {
            printf("Enter customer name > ");
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



static int customers_listProd(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    int finished=0;
    SQLRETURN ret2; /* ODBC API return customerName */
    #define BufferLength 512
    char x1[BufferLength] = "\0";
    char productName[BufferLength] = "\0";
    char productCode[BufferLength] = "\0";
    char total[BufferLength] = "\0";
    
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
    ret = SQLPrepare(stmt, (SQLCHAR*) "select  productname, productcode, sum(quantityordered) as total from orders natural join orderdetails natural join products where customernumber = ? group by productname, productcode order by productcode ASC", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Error in SQLPrepare\n");
        odbc_extract_error("Error preparing statement", stmt, SQL_HANDLE_ENV);
        return FINISHED;
    }

    printf("Enter customer number > ");
    (void) fflush(stdout);

    while (finished == 0 && fgets(x1, (int) sizeof(x1), stdin) != NULL) {  
        remove_space(x1);  


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

  
        ret = SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) productName, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

        ret = SQLBindCol(stmt, 2, SQL_C_CHAR, (SQLCHAR*) productCode, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

        ret = SQLBindCol(stmt, 3, SQL_C_CHAR, (SQLCHAR*) total, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }

        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("%s %s\n", productName, total);
            finished = 1;
        }

        if (finished == 0) {
            printf("Enter customer number > ");
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

static int customers_balance(void) {
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    int finished=0;
    SQLRETURN ret2; /* ODBC API return customerName */
    #define BufferLength 512
    char x1[BufferLength] = "\0";
    char risultato[BufferLength] = "\0";
    
    
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
    ret = SQLPrepare(stmt, (SQLCHAR*) "SELECT(SELECT SUM(amount) AS saldo FROM payments WHERE customernumber = ?)-(SELECT SUM(od.quantityordered * od.priceeach) AS buyed FROM customers c NATURAL JOIN orders NATURAL JOIN orderdetails od WHERE c.customernumber = ?) as risultato", SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        printf("Error in SQLPrepare\n");
        odbc_extract_error("Error preparing statement", stmt, SQL_HANDLE_ENV);
        return FINISHED;
    }

    printf("Enter customer number > ");
    (void) fflush(stdout);

    while (finished == 0 && fgets(x1, (int) sizeof(x1), stdin) != NULL) {  
        remove_space(x1);  


        ret = SQLBindParameter(stmt, 1, SQL_PARAM_INPUT,
                                SQL_C_CHAR, SQL_CHAR, 0, 0, x1, 0, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding parameter\n");
            odbc_extract_error("Error binding parameter", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }
        ret = SQLBindParameter(stmt, 2, SQL_PARAM_INPUT,
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

  
        ret = SQLBindCol(stmt, 1, SQL_C_CHAR, (SQLCHAR*) risultato, BufferLength, NULL);
        if (!SQL_SUCCEEDED(ret)) {
            printf("Error binding column\n");
            odbc_extract_error("Error binding column", stmt, SQL_HANDLE_STMT);
            return FINISHED;
        }


        while (SQL_SUCCEEDED(ret = SQLFetch(stmt))) {
            printf("Balance = %s\n", risultato);
            finished = 1;
        }

        if (finished == 0) {
            printf("Enter customer number > ");
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