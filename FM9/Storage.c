#include "Storage.h"

void createStorage() {
	size_t tamanioLinea = settings->max_linea;
	size_t cantLineas = settings->tamanio / tamanioLinea;

	storage = calloc(cantLineas, sizeof(char*));

	for (int i = 0; i < cantLineas; i++) {
		*(storage + i) = calloc(tamanioLinea, sizeof(char));
	}
}
