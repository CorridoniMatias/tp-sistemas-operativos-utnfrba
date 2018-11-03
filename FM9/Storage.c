#include "headers/Storage.h"

void createStorage() {

	Logger_Log(LOG_INFO, "FM9 -> Creando Storage.");

	tamanioLinea = settings->max_linea;
	cantLineas = settings->tamanio / tamanioLinea;

	storage = calloc(cantLineas, sizeof(char*));

	for (int i = 0; i < cantLineas; i++) {
		*(storage + i) = calloc(tamanioLinea, sizeof(char));
		Logger_Log(LOG_DEBUG, "FM9 -> Linea %d creada.", i);
	}
	Logger_Log(LOG_INFO, "FM9 -> Storage creado.");
}

void freeStorage() {

	for (int i = 0; i < cantLineas; i++) {
		free(*(storage + i));
		Logger_Log(LOG_DEBUG, "FM9 -> Linea %d liberada.", i);
	}

	free(storage);
	Logger_Log(LOG_DEBUG, "FM9 -> Storage liberado.");
}

int writeStorage(void* data, size_t size, size_t offset) {

	size_t cantLineasNecesarias = size / tamanioLinea;
	size_t espacioUltimaLinea = size % tamanioLinea;
	size_t espacioTotalUsado = offset + cantLineasNecesarias - 1;

	if (espacioUltimaLinea > 0 )
		espacioTotalUsado++;
	if(espacioTotalUsado > cantLineas)
		return INSUFFICIENT_SPACE;

	Logger_Log(LOG_DEBUG, "FM9 -> cantLineasNecesarias = %d.", cantLineasNecesarias);
	Logger_Log(LOG_DEBUG, "FM9 -> espacioUltimaLinea = %d.", espacioUltimaLinea);

	int i = 0;
	while (i < cantLineasNecesarias) {
		memcpy(storage[offset + i], data + tamanioLinea * i, tamanioLinea);
		Logger_Log(LOG_DEBUG, "FM9 -> guardadoEnStorage = %s -- En linea = %d.", storage[offset + i], offset + i);
		i++;
	}
	if (espacioUltimaLinea > 0) {
		memcpy(storage[offset + i], data + tamanioLinea * i, espacioUltimaLinea);

		Logger_Log(LOG_DEBUG, "FM9 -> guardadoEnStorage = %s -- En linea = %d.", storage[offset + i], offset + i);
	}

	return 1;
}

int readStorage(void* target, size_t size, size_t offset)
{
	size_t cantLineasALeer = size / tamanioLinea;
	size_t espacioUltimaLinea = size % tamanioLinea;
	size_t espacioTotalUsado = offset + cantLineasALeer - 1;

	if (espacioUltimaLinea > 0 )
		espacioTotalUsado++;
	if(espacioTotalUsado > cantLineas)
		return INSUFFICIENT_DATA;

	Logger_Log(LOG_DEBUG, "FM9 -> cantLineasNecesarias = %d.", cantLineasALeer);
	Logger_Log(LOG_DEBUG, "FM9 -> espacioUltimaLinea = %d.", espacioUltimaLinea);

	int i = 0;
	while (i < cantLineasALeer) {
		memcpy(target + tamanioLinea * i, storage[offset + i], tamanioLinea);
		Logger_Log(LOG_DEBUG, "FM9 -> guardadoEnStorage = %s -- En linea = %d.", storage[offset + i], offset + i);
		Logger_Log(LOG_DEBUG, "FM9 -> guardadoEnTarget = %s -- En linea = %d.", (target + tamanioLinea * i), offset + i);
		i++;
	}
	if (espacioUltimaLinea > 0) {
		memcpy(target + tamanioLinea * i, storage[offset + i], tamanioLinea);

		Logger_Log(LOG_DEBUG, "FM9 -> guardadoEnStorage = %s -- En linea = %d.", storage[offset + i], offset + i);
		Logger_Log(LOG_DEBUG, "FM9 -> guardadoEnTarget = %s -- En linea = %d.", (target + tamanioLinea * i), offset + i);
	}

	return 1;
}
