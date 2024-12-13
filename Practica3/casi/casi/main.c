#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utils.h"

/* Variables globales para gestionar el estado del programa */
char tablaActual[100] = ""; /* Nombre de la tabla en uso */
char indiceActual[100] = ""; /* Nombre del archivo índice correspondiente */
enum {USE = 1, INSERT, PRINT, EXIT};
void menu();

void remove_space(char *s) {
    s[strlen(s)-1]  = '\0';
}

int main() {
    remove(tablaActual);
    remove(indiceActual);
    menu();
    return 0;
}

void menu() {
    int opcion;
    bool tablaSeleccionada = false;
    char book_id[4];
    char title[256];
    int ch;
    Book nuevoLibro;
    size_t nivelMaximo;
    size_t len;

    do {
        printf("\n=== Menú Interfaz de Usuario ===\n"
                "1. Use (Seleccionar una tabla)\n"
                "2. Insert (Agregar un registro a la tabla)\n"
                "3. Print (Imprimir el árbol binario de la tabla)\n"
                "4. Exit (Salir)\n"
                "Elige una opción: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case USE: { 
                printf("Ingresa el nombre de la tabla (.dat): ");
                scanf("%s", tablaActual);

                if (check_dat(tablaActual)) {
                    if (createTable(tablaActual)) {
                        replaceExtensionByIdx(tablaActual, indiceActual);
                        printf("Tabla '%s' seleccionada y lista para usar.\n", tablaActual);
                        tablaSeleccionada = true;
                    } else {
                        printf("Error: no se pudo seleccionar o crear la tabla.\n");
                        tablaSeleccionada = false;
                    }
                } else {
                    printf("Error: el nombre de la tabla debe terminar con '.dat'.\n");
                    tablaSeleccionada = false;
                }
                break;
            }

            case INSERT: {
                if (!tablaSeleccionada) {
                    printf("Error: selecciona una tabla primero usando la opción 'Use'.\n");
                    exit(1);
                }

                printf("Ingresa el ID del libro (4 caracteres): ");
                scanf("%4s", book_id); /* Limitar a 4 caracteres */

                /* Consumir cualquier carácter restante en el búfer de entrada */
                
                while ((ch = getchar()) != '\n' && ch != EOF);

                strncpy(nuevoLibro.book_id, book_id, PK_SIZE);

                /* Leer el título del libro */
                printf("Ingresa el título del libro: ");
                if (fgets(title, sizeof(title), stdin) != NULL) {
                    /* Eliminar el carácter de nueva línea si está presente */
                    remove_space(title);
                    len = strlen(title);
                    /* Asignar memoria para el título */
                    nuevoLibro.title_len = len;
                    nuevoLibro.title = malloc((len) * sizeof(char));
                    if (nuevoLibro.title == NULL) {
                        printf("Error: no se pudo asignar memoria para el título.\n");
                        exit(1);
                        break;
                    }
                    strncpy(nuevoLibro.title, title, len);
                } else {
                    printf("Error al leer el título del libro.\n");
                    break;
                }

                /* Agregar el libro a la tabla */
                if (addTableEntry(&nuevoLibro, tablaActual, indiceActual)) {
                    printf("Libro agregado correctamente a la tabla.\n");
                } else {
                    printf("Error: no se pudo agregar el libro.\n");
                }

                /* Liberar memoria asignada para el título */
                free(nuevoLibro.title);
                break;
}

            case PRINT: { 
                if (!tablaSeleccionada) {
                    printf("Error: selecciona una tabla primero usando la opción 'Use'.\n");
                } else {
                    
                    printf("Ingresa el nivel máximo a imprimir: ");
                    scanf("%zu", &nivelMaximo);

                    printf("Imprimiendo el árbol binario de la tabla '%s':\n", tablaActual);
                    printTree(nivelMaximo, indiceActual);
                }
                break;
            }

            case EXIT: {
                printf("Saliendo del programa.\n");
                /* Limpieza adicional si es necesaria */
                return;
            }

            default:
                printf("Error: opción no válida. Intenta de nuevo.\n");
                break;
        }
    } while (opcion != 4);
}
