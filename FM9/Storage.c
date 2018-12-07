#include "headers/Storage.h"

void createStorage() {

	Logger_Log(LOG_INFO, "FM9 -> Creando Storage.");

	tamanioLinea = settings->max_linea;
	cantLineas = settings->tamanio / tamanioLinea;

	storage = calloc(cantLineas, sizeof(char*));

	for (int i = 0; i < cantLineas; i++) {
		*(storage + i) = calloc(tamanioLinea, sizeof(char));
//		Logger_Log(LOG_DEBUG, "FM9 -> Linea %d creada.", i);
	}
	Logger_Log(LOG_INFO, "FM9 -> Storage creado.");
}

void freeStorage() {

	for (int i = 0; i < cantLineas; i++) {
		free(*(storage + i));
//		Logger_Log(LOG_DEBUG, "FM9 -> Linea %d liberada.", i);
	}

	free(storage);
	Logger_Log(LOG_INFO, "FM9 -> Storage liberado.");
}

int writeLine(void* data, int numLinea) {

	verifyLineNumber(numLinea);
	memcpy(storage[numLinea], data, tamanioLinea);
	char* linea = calloc(1,tamanioLinea);

	memcpy(linea, data, tamanioLinea);
	linea = realloc(linea, tamanioLinea + 1);
	memcpy(linea + tamanioLinea, "\0", 1);
	printf("linea = %s.", linea);
	Logger_Log(LOG_DEBUG, "Contenido de Data: %s.", linea);
	free(linea);
//	Logger_Log(LOG_DEBUG, "FM9 -> Contenido guardado en Storage = %s - En linea = %d.", storage[numLinea], numLinea);
//
//	Logger_Log(LOG_INFO, "FM9 -> Escritura en Storage exitosa en linea %d.", numLinea);

	return 1;
}

int readLine(void* target, int numLinea) {

	verifyLineNumber(numLinea)
	char* linea = calloc(1,tamanioLinea);
	memcpy(linea, storage[numLinea], tamanioLinea);
	linea = realloc(linea, tamanioLinea + 1);
	memcpy(linea + tamanioLinea, "\0", 1);

	Logger_Log(LOG_DEBUG, "Contenido de Storage = %s -- En linea = %d.", linea, numLinea);

	memcpy(target, linea, tamanioLinea);

	free(linea);

//	Logger_Log(LOG_INFO, "FM9 -> Lectura de Storage exitosa de linea %d.", numLinea);

	return 1;
}
