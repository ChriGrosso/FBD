#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*
 * @brief Variable global que almacena el número de registros eliminados.
 */
int no_deleted_registers = NO_DELETED_REGISTERS;

/*
 * @brief Variable global que guarda el ID máximo de nodo en el árbol.
 */
unsigned long max_nodeId = 0;

/*
 * @brief Actualiza la variable global `max_nodeId` si el valor proporcionado es mayor.
 * 
 * @param n Nuevo ID de nodo que se compara con el valor actual de `max_nodeId`.
 */
void get_max_nodeId(unsigned long n) {
    if(n> max_nodeId) max_nodeId = n;
}

/*
 * @brief Verifica si el nombre de archivo proporcionado tiene la extensión `.dat`.
 * 
 * @param filename Nombre del archivo a verificar.
 * @return true si el archivo tiene la extensión `.dat`, false en caso contrario.
 */
bool check_dat(const char* filename) {
    size_t len = strlen(filename);
    if(filename == NULL || !(filename[len-3] == 'd' && filename[len-2] == 'a' && filename[len-1] == 't'))
        return false;
    return true;
}

/*
 * @brief Reemplaza la extensión `.dat` de un archivo por `.idx`.
 * 
 * @param fileName Nombre del archivo original.
 * @param indexName Nombre resultante con la extensión modificada.
 */
void replaceExtensionByIdx(const char *fileName, char * indexName) {
    size_t len = strlen(fileName);
    strcpy(indexName,fileName);
    indexName[len-3] = 'i';
    indexName[len-2] = 'd';
    indexName[len-1] = 'x';
    return;
}

/*
 * @brief Crea un archivo de tabla con el nombre proporcionado, inicializando su contenido.
 * 
 * @param tableName Nombre del archivo de tabla a crear.
 * @return true si se crea o se encuentra la tabla correctamente, false en caso de error.
 */
bool createTable(const char * tableName) {
    FILE *f = NULL;
    int a = -1;
    bool st;
    if(check_dat(tableName) == false)
        return false;
    
    f = fopen(tableName,"rb+");
    if(f != NULL) {
        printf("Archivo ya existe\n");
        fclose(f);
        return true;
    }

    if((f = fopen(tableName,"wb+")) == NULL)
        return false;
        
    fwrite(&a,sizeof(int),1,f );
    
    fclose(f);

    st = createIndex(tableName);

    return st;
    
}

/*
 * @brief Crea un archivo de índice asociado a una tabla, inicializando su estructura.
 * 
 * @param indexName Nombre del archivo de índice a crear.
 * @return true si se crea correctamente, false en caso de error.
 */
bool createIndex(const char *indexName) {
    FILE *f = NULL;
    int a = -1;
    char *indexName_Rep;

    if(!indexName)
        return false;
    
    indexName_Rep = (char*) malloc((strlen(indexName)+1)* sizeof(char));

    if(indexName_Rep == NULL)
        return false;
    
    
    replaceExtensionByIdx(indexName,indexName_Rep);
    
    if((f = fopen(indexName_Rep,"rb+"))) {
        fclose(f);
        free(indexName_Rep);
        return true;
    }

    
    if((f = fopen(indexName_Rep,"wb+")) == NULL) {
        free(indexName_Rep);
        return false;
    }
    fwrite((void*)&a,sizeof(int),1,f );
    fwrite((void*)&a,sizeof(int),1,f);
    fclose(f);

    free(indexName_Rep);
    return true;

}
/*
 * @brief Imprime un nodo específico del árbol binario desde un archivo de índice.
 * 
 * @param _level Nivel actual en el árbol.
 * @param level Nivel máximo a imprimir.
 * @param indexFileHandler Manejador del archivo de índice.
 * @param node_id ID del nodo a imprimir.
 * @param side Indica si el nodo es izquierdo ('l') o derecho ('r').
 */
