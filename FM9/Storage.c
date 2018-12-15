#include "headers/Storage.h"

void createStorage() {

//	Logger_Log(LOG_INFO, "FM9 -> Creando Storage.");

	tamanioLinea = settings->max_linea;
	cantLineas = settings->tamanio / tamanioLinea;

	storage = calloc(cantLineas, sizeof(char*));

	for (int i = 0; i < cantLineas; i++) {
		*(storage + i) = calloc(tamanioLinea, sizeof(char));
//		Logger_Log(LOG_DEBUG, "FM9 -> Linea %d creada.", i);
	}
	loggLines = true;
	pthread_mutex_init(&storageLock, NULL);
	Logger_Log(LOG_INFO, "FM9 -> Storage creado.");
	Logger_Log(LOG_INFO, "FM9 -> Tamaño storage = %d - Tamaño linea = %d - Cantidad de lineas = %d", settings->tamanio, tamanioLinea, cantLineas);
}

void freeStorage() {

	for (int i = 0; i < cantLineas; i++) {
		free(*(storage + i));
//		Logger_Log(LOG_DEBUG, "FM9 -> Linea %d liberada.", i);
	}

	free(storage);
	pthread_mutex_destroy(&storageLock);
	Logger_Log(LOG_INFO, "FM9 -> Storage liberado.");
}

int writeLine(void* data, int numLinea) {

	verifyLineNumber(numLinea);
	pthread_mutex_lock(&storageLock);
	memcpy(storage[numLinea], data, tamanioLinea);
	pthread_mutex_unlock(&storageLock);
//	char* linea = calloc(1,tamanioLinea);
//	memset(linea,0,tamanioLinea);
//	memcpy(linea, data, tamanioLinea);
//	linea = realloc(linea, tamanioLinea + 1);
//	memcpy(linea + tamanioLinea, "\0", 1);
//	printf("linea = %s.", linea);
////	Logger_Log(LOG_DEBUG, "Contenido de Data: %s.", linea);
//	free(linea);
//	Logger_Log(LOG_DEBUG, "FM9 -> Contenido guardado en Storage = %s - En linea = %d.", storage[numLinea], numLinea);
	if(loggLines)
		Logger_Log(LOG_INFO, "FM9 -> Escritura en Storage en linea %d exitosa!", numLinea);

	return 1;
}

int readLine(void* target, int numLinea) {

//	char* linea = NULL;
	verifyLineNumber(numLinea)
//	linea = calloc(1,tamanioLinea);
//	memset(linea,0,tamanioLinea);
	pthread_mutex_lock(&storageLock);
	memcpy(target, storage[numLinea], tamanioLinea);
	pthread_mutex_unlock(&storageLock);
//	linea = realloc(linea, tamanioLinea + 1);
//	memcpy(linea + tamanioLinea, "\0", 1);

//	Logger_Log(LOG_DEBUG, "Contenido de Storage = %s -- En linea = %d.", linea, numLinea);
//	target = memset(target,0,tamanioLinea);
//	memcpy(target, linea, tamanioLinea);

//	printf("\n\ntarget=\"%s\"\n\n",(char*)target);

//	free(linea);
	if(loggLines)
		Logger_Log(LOG_INFO, "FM9 -> Lectura de Storage de linea %d exitosa!", numLinea);

	return 1;
}
