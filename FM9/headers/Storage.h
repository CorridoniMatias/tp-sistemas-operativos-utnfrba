#ifndef STORAGE_H_
#define STORAGE_H_

#include <stdio.h>
#include "bibliotecaFM9.h"
#include "kemmens/logger.h"


//FM9 General Errors:

#define INVALID_WRITE	-1
#define INVALID_READ	-2

int tamanioLinea;
int cantLineas;

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

//Requerimiento: Guardar datos en memoria
/*
 * 	Parámetros: [Data, NumLinea]
 *
 *	Descripción: Ante un pedido de guardado en el storage, el FM9 guardará los bytes de Data en la linea NumLinea.
 *
 *	Retorna: si se pudo escribir 1, caso contrario INVALID_WRITE
 */
int writeLine(void* data, int numLinea);

//Requerimiento: Leer datos de memoria
/*
 * 	Parámetros: [Target, NumLinea]
 *
 *	Descripción: Ante un pedido de lectura desde el storage, la linea numLinea y lo leido lo copia en target.
 *
 *	Retorna: si se pudo leer 1, caso contrario INVALID_READ
 */
int readLine(void* target, int numLinea);

#endif /* STORAGE_H_ */
