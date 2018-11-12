#include "headers/Storage.h"

int verifyLineNumber(int numLinea){
	if (numLinea >= cantLineas)
		return INVALID_LINE_NUMBER;
	return 1;
}

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

//	Logger_Log(LOG_DEBUG, "FM9 -> Contenido de Data: %s.", data);

	memcpy(storage[numLinea], data, tamanioLinea);

//	Logger_Log(LOG_DEBUG, "FM9 -> Contenido guardado en Storage = %s - En linea = %d.", storage[numLinea], numLinea);

	Logger_Log(LOG_INFO, "FM9 -> Escritura en Storage exitosa en linea %d.", numLinea);

	return 1;
}

int readLine(void* target, int numLinea) {

	verifyLineNumber(numLinea);

//	Logger_Log(LOG_DEBUG, "FM9 -> Contenido de Storage = %s -- En linea = %d.", storage[numLinea], numLinea);

	memcpy(target, storage[numLinea], tamanioLinea);

//	Logger_Log(LOG_DEBUG, "FM9 -> Contenido guardado en target = %s.", (char*)target);

	Logger_Log(LOG_INFO, "FM9 -> Lectura de Storage exitosa de linea %d.", numLinea);

	return 1;
}
