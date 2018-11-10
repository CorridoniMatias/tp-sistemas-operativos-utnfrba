#ifndef STORAGE_H_
#define STORAGE_H_

#include <stdio.h>
#include "bibliotecaFM9.h"
#include "kemmens/logger.h"


//FM9 General Errors:

#define INVALID_WRITE	-1
#define INVALID_READ	-2

size_t tamanioLinea;
size_t cantLineas;

char** storage;

//Requerimiento: Inicializa Storage/Memoria Real
/*
 *
 * 		Descripción: Inicializa la variable storage con el tamaño de linea y la cantidad de lineas especificadas
 * 		en el archivo de configuración.
 */
void createStorage();

//Requerimiento: Liberar Storage/Memoria Real
/*
 *
 * 		Descripción: Libera toda la memoria ocupada por el storage.
 */
void freeStorage();

//Requerimiento: Guardar Datos
/*
 * 	Parámetros: [Data, NumLinea, Size]
 *
 *	Descripción: Ante un pedido de guardado en el storage, el FM9 guardará los bytes de Data, definidos por el valor
 *	de Size, a partir de la linea NumLinea.
 *
 *	Retorna:
 */
int writeStorage(void* data, size_t size, size_t numLinea);

int readStorage(void* target, size_t size, size_t offset);

#endif /* STORAGE_H_ */
