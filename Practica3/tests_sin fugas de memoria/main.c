#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "utils.h"

// Variables globales para gestionar el estado del programa
char tablaActual[100] = ""; // Nombre de la tabla en uso
char indiceActual[100] = ""; // Nombre del archivo índice correspondiente

void menu();

int main() {
    menu();
    return 0;
}

void menu() {
    int opcion;
    bool tablaSeleccionada = false;

    do {
        printf("\n=== Menú Interfaz de Usuario ===\n");
        printf("1. Use (Seleccionar una tabla)\n");
        printf("2. Insert (Agregar un registro a la tabla)\n");
        printf("3. Print (Imprimir el árbol binario de la tabla)\n");
        printf("4. Exit (Salir)\n");
        printf("Elige una opción: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1: { // Use
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

            case 2: { // Insert
                if (!tablaSeleccionada) {
                    printf("Error: selecciona una tabla primero usando la opción 'Use'.\n");
                } else {
                    char book_id[4];
                    char title[256];
                    Book nuevoLibro;

                    printf("Ingresa el ID del libro (4 caracteres): ");
                    scanf("%4s", book_id); // Limitar a 4 caracteres

                    // Consumir cualquier carácter restante en el búfer de entrada
                    int ch;
                    while ((ch = getchar()) != '\n' && ch != EOF);

                    strncpy(nuevoLibro.book_id, book_id, 4);

                    // Leer el título del libro
                    printf("Ingresa el título del libro: ");
                    if (fgets(title, sizeof(title), stdin) != NULL) {
                        // Eliminar el carácter de nueva línea si está presente
                        size_t len = strlen(title)-1;
                        // Asignar memoria para el título
                        nuevoLibro.title_len = len;
                        nuevoLibro.title = malloc((len) * sizeof(char));
                        if (nuevoLibro.title == NULL) {
                            printf("Error: no se pudo asignar memoria para el título.\n");
                            break;
                        }
                        strncpy(nuevoLibro.title, title, len);
                    } else {
                        printf("Error al leer el título del libro.\n");
                        break;
                    }

                    // Agregar el libro a la tabla
                    if (addTableEntry(&nuevoLibro, tablaActual, indiceActual)) {
                        printf("Libro agregado correctamente a la tabla.\n");
                    } else {
                        printf("Error: no se pudo agregar el libro.\n");
                    }

                    // Liberar memoria asignada para el título
                    free(nuevoLibro.title);
                }
                break;
}

            case 3: { // Print
                if (!tablaSeleccionada) {
                    printf("Error: selecciona una tabla primero usando la opción 'Use'.\n");
                } else {
                    size_t nivelMaximo;
                    printf("Ingresa el nivel máximo a imprimir: ");
                    scanf("%zu", &nivelMaximo);

                    printf("Imprimiendo el árbol binario de la tabla '%s':\n", tablaActual);
                    printTree(nivelMaximo, indiceActual);
                }
                break;
            }

            case 4: { // Exit
                printf("Saliendo del programa.\n");
                // Limpieza adicional si es necesaria
                break;
            }

            case 5: {
                FILE *file;
                const char *filename = "MB2.dat";
                char buffer[1024]; // Buffer per leggere i dati
                size_t bytesRead;

                // Aprire il file in modalità binaria per la lettura
                file = fopen(filename, "rb");
                if (file == NULL) {
                    perror("Errore nell'apertura del file");
                    return EXIT_FAILURE;
                }

                printf("Contenuto tradotto del file %s:\n", filename);

                // Leggere i dati dal file
                while ((bytesRead = fread(buffer, 1, sizeof(buffer) - 1, file)) > 0) {
                    buffer[bytesRead] = '\0'; // Aggiungere un terminatore di stringa
                    printf("%s", buffer);    // Stampa il contenuto come stringa
                }

                if (ferror(file)) {
                    perror("Errore nella lettura del file");
                    fclose(file);
                    return EXIT_FAILURE;
                }

                // Chiudere il file
                fclose(file);
                printf("\nFile letto e tradotto con successo.\n");

                return EXIT_SUCCESS;
            }

            default:
                printf("Error: opción no válida. Intenta de nuevo.\n");
        }
    } while (opcion != 4);
}
