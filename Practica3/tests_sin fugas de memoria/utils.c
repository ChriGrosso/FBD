#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

void printnode(size_t _level, size_t level, FILE * indexFileHandler, int node_id, char side) {

    Node currentNode;
    size_t i;
    char key[5];

    if (node_id == -1 || _level > level) {
        return; 
    }

    get_max_nodeId(node_id);
    fseek(indexFileHandler,sizeof(int)*2,SEEK_SET);
    
    if(fseek(indexFileHandler, node_id * (sizeof(int)*4 + sizeof(char)*4), SEEK_CUR) !=0) {
        printf("ERROR al buscar nodo\n");
        return;
    }


    fread(key,sizeof(char),4,indexFileHandler);
    fread(&currentNode.left,sizeof(int),1,indexFileHandler);
    fread(&currentNode.right,sizeof(int),1,indexFileHandler);
    fread(&currentNode.parent,sizeof(int),1,indexFileHandler);
    fread(&currentNode.offset,sizeof(int),1,indexFileHandler);

    for (i = 0; i < _level; i++) {
        printf("\t"); 
    }
    key[4] = '\0';
    if(side != '\0')
        printf("%c %s (%d): %d\n", side,key, node_id, currentNode.offset);
    else
        printf("%s (%d): %d\n", key, node_id, currentNode.offset);
    
    printnode(_level + 1, level, indexFileHandler, currentNode.left, 'l'); /* left*/
    printnode(_level + 1, level, indexFileHandler, currentNode.right, 'r'); /* right*/
    return;
}

void printTree(size_t level, const char * indexName)
{
    FILE *f;
    int aux;
    if(indexName == NULL)
        return;

    f = fopen(indexName, "rb");
    
    if (f == NULL) {
        return;
    }

    fread(&aux,sizeof(int),1,f);
    fread(&aux,sizeof(int),1,f);

    printnode(0, level, f, 0, '\0'); 
    fclose(f);
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

