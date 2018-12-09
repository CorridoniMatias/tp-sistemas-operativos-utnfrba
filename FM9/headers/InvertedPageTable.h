#ifndef INVERTED_PAGE_TABLE_H_
#define INVERTED_PAGE_TABLE_H_

#include <stdio.h>
#include <commons/collections/dictionary.h>
#include "Paging.h"
#include "FM9Errors.h"

typedef struct {
	int firstPage;
	int numberOfPages;
}t_pages_per_file;


typedef struct {
	int dtbId;
	int page;
} IPTEntry;

IPTEntry* IPTable;

typedef struct{
	t_dictionary * pagesPerFiles;
	int nextPageNumber;
}t_pages;


/*El diccionario pagesPerDTBTable es un diccionario que cada key es la id de un DTB y el valor es
 * una t_pages con un diccionario que cada key es la dirección lógica del archivo y los valores
 * son una t_pages_per_file con la pagina inicial y la cantidad de páginas.
 */
t_dictionary* pagesPerDTBTable;

//Requerimiento: Inicializa la variable IPTable
/*
 * 		Descripción: Inicializa la variable IPTable con la cantidad de frames que existen.
 */
void createIPTStructures();

//Requerimiento: Liberar la tabla de páginas invertida y el diccionario con los procesos, sus archivos
/* y páginas
 *
 * 		Descripción: Libera la memoria ocupada por la tabla de páginas invertida.
 */
void freeIPTStructures();


//Requerimiento: Guardar datos usando la Tabla de Páginas Invertida
/*
 * 	Parámetros: [Data, Size]
 *
 *	Descripción: Ante un pedido de guardado de información, se copia el contenido de data en los distintos
 *	frames disponibles y se actualizan las estructuras administrativas.
 *
 *	Retorna: si se pudo leer retorna la dirección lógica perteneciente a la primera página para poder
 *	acceder posteriormente. Si no hubo espacio suficiente retorna INSUFFICIENT_SPACE.
 */

int addressTranslation_TPI(int logicalAddress, int dtbID);

int writeData_TPI(void* data, int size, int dtbID);

int readData_TPI(void** target, int logicalAddress, int dtbID);

int closeFile_TPI(int dtbID, int logicalAddress) ;

int dump_TPI(int dtbID);

void freeFrames(t_pages_per_file* pages, int dtbID);

void updateIPTable(int frameNumber, int pageNumber, int dtbID);

int getFrameOfPage(int page, int dtbID);

#endif /* INVERTED_PAGE_TABLE_H_ */
