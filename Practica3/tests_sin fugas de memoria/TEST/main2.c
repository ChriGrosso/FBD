#ifndef CODE_V2_UTILS_H
#define CODE_V2_UTILS_H
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define NO_DELETED_REGISTERS -1

#define INDEX_HEADER_SIZE 8
#define DATA_HEADER_SIZE  4
#define PK_SIZE 4

/* Our table is going to contain a string (title) and
   an alphanumeric primary key (book_id)
*/
typedef struct book {
    char book_id[PK_SIZE ]; /* primary key */
    size_t title_len; /* title length */
    char *title; /* string to be saved in the database */
} Book;

/* Note that in general a struct cannot be used to
   handle tables in databases since the table structure
   is unknown on compilation time.
*/

typedef struct node {
    char book_id[PK_SIZE];
    int left, right, parent, offset;
}Node;


/* Function prototypes.
   see function description in the following sections

   All function return true if success or false if failed
   except findKey which return true if register found
   and false otherwise.
   */

bool createTable(const char * tableName);
bool createIndex(const char * indexName);
bool findKey(const char * book_id, const char * indexName,
             int * nodeIDOrDataOffset);
bool addTableEntry(Book * book, const char * tableName,
                   const char * indexName);
bool addIndexEntry(char * book_id, int bookOffset, const char * indexName);

void printTree_debug(size_t level, const char * indexName);

/* Other functions that you may find useful
 * change extension
 */
void replaceExtensionByIdx(const char * fileName, char * indexName);

#endif /* CODE_V2_UTILS_H */

int no_deleted_registers = NO_DELETED_REGISTERS;

unsigned long max_nodeId = 0;

void get_max_nodeId(unsigned long n) {
    if(n> max_nodeId) max_nodeId = n;
}
bool check_dat(const char* filename) {
    size_t len = strlen(filename);
    if(filename == NULL || !(filename[len-3] == 'd' && filename[len-2] == 'a' && filename[len-1] == 't'))
        return false;
    return true;
}
void replaceExtensionByIdx(const char *fileName, char * indexName) {
    size_t len = strlen(fileName);
    strcpy(indexName,fileName);
    indexName[len-3] = 'i';
    indexName[len-2] = 'd';
    indexName[len-1] = 'x';
    return;
}


bool createTable(const char *tableName) {
    FILE *f = NULL;
    int a = -1;
    bool st;

    if (check_dat(tableName) == false)
        return false;
    
    // Intentar abrir el archivo en modo de lectura/escritura
    f = fopen(tableName, "rb+");
    if (f != NULL) {
        printf("Archivo ya existe\n");
        fclose(f);
    }
    // Crear el archivo si no existe
    else{
        if ((f = fopen(tableName, "wb+")) == NULL)
            return false;
        fwrite(&a, sizeof(int), 1, f);
        fclose(f);
    } 
    st = createIndex(tableName);
    return st;
}

bool createIndex(const char *indexName) {
    FILE *f = NULL;
    int a = -1;
    char *indexName_Rep;
    if (!indexName)
        return false;

    indexName_Rep = (char *)malloc((strlen(indexName) + 1) * sizeof(char));

    if (indexName_Rep == NULL)
        return false;

    replaceExtensionByIdx(indexName, indexName_Rep);

    // Intentar abrir el archivo en modo de lectura/escritura
    if ((f = fopen(indexName_Rep, "rb+")) != NULL) {
        printf("Índice ya existe\n");
        fclose(f);
        free(indexName_Rep);
        return true; // Si el índice ya existe, devolvemos true
    }

    // Crear el índice si no existe
    if ((f = fopen(indexName_Rep, "wb+")) == NULL) {
        free(indexName_Rep);
        return false;
    }

    fwrite((void *)&a, sizeof(int), 1, f);
    fwrite((void *)&a, sizeof(int), 1, f);
    fclose(f);

    free(indexName_Rep);
    return true;
}

void printnode_debug(size_t _level, size_t level, FILE *indexFileHandler, int node_id, char side) {
    Node currentNode;
    size_t i;
    char key[5];

    if (node_id == -1 || _level > level) {
        return; 
    }

    // Log: Starting to process node
    printf("[DEBUG] Processing node ID: %d at level: %zu\n", node_id, _level);

    fseek(indexFileHandler, sizeof(int) * 2, SEEK_SET);
    if (fseek(indexFileHandler, node_id * (sizeof(int) * 4 + sizeof(char) * 4), SEEK_CUR) != 0) {
        printf("ERROR: Unable to seek node\n");
        return;
    }

    fread(key, sizeof(char), 4, indexFileHandler);
    fread(&currentNode.left, sizeof(int), 1, indexFileHandler);
    fread(&currentNode.right, sizeof(int), 1, indexFileHandler);
    fread(&currentNode.parent, sizeof(int), 1, indexFileHandler);
    fread(&currentNode.offset, sizeof(int), 1, indexFileHandler);

    for (i = 0; i < _level; i++) {
        printf("\t"); 
    }
    key[4] = '\0';
    if (side != '\0')
        printf("%c %s (%d): %d\n", side, key, node_id, currentNode.offset);
    else
        printf("%s (%d): %d\n", key, node_id, currentNode.offset);

    // Log: Node details
    printf("[DEBUG] Node Details - Key: %s, Left: %d, Right: %d, Parent: %d, Offset: %d\n",
           key, currentNode.left, currentNode.right, currentNode.parent, currentNode.offset);

    printnode_debug(_level + 1, level, indexFileHandler, currentNode.left, 'l'); /* left*/
    printnode_debug(_level + 1, level, indexFileHandler, currentNode.right, 'r'); /* right*/
    return;
}

