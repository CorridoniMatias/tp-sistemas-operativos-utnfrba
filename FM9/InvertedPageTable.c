#include "headers/InvertedPageTable.h"

void createIPTStructures() {

	createPagingStructures();

	Logger_Log(LOG_INFO, "FM9 -> Creando tabla de páginas invertida.");

	pagesPerDTBTable = dictionary_create();

	IPTable = calloc(cantFrames, sizeof(IPTEntry));
	IPTEntry entry = { .dtbId = 0, .page = 0 };
	for (int i = 0; i < cantFrames; i++) {
		IPTable[i] = entry;

//		Logger_Log(LOG_DEBUG, "FM9 -> Entrada de tabla de páginas invertida %d creada.", i);
	}

	Logger_Log(LOG_INFO, "FM9 -> Se creó tabla de páginas invertida.");
}

void freeIPTStructures() {

	free(IPTable);

	void dictionaryDestroyer(void* DTBPages) {
		void listDestroyer(void* pages) {
			list_destroy_and_destroy_elements(pages, free);
		}
		dictionary_clean_and_destroy_elements(DTBPages, listDestroyer);
	}

	dictionary_clean_and_destroy_elements(pagesPerDTBTable,
			dictionaryDestroyer);

	Logger_Log(LOG_INFO, "FM9 -> Tabla de páginas invertida liberada.");
}

int writeData_TPI(void* data, int size, int dtbID) {
	char* dtbKey = string_itoa(dtbID);
	t_pages* paginas;
	if (!dictionary_has_key(pagesPerDTBTable, dtbKey)) {
		paginas = malloc(sizeof(t_pages));
		paginas->pagesPerFiles = dictionary_create();
		paginas->nextPageNumber = 0;
		dictionary_put(pagesPerDTBTable, dtbKey, paginas);
	} else
		paginas = dictionary_get(pagesPerDTBTable, dtbKey);
	free(dtbKey);

	t_list* freeFrames = getFreeFrames(size);
	if (freeFrames == NULL)
		return INSUFFICIENT_SPACE;
	//Conseguir en que páginas empiezan los datos
	t_pages_per_file* pages = malloc(sizeof(t_pages_per_file));
	pages->firstPage = paginas->nextPageNumber;
	pages->numberOfPages = 1;
	int offset = 0;
	while (!list_is_empty(freeFrames)) {
		int frameNumber = *((int*) list_remove(freeFrames, 0));
		if (writeFrame(data + offset, frameNumber) == INVALID_LINE_NUMBER)
			return INVALID_LINE_NUMBER;
		offset += tamanioFrame;
		pages->numberOfPages++;
		updateIPTable(frameNumber, paginas->nextPageNumber++, dtbID);
	}
	char* pageKey = string_itoa(pages->firstPage);
	dictionary_put(paginas->pagesPerFiles, pageKey, pages);
	free(pageKey);
	return pages->firstPage * cantLineasPorFrame;
}

int readData_TPI(void* target, int logicalAddress, int dtbID) {
	char* dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(pagesPerDTBTable, dtbKey))
		return ITS_A_TRAP;
	t_pages* paginas = dictionary_get(pagesPerDTBTable, dtbKey);
	free(dtbKey);
	int pageNumber = logicalAddress / cantLineasPorFrame;
	char* pageKey = string_itoa(pageNumber);
	if (!dictionary_has_key(paginas->pagesPerFiles, pageKey))
		return ITS_A_TRAP;
	t_pages_per_file* pages = dictionary_get(paginas->pagesPerFiles, pageKey);
	free(pageKey);
	int initialPage = pages->firstPage, frameNumber, offset = 0, sizeRead = 0;
	target = malloc(1);
	while (offset < pages->numberOfPages) {
		frameNumber = getFrameOfPage(initialPage + offset, dtbID);
		realloc(target, sizeRead + tamanioFrame);
		if (readFrame(target + sizeRead, frameNumber) == INVALID_LINE_NUMBER)
			break;
		sizeRead += tamanioFrame;
		offset++;
	}
	return sizeRead;
}

int closeFile_TPI(int dtbID, int logicalAddress) {
	char* dtbKey = string_itoa(dtbID);
	if (dictionary_has_key(pagesPerDTBTable, dtbKey))
		return ITS_A_TRAP;
	t_pages* paginas = dictionary_get(pagesPerDTBTable, dtbKey);
	free(dtbKey);
	int pageNumber = logicalAddress / cantLineasPorFrame;
	char* pageKey = string_itoa(pageNumber);
	if (!dictionary_has_key(paginas->pagesPerFiles, pageKey))
		return ITS_A_TRAP;
	t_pages_per_file* pages = dictionary_get(paginas->pagesPerFiles, pageKey);
	freeFrames(pages, dtbID);
	dictionary_remove_and_destroy(paginas->pagesPerFiles, pageKey, free);
	free(pageKey);
	return 1;
}

int dump_TPI(int dtbID) {
	char* dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(pagesPerDTBTable, dtbKey))
		return -1;

	t_pages* paginas = dictionary_get(pagesPerDTBTable, dtbKey);
	free(dtbKey);
	Logger_Log(LOG_INFO, "Proceso %d", dtbID);
	int i = 0;
	void pageDumper(char* key, void * data) {
		t_pages_per_file* pages = data;
		int offset = 0;
		int frameNumber = getFrameOfPage(pages->firstPage + offset);
		Logger_Log(LOG_INFO,
				"Archivo número %d : Página inicial = %d : Cantidad de páginas usadas = %d",
				i, pages->firstPage, pages->numberOfPages);
		char* buffer = malloc(tamanioFrame);
		Logger_Log(LOG_INFO, "Contenido Archivo %d", i);
		while (offset < pages->numberOfPages) {
			frameNumber = getFrameOfPage(pages->firstPage + offset, dtbID);
			readFrame(buffer, frameNumber);
			offset++;
		}
		free(buffer);
	}
	dictionary_iterator(paginas->pagesPerFiles, pageDumper);
	return 1;

}

void freeFrames(t_pages_per_file* pages, int dtbID) {
	int offset = 0;
	while (offset < pages->numberOfPages) {
		int frameNumber = getFrameOfPage(pages->firstPage + offset, dtbID);
		if (frameNumber == ITS_A_TRAP)
			continue;
		addFreeFrame(frameNumber);
		offset++;
	}
}

int addressTranslation_TPI(int logicalAddress, int dtbID) {
	int lineNum = getFrameOfPage(logicalAddress / cantLineasPorFrame, dtbID);
	if (lineNum == ITS_A_TRAP)
		return ITS_A_TRAP;
	lineNum *= cantLineasPorFrame;
	int offset = logicalAddress % cantLineasPorFrame;
	lineNum += offset;
	Logger_Log(LOG_INFO, "FM9 -> Dirección lógica = %d.", logicalAddress);
	Logger_Log(LOG_INFO, "FM9 -> Dirección física = %d.", lineNum);

	return lineNum;
}

void updateIPTable(int frameNumber, int pageNumber, int dtbID) {
	IPTable[frameNumber]->dtbId = dtbID;
	IPTable[frameNumber]->page = pageNumber;
}

int getFrameOfPage(int page, int dtbID) {
	for (int i = 0; i < cantFrames; i++) {
		if (IPTable[i]->dtbId == dtbID && IPTable[i]->page == page)
			return i;
	}
	return ITS_A_TRAP;
}

