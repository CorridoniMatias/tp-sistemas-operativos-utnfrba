#ifndef INVERTED_PAGE_TABLE_H_
#define INVERTED_PAGE_TABLE_H_

#include <stdio.h>
#include <commons/collections/dictionary.h>
#include "Paging.h"

#define INSUFFICIENT_SPACE -1

typedef struct {
	int dtbId;
	int page;
} IPTEntry;

IPTEntry* IPTable;

/*El diccionario DTBsPages es un diccionario que cada key es la id de un DTB y el valor
 * es otro diccionario más, cuyas keys son las direcciones lógicas de cada archivo abierto
 * y el valor es una lista con las paginas en las que esta el archivo.
 */
t_dictionary* DTBsPages;

//Requerimiento: Inicializa la variable IPTable
/*
 * 		Descripción: Inicializa la variable IPTable con la cantidad de frames que existen.
 */
void createStructuresIPT();

//Requerimiento: Liberar la tabla de páginas invertida y el diccionario con los procesos, sus archivos
/* y páginas
 *
 * 		Descripción: Libera la memoria ocupada por la tabla de páginas invertida.
 */
void freeStructuresIPT();


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
int saveData(void* data, int size);

#endif /* INVERTED_PAGE_TABLE_H_ */
