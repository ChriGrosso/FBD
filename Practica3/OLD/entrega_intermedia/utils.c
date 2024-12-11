#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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
    FILE *f = NULL;
    int a = -1;
    
    if(check_dat(tableName) == false)
        return false;
    
    f = fopen(tableName,"rb+");
    if(f != NULL) {
        printf("Archivo ya existe\n");
        fclose(f);
        return false;
    }

    if((f = fopen(tableName,"wb+")) == NULL)
        return false;
        
    fwrite(&a,sizeof(int),1,f );
    fclose(f);
    return createIndex(tableName);
    
}

bool createIndex(const char *indexName) {
    FILE *f = NULL;
    int a = -1;
    char *indexName_Rep = (char*) malloc((strlen(indexName)+1)* sizeof(char));

    if(indexName_Rep == NULL)
        return false;
    replaceExtensionByIdx(indexName,indexName_Rep);


    if((f = fopen(indexName_Rep,"rb+")) == NULL) {
        if((f = fopen(indexName_Rep,"wb+")) == NULL) {
            free(indexName_Rep);
            return false;
        }
        fwrite((void*)&a,sizeof(int),1,f );
        fwrite((void*)&a,sizeof(int),1,f);
        fclose(f);
    }

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
    int root;
    char key[5] = "\0";
    Node currentNode;
    
    printf("%s\n",indexName);
    if((f = fopen(indexName,"rb+")) == NULL)
        return false;
    
    fread(&root,sizeof(int),1,f);
    fseek(f,root,8);
    
    while (currentOffset != -1) {

    if(fseek(f,sizeof(Node), SEEK_CUR) !=0) {
        printf("ERROR al buscar nodo\n");
        return false;
    }
        fread(key,sizeof(char),4,f);
        printf("%s\n",key);
        fread(&currentNode.left,sizeof(int),1,f);
        fread(&currentNode.right,sizeof(int),1,f);
        fread(&currentNode.parent,sizeof(int),1,f);
        fread(&currentNode.offset,sizeof(int),1,f);
        key[4] = '\0';

        cmp = strcmp(book_id,key);
        if(!cmp) {
            *nodeIDOrDataOffset = currentOffset;
            fclose(f);
            return true;
        }

        if(cmp < 0)
            currentOffset = currentNode.left;    
        else
            currentOffset = currentNode.right;
    }

    *nodeIDOrDataOffset = -1;
    fclose(f);
    return false;
 }

bool addIndexEntry(char * book_id,  int bookOffset, char const * indexName) {
    FILE *f;
    if((f = fopen(indexName,"ab+")) == NULL)
        return false;
    
    fseek(f,bookOffset,SEEK_CUR);
    fwrite(book_id,sizeof(char),1,f);
    return true;
}

bool addTableEntry(Book * book, const char * dataName,
                   const char * indexName) {
    
    FILE *f;
    int offset = -1;
    if((f = fopen(dataName,"ab+")) == NULL)
        return false;
    
    if(findKey(book->book_id,indexName, &offset) == true)
        return false;
    /*comprobar si hay registro borrado*/


    /*************/

    int lenTitle = strlen (book->title ) ;
    fwrite ( book->book_id , PK_SIZE, 1 , f ) ;
    fwrite ( &lenTitle ,sizeof ( int ) , 1 , f ) ;
    fwrite ( book->title ,lenTitle , 1 , f ) ;

    addIndexEntry(book->book_id,offset,indexName);
    return true;
}