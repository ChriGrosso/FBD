#include "utils.h"

int no_deleted_registers = NO_DELETED_REGISTERS;

void replaceExtensionByIdx(const char *fileName, char * indexName) {
    return;
}


bool createTable(const char * tableName) {
    return true;
    }

bool createIndex(const char *indexName) {
    return true;
}
void printnode(size_t _level, size_t level, FILE * indexFileHandler, int node_id, char side) {
    if (node_id == -1) {
        return; // Nodo null
    }
    fseek(indexFileHandler, node_id * sizeof(Node), SEEK_SET);
    // Leo Nodo
    Node currentNode;
    size_t i;
    fread(&currentNode, sizeof(Node), 1, indexFileHandler);
    for (i = 0; i < _level; i++) {
        printf("\t"); //Nivel basado en la profundidad
    }
    printf("%c %s (%d): %d\n", side, currentNode.book_id, node_id, currentNode.offset);
    
    printnode(_level + 1, level, indexFileHandler, currentNode.left, 'l'); // left
    printnode(_level + 1, level, indexFileHandler, currentNode.right, 'r'); // right
    return;
}

void printTree(size_t level, const char * indexName)
{
    FILE *f = fopen(indexName, "rb");
    if (f == NULL) {
        return;
    }
    printnode(0, level, f, 0, ' '); // Empieza de la Raiz
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