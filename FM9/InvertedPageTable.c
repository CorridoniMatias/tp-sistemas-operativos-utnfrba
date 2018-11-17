#include "headers/InvertedPageTable.h"

void createIPTStructures() {

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

void freeIPTStructures(){

	free(IPTable);

	void dictionaryDestroyer(void* DTBPages){
		void listDestroyer(void* pages){
			list_destroy_and_destroy_elements(pages,free);
		}
		dictionary_clean_and_destroy_elements(DTBPages,listDestroyer);
	}

	dictionary_clean_and_destroy_elements(DTBsPages,dictionaryDestroyer);

	Logger_Log(LOG_INFO, "FM9 -> Tabla de páginas invertida liberada.");
}

int saveData(void* data, int size){
	int framesNecesarios = framesNeededAreAvailable(size);
	if(framesNecesarios == INSUFFICIENT_FRAMES_AVAILABLE)
		return INSUFFICIENT_SPACE;


}