void printnode(size_t _level, size_t level, FILE * indexFileHandler, int node_id, char side) {

    Node currentNode;
    size_t i;
    char key[5];

    if (node_id == -1 || _level >= level) {
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

/*
 * @brief Imprime el árbol binario hasta un nivel dado desde un archivo de índice.
 * 
 * @param level Nivel máximo a imprimir.
 * @param indexName Nombre del archivo de índice.
 */
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

/*
 * @brief Busca una clave específica en el archivo de índice.
 * 
 * @param book_id ID del libro a buscar.
 * @param indexName Nombre del archivo de índice.
 * @param nodeIDOrDataOffset Dirección para almacenar el resultado de la búsqueda.
 * @return true si encuentra la clave, false si no la encuentra.
 */
bool findKey(const char * book_id, const char *indexName,
             int * nodeIDOrDataOffset)
 {
    FILE *f;
    int currentOffset = 0, cmp;
    int last_node;
    Node currentNode;
    
    if((f = fopen(indexName,"rb+")) == NULL)
        return false;
    
    fseek(f,0,SEEK_SET);
    fread(&currentOffset,sizeof(int),1,f);
    fseek(f,sizeof(int)*2,SEEK_SET);
    
    while (currentOffset != -1) {

    if(fseek(f,currentOffset * sizeof(Node) + sizeof(int)*2, SEEK_SET) != 0) {
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

/*
 * @brief Lee los datos de un nodo desde un archivo y los almacena en una estructura `Node`.
 * 
 * Esta función extrae información de un nodo almacenado en un archivo binario y la carga 
 * en una estructura `Node` proporcionada. Los campos leídos incluyen el ID del libro, 
 * los punteros izquierdo y derecho, el nodo padre, y el offset en el archivo de datos.
 * 
 * @param node Puntero a la estructura `Node` donde se almacenarán los datos leídos.
 * @param f Manejador del archivo binario que contiene los datos del nodo.
 * 
 * @note Si el manejador del archivo o el puntero al nodo son nulos, la función no realiza 
 *       ninguna operación y retorna inmediatamente.
 */
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

/*
 * @brief Inserta un nuevo nodo en el árbol binario representado en un archivo de índice.
 * 
 * @param node Nodo a insertar.
 * @param indexName Nombre del archivo de índice.
 * @return true si se inserta correctamente, false en caso de error.
 */
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
    if(root == -1) {
        
        node->parent = -1;
        fseek(f,0,SEEK_SET);
        fwrite(&max_nodeId,sizeof(int),1,f);
        
        fseek(f,sizeof(int)*2,SEEK_SET);

        fwrite(node->book_id,PK_SIZE *sizeof(char),1,f);
        fwrite(&node->left,sizeof(int),1,f);
        fwrite(&node->right,sizeof(int),1,f);
        fwrite(&node->parent,sizeof(int),1,f);
        fwrite(&node->offset,sizeof(int),1,f);
        fclose(f);
        
        return true;
    }

    if(deleted != -1) {
        fseek(f,deleted*sizeof(Node) + sizeof(int)*2,SEEK_SET);
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

/*
 * @brief Agrega una entrada al índice asociado con la tabla.
 * 
 * @param book_id ID del libro.
 * @param bookOffset Offset del libro en el archivo de datos.
 * @param indexName Nombre del archivo de índice.
 * @return true si se agrega correctamente, false en caso de error.
 */
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

/*
 * @brief Agrega un registro de libro a la tabla y su índice asociado.
 * 
 * @param book Información del libro a agregar.
 * @param dataName Nombre del archivo de datos.
 * @param indexName Nombre del archivo de índice.
 * @return true si se agrega correctamente, false en caso de error.
 */
bool addTableEntry(Book * book, const char * dataName, const char * indexName) {
    
    FILE *f;
    int offset = -1;
    int lenTitle;
    int deleted = -1, next_deleted;
    Book aux_book;
    if((f = fopen(dataName,"ab")) == NULL) {
        printf("Error al abrir archivo\n");
        return false;
    }
    
    if(findKey(book->book_id,indexName, &offset) == true) {
        printf("El libro ya existe\n");
        fclose(f);
        return false;
    }
    /*comprobar si hay registro borrado*/
    /*
    fseek(f,0,SEEK_SET);
    fread(&deleted,sizeof(int),1,f);

    if(deleted != -1) {
        fseek(f,deleted*sizeof(Book) + 2*sizeof(int),SEEK_SET);
        fread(&aux_book.book_id,sizeof(char)*PK_SIZE,4,f);
        
        fclose(f);
        return true;
    }
    */
    /*************/

    fseek(f,0,SEEK_END);
    offset = ftell(f);
    lenTitle = strlen (book->title ) ;
    fwrite ( book->book_id , PK_SIZE, 1 , f ) ;
    fwrite ( &lenTitle ,sizeof ( int ) , 1 , f ) ;
    fwrite ( book->title ,lenTitle , 1 , f ) ;
    fclose(f);

    
    return addIndexEntry(book->book_id,offset,indexName);
}