void printTree_debug(size_t level, const char *indexName) {
    FILE *f;
    int aux;

    if (indexName == NULL) {
        printf("[DEBUG] Index name is NULL\n");
        return;
    }

    f = fopen(indexName, "rb");
    if (f == NULL) {
        printf("[DEBUG] Unable to open index file: %s\n", indexName);
        return;
    }

    fread(&aux, sizeof(int), 1, f);
    fread(&aux, sizeof(int), 1, f);

    printf("[DEBUG] Starting tree print for index: %s\n", indexName);
    printnode_debug(0, level, f, 0, '\0'); 

    fclose(f);
    printf("[DEBUG] Finished tree print\n");
    return;
}

bool findKey(const char * book_id, const char *indexName,
             int * nodeIDOrDataOffset)
 {
    FILE *f;
    int currentOffset = 0, cmp;
    int last_node;
    Node currentNode;
    
    if((f = fopen(indexName,"rb+")) == NULL)
        return false;
    
    fread(&currentOffset,sizeof(int),1,f);
    fseek(f,sizeof(int)*2,SEEK_SET);
    
    while (currentOffset != -1) {

    if(fseek(f,currentOffset * sizeof(Node) + sizeof(int)*2, SEEK_SET) !=0) {
        printf("ERROR al buscar nodo\n");
        fclose(f);
        return false;
    }
        fread(currentNode.book_id,sizeof(char),4,f);
        fread(&currentNode.left,sizeof(int),1,f);
        fread(&currentNode.right,sizeof(int),1,f);
        fread(&currentNode.parent,sizeof(int),1,f);
        fread(&currentNode.offset,sizeof(int),1,f);

        cmp = strncmp(book_id,currentNode.book_id,PK_SIZE);
        if(!cmp) {
            *nodeIDOrDataOffset = currentNode.offset;
            fclose(f);
            return true;
        }

        if(cmp < 0) {
            last_node = currentOffset;
            currentOffset = currentNode.left;
        }    
        else {
            last_node = currentOffset;
            currentOffset = currentNode.right;
        }
    }

    *nodeIDOrDataOffset = last_node;
    fclose(f);
    return false;
 }

void node_read(Node *node, FILE *f) {
    char book_id[4];
    int left, right, parent, offset;
    if(!f || !node)
        return;
    
    fread(book_id,sizeof(char)*PK_SIZE,1,f);
    fread(&left,sizeof(int),1,f);
    fread(&right,sizeof(int),1,f);
    fread(&parent,sizeof(int),1,f);
    fread(&offset,sizeof(int),1,f);

    node->left = left;
}

bool Tree_insert(Node *node, const char *indexName){
    Node parent,deletedNode;
    FILE *f;
    int cmp;
    int root, deleted,next_deleted;
    if(!node || !indexName)
        return false;
    
    if(!(f = fopen(indexName,"r+b"))) {
        return false;
    }

    fseek(f,0,SEEK_SET);
    fread(&root,sizeof(int),1,f);
    fread(&deleted,sizeof(int),1,f);

    if(deleted != -1) {
        fseek(f,4*sizeof(Node) + sizeof(int)*2,SEEK_SET);
        node_read(&deletedNode,f);
        
        next_deleted = deletedNode.left;

        fseek(f,deleted*sizeof(Node) + sizeof(int)*2,SEEK_SET);
        
        fwrite(node->book_id,PK_SIZE *sizeof(char),1,f);
        fwrite(&node->left,sizeof(int),1,f);
        fwrite(&node->right,sizeof(int),1,f);
        fwrite(&node->parent,sizeof(int),1,f);
        fwrite(&node->offset,sizeof(int),1,f);

        

        fseek(f,0,SEEK_SET);
        fread(&root,sizeof(int),1,f);
        fwrite(&next_deleted,sizeof(int),1,f); /*siguiente registro borrado*/

        if(fseek(f,node->parent * sizeof(Node) + sizeof(int)*2, SEEK_SET) !=0) {
            printf("ERROR al buscar nodo\n");
            fclose(f);
            return false;
        }
        fread(parent.book_id,sizeof(char),4,f);
        
        cmp = strncmp(node->book_id,parent.book_id,PK_SIZE);
        if(cmp < 0){
            parent.left = deleted;
            fwrite(&parent.left,sizeof(int),1,f);

        }
        else if(cmp > 0) {
            fread(&parent.left,sizeof(int),1,f);
            parent.right = deleted;
            fwrite(&parent.right,sizeof(int),1,f);
        }

    }

   else {
        max_nodeId++;

        fseek(f,0,SEEK_END);
        fwrite(node->book_id,PK_SIZE *sizeof(char),1,f);
        fwrite(&node->left,sizeof(int),1,f);
        fwrite(&node->right,sizeof(int),1,f);
        fwrite(&node->parent,sizeof(int),1,f);
        fwrite(&node->offset,sizeof(int),1,f);

        if(fseek(f,node->parent * sizeof(Node) + sizeof(int)*2, SEEK_SET) !=0) {
            printf("ERROR al buscar nodo\n");
                fclose(f);
            return false;
        }
        fread(parent.book_id,sizeof(char),4,f);
        
        cmp = strncmp(node->book_id,parent.book_id,PK_SIZE);
        if(cmp < 0){
            parent.left = max_nodeId;
            fwrite(&parent.left,sizeof(int),1,f);

        }
        else if(cmp > 0) {
            fread(&parent.left,sizeof(int),1,f);
            parent.right = max_nodeId;
            fwrite(&parent.right,sizeof(int),1,f);
        }
   }



    fclose(f);
    return true;

}

