#ifndef INVERTED_PAGE_TABLE_H_
#define INVERTED_PAGE_TABLE_H_

#include <stdio.h>
#include <commons/collections/dictionary.h>
#include "Paging.h"

typedef struct {
	int dtbId;
	int page;
} IPTEntry;

IPTEntry* IPTable;

t_dictionary* DTBsPages;

//Requerimiento: Inicializa la variable IPTable
/*
 * 		Descripción: Inicializa la variable IPTable con la cantidad de frames que existen.
 */
void createStructuresIPT();

//Requerimiento: Liberar la tabla de páginas invertida.
/*
 *
 * 		Descripción: Libera la memoria ocupada por la tabla de páginas invertida.
 */
void freeStructuresIPT();

#endif /* INVERTED_PAGE_TABLE_H_ */
