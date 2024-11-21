#include "utils.h"

int no_deleted_registers = NO_DELETED_REGISTERS;

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


bool createTable(const char * tableName) {
    FILE *f;
    int a = -1;
    
    if(check_dat(tableName) == false)
        return false;
    
    if(f = fopen(tableName,"rb") == NULL) {
        if((f = fopen(tableName,"wb")) == NULL)
            return false;
        
        fwrite(&a,sizeof(int),1,f );
        fclose(f);
        return createIndex(tableName);
    }
    return true;
}

bool createIndex(const char *indexName) {
    FILE *f;
    int a = -1;
    char *indexName_Rep = (char*) malloc((strlen(indexName)+1)* sizeof(char));

    replaceExtensionByIdx(indexName,indexName_Rep);

    if(indexName_Rep == NULL)
        return false;
    if(f = fopen(indexName_Rep,"rb") == NULL) {
        if((f = fopen(indexName_Rep,"wb")) == NULL)
            return false;
        
        fwrite(&a,sizeof(int),2,f );
        fclose(f);
        free(indexName_Rep);
    }
    return true;
}
void printnode(size_t _level, size_t level, FILE * indexFileHandler, int node_id, char side) {
    if (node_id == -1) {
        return; /*Nodo null*/ 
    }
    fseek(indexFileHandler, node_id * sizeof(Node), SEEK_SET);
    // Leo Nodo
    Node currentNode;
    size_t i;
    fread(&currentNode, sizeof(Node), 1, indexFileHandler);
    for (i = 0; i < _level; i++) {
        printf("\t"); /*Nivel basado en la profundidad*/
    }
    printf("%c %s (%d): %d\n", side, currentNode.book_id, node_id, currentNode.offset);
    
    printnode(_level + 1, level, indexFileHandler, currentNode.left, 'l'); /* left*/
    printnode(_level + 1, level, indexFileHandler, currentNode.right, 'r'); /* right*/
    return;
}

void printTree(size_t level, const char * indexName)
{
    FILE *f = fopen(indexName, "rb");
    if (f == NULL) {
        return;
    }
    printnode(0, level, f, 0, ' '); /* Empieza de la Raiz*/
    fclose(f);
    return;
}

bool findKey(const char * book_id, const char *indexName,
             int * nodeIDOrDataOffset)
 {
     return true;
 }

bool addIndexEntry(char * book_id,  int bookOffset, char const * indexName) {
    return true;
}

bool addTableEntry(Book * book, const char * dataName,
                   const char * indexName) {
    return true;
}