bool addIndexEntry(char * book_id,  int bookOffset, char const * indexName) {
    int parentOffset;
    Node *node;

    bool st;

    if(book_id == NULL || bookOffset < 0 || indexName == NULL)
        return false;


    if(findKey(book_id,indexName,&parentOffset) == true) {
        return false;
    }
    if(!(node = (Node*) malloc(sizeof(Node))))
        return false;

    strncpy(node->book_id,book_id,PK_SIZE);
    node->left = node->right = -1;
    node->parent = parentOffset;
    node->offset = bookOffset;
    st = Tree_insert(node,indexName);

    free(node);
    return st;
}

// bool addTableEntry(Book * book, const char * dataName,
//                    const char * indexName) {
    
//     FILE *f;
//     int offset = -1;
//     int lenTitle;
//     int deleted;
//     Book aux_book;
//     printf("%s\n",book->book_id);
//     if((f = fopen(dataName,"ab")) == NULL) {
//         printf("Error al abrir archivo\n");
//         return false;
//     }
    
//     if(findKey(book->book_id,indexName, &offset) == true) {
//         printf("El libro ya existe\n");
//         fclose(f);
//         return false;
//     }
//     /*comprobar si hay registro borrado*/

//     fseek(f,sizeof(int),SEEK_SET);
//     fread(&deleted,sizeof(int),1,f);

//     if(deleted != -1) {
//         fseek(f,deleted*sizeof(Book) + 2*sizeof(int),SEEK_SET);
//         fread(&aux_book.book_id,sizeof(char)*PK_SIZE,4,f);
        
//         fclose(f);
//         return true;
//     }
//     /*************/

//     fseek(f,0,SEEK_END);
//     lenTitle = strlen (book->title ) ;
//     fwrite ( book->book_id , PK_SIZE, 1 , f ) ;
//     fwrite ( &lenTitle ,sizeof ( int ) , 1 , f ) ;
//     fwrite ( book->title ,lenTitle , 1 , f ) ;
//     fclose(f);

//     addIndexEntry(book->book_id,offset,indexName);
//     return true;
// }


bool addTableEntry(Book *book, const char *tableName, const char *indexName) {
    FILE *dataFile;
    int deletedOffset;
    int bookOffset, found;
    int titleLen;
    int nextDeletedOffset;

    dataFile = fopen(tableName, "r+b");
    if (!dataFile) return 0;

    fseek(dataFile, 0, SEEK_SET);
    fread(&deletedOffset, sizeof(int), 1, dataFile);

    titleLen = strlen(book->title);
    found = 0;

    if (deletedOffset == -1) {
        fseek(dataFile, 0, SEEK_END);
        bookOffset = ftell(dataFile);
    } else {
        while (deletedOffset != -1) {
            fseek(dataFile, deletedOffset + PK_SIZE + sizeof(int), SEEK_SET);
            fread(&nextDeletedOffset, sizeof(int), 1, dataFile);

            if (nextDeletedOffset >= titleLen) {
                bookOffset = deletedOffset;
                fseek(dataFile, deletedOffset + PK_SIZE + sizeof(int), SEEK_SET);
                fread(&deletedOffset, sizeof(int), 1, dataFile);
                fseek(dataFile, 0, SEEK_SET);
                fwrite(&deletedOffset, sizeof(int), 1, dataFile);
                found = 1;
                break;
            } else {
                deletedOffset = nextDeletedOffset;
            }
        }

        if (!found) {
            fseek(dataFile, 0, SEEK_END);
            bookOffset = ftell(dataFile);
        }
    }

    fseek(dataFile, bookOffset, SEEK_SET);
    fwrite(book->book_id, PK_SIZE, 1, dataFile);
    fwrite(&titleLen, sizeof(int), 1, dataFile);
    fwrite(book->title, titleLen, 1, dataFile);

    fclose(dataFile);
    return addIndexEntry(book->book_id, bookOffset, indexName);
}

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
                    printTree_debug(nivelMaximo, indiceActual);
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
                    return;
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
                    return;
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
