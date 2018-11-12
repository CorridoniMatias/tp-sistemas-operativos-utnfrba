#include "headers/InvertedPageTable.h"

void createStructuresIPT() {

	Logger_Log(LOG_INFO, "FM9 -> Creando tabla de páginas invertida.");

	DTBsPages = dictionary_create();

	IPTable = calloc(cantFrames, sizeof(IPTEntry));
	IPTEntry entry = { .dtbId = 0, .page = 0 };
	for (int i = 0; i < cantFrames; i++) {
		IPTable[i] = entry;

//		Logger_Log(LOG_DEBUG, "FM9 -> Entrada de tabla de páginas invertida %d creada.", i);
	}

	Logger_Log(LOG_INFO, "FM9 -> Se creó tabla de páginas invertida.");
}

void freeStructuresIPT(){

	free(IPTable);

	Logger_Log(LOG_INFO, "FM9 -> Tabla de páginas invertida liberada.");
}

