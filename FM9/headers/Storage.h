#ifndef STORAGE_H_
#define STORAGE_H_

#include <stdio.h>
#include "kemmens/logger.h"
#include "FM9lib.h"
#include "FM9Errors.h"
#include <pthread.h>

#define verifyLineNumber(numLinea)					\
	if (numLinea >= cantLineas || numLinea < 0)		\
	{	return INVALID_LINE_NUMBER;}

int tamanioLinea;
int cantLineas;
char** storage;
pthread_mutex_t storageLock;
bool loggLines;

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
 *	Retorna: si se pudo escribir 1, caso contrario INVALID_LINE_NUMBER
 */
int writeLine(void* data, int numLinea);

//Requerimiento: Leer datos de memoria
/*
 * 	Parámetros: [Target, NumLinea]
 *
 *	Descripción: Ante un pedido de lectura desde el storage, la linea numLinea y lo leido lo copia en target.
 *
 *	Retorna: si se pudo leer 1, caso contrario INVALID_LINE_NUMBER
 */
int readLine(void* target, int numLinea);

#endif /* STORAGE_H_ */
