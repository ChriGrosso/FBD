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
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    printf("Enter customer name > ");
    (void) fflush(stdout);
    while (!finished && (fgets(x1, (int) sizeof(x1), stdin) != NULL)) {  
        char query[BufferLength + 100];
        remove_space(x1);
        /* snprintf is not defined if ansi flag is enabled */
        (void) snprintf(query, (size_t)(BufferLength + 150), "Select customername, contactfirstname, contactlastname, customernumber from customers where contactfirstname LIKE '%%%s%%' or contactlastname LIKE '%%%s%%' order by customernumber;",x1,x1);
        
        // "Select customername, contactfirstname, contactlastname, customernumber from customers where contactfirstname like '%s'; ",x1);
                                                            //or contactlastname= '%s' "
                                                            //"order by customernumber;",x1,x1);


        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);

        /* Vincula las columnas de la consulta a variables */
        SQLBindCol(stmt, 1, SQL_C_CHAR, customerName, BufferLength, NULL);
        SQLBindCol(stmt, 2, SQL_C_CHAR, contactFirstName, BufferLength, NULL);
        SQLBindCol(stmt, 3, SQL_C_CHAR, contactLastName, BufferLength, NULL);
        SQLBindCol(stmt, 4, SQL_C_CHAR, customerNumber, BufferLength, NULL);
        
        /* Recorre y muestra los resultados */
        while(SQL_SUCCEEDED(ret = SQLFetch(stmt))){
            printf("%s %s %s %s\n", customerNumber, customerName, contactFirstName,contactLastName);
        }

        ret2 = SQLCloseCursor(stmt);
        if (!SQL_SUCCEEDED(ret2)) {
            odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return ret;
        }
        finished=1;
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

static int customers_listProd(void){
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
        return EXIT_FAILURE;
    }

    /* Allocate a statement handle */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        odbc_extract_error("", stmt, SQL_HANDLE_ENV);
        return ret;
    }

    printf("Customer Number = ");
    (void) fflush(stdout);
    while ((fgets(x1, (int) sizeof(x1), stdin) != NULL) && !finished) {  
        char query[BufferLength + 100];
        /* snprintf is not defined if ansi flag is enabled */
        (void) snprintf(query, (size_t)(BufferLength + 150), "select  productname, productcode, sum(quantityordered) as total from orders natural join orderdetails natural join products where customernumber = %d group by productname, productcode order by productcode ASC;",atoi(x1));
        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);

        /* Vincula las columnas de la consulta a variables */
        SQLBindCol(stmt, 1, SQL_C_CHAR, productName, BufferLength, NULL);
        SQLBindCol(stmt, 2, SQL_C_CHAR, productCode, BufferLength, NULL);
        SQLBindCol(stmt, 3, SQL_C_CHAR, total, BufferLength, NULL);
        
        /* Recorre y muestra los resultados */
        while(SQL_SUCCEEDED(ret = SQLFetch(stmt))){
            printf("%s %s\n", productName, total);
        }

        ret2 = SQLCloseCursor(stmt);
        if (!SQL_SUCCEEDED(ret2)) {
            odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return ret;
        }
        finished=1;
        printf("Press enter to Continue...\n");
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
    
    return 0;
}
static customers_balance(void){
    SQLHENV env = NULL;
    SQLHDBC dbc = NULL;
    SQLHSTMT stmt = NULL;
    int ret; /* odbc.c */
    int finished=0;
    SQLRETURN ret2; /* ODBC API return customerName */
    #define BufferLength 512
    char x1[BufferLength] = "\0";
    char customerNumber[BufferLength] = "\0";
    char customerName[BufferLength] = "\0";
    char saldo[BufferLength] = "\0";
    
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

    printf("Enter customer number > ");
    (void) fflush(stdout);
    while ( !finished && (fgets(x1, (int) sizeof(x1), stdin) != NULL)) {  
        char query[BufferLength + 100];
        /* snprintf is not defined if ansi flag is enabled */
        (void) snprintf(query, (size_t)(BufferLength + 150), "SELECT customernumber, customername, (SUM(p.amount) - SUM(od.quantityordered * od.priceeach)) AS saldo FROM customers c NATURAL JOIN payments p NATURAL JOIN orderdetails od WHERE c.customernumber = %d GROUP BY c.customernumber, c.customername;",atoi(x1));
        (void) SQLExecDirect(stmt, (SQLCHAR*) query, SQL_NTS);

        /* Vincula las columnas de la consulta a variables */
        SQLBindCol(stmt, 1, SQL_C_CHAR, customerNumber, BufferLength, NULL);
        SQLBindCol(stmt, 2, SQL_C_CHAR, customerName, BufferLength, NULL);
        SQLBindCol(stmt, 3, SQL_C_CHAR, saldo, BufferLength, NULL);
        
        /* Recorre y muestra los resultados */
        while(SQL_SUCCEEDED(ret = SQLFetch(stmt))){
            printf("%s\n", saldo);
        }

        ret2 = SQLCloseCursor(stmt);
        if (!SQL_SUCCEEDED(ret2)) {
            odbc_extract_error("", stmt, SQL_HANDLE_STMT);
        return ret;
        }
        finished=1;
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
    
    return 0;
    return 0;
}