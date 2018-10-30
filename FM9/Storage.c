#include "headers/Storage.h"

void createStorage() {

	tamanioLinea = settings->max_linea;
	cantLineas = settings->tamanio / tamanioLinea;

	storage = calloc(cantLineas, sizeof(char*));

	for (int i = 0; i < cantLineas; i++) {
		*(storage + i) = calloc(tamanioLinea, sizeof(char));
		Logger_Log(LOG_DEBUG, "FM9 -> Linea %d creada.", i);
	}
	Logger_Log(LOG_DEBUG, "FM9 -> Storage creado.");
}

void freeStorage() {

	for (int i = 0; i < cantLineas; i++) {
		free(*(storage + i));
		Logger_Log(LOG_DEBUG, "FM9 -> Linea %d liberada.", i);
	}

	free(storage);
	Logger_Log(LOG_DEBUG, "FM9 -> Storage liberado.");
}

int save(void* data, size_t numLinea, size_t size){
	return -1;
}
