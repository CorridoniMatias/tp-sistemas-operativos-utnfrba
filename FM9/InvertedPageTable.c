#include "headers/InvertedPageTable.h"

void createIPTStructures() {

	createPagingStructures();
	pagesPerDTBTable = dictionary_create();

	IPTable = calloc(cantFrames, sizeof(IPTEntry));
	IPTEntry entry = { .dtbId = 0, .page = 0 };
	for (int i = 0; i < cantFrames; i++) {
		IPTable[i] = entry;

//		Logger_Log(LOG_DEBUG, "FM9 -> Entrada de tabla de páginas invertida %d creada.", i);
	}
	Logger_Log(LOG_INFO, "FM9 -> Estructuras de tabla de páginas invertida creadas.");
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
	freePagingStructures();
//	Logger_Log(LOG_INFO, "FM9 -> Tabla de páginas invertida liberada.");
	Logger_Log(LOG_INFO, "FM9 -> Estructuras de tabla de páginas invertida liberadas.");
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
	pages->numberOfPages = 0;
	int offset = 0;
	while (!list_is_empty(freeFrames)) {
		int* frameNumber = list_remove(freeFrames, 0);
//		if (writeFrame(data + offset, *frameNumber) <= 0)
//			return ITS_A_TRAP;
//		printf("\n\n\nnumero de frame %d\n\n\n",*frameNumber);
		//Para solucionar la fragmentacion interna en la ultima pagina
		char* buffer = calloc(1, tamanioFrame);
		if(size - offset < tamanioFrame){
//			printf("\n\n\nhay fragmentacion interna\n\n\n");
			memcpy(buffer,data+offset,size-offset);
		}
		else
			memcpy(buffer,data+offset,tamanioFrame);
		if (writeFrame(buffer, *frameNumber) <= 0)
			return ITS_A_TRAP;
		free(buffer);
		offset += tamanioFrame;
		pages->numberOfPages++;
		updateIPTable(*frameNumber, paginas->nextPageNumber++, dtbID);
		free(frameNumber);
	}
	char* pageKey = string_itoa(pages->firstPage);
	dictionary_put(paginas->pagesPerFiles, pageKey, pages);
	free(pageKey);
	return pages->firstPage * cantLineasPorFrame;
}

int readData_TPI(void** target, int logicalAddress, int dtbID) {
	char* dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(pagesPerDTBTable, dtbKey))
	{	free(dtbKey);
		return ITS_A_TRAP;
	}
	t_pages* paginas = dictionary_get(pagesPerDTBTable, dtbKey);
	free(dtbKey);
	int pageNumber = logicalAddress / cantLineasPorFrame;
	char* pageKey = string_itoa(pageNumber);
	if (!dictionary_has_key(paginas->pagesPerFiles, pageKey))
	{
		free(pageKey);
		return ITS_A_TRAP;
	}
	t_pages_per_file* pages = dictionary_get(paginas->pagesPerFiles, pageKey);
	free(pageKey);
	int initialPage = pages->firstPage, frameNumber, offset = 0, sizeRead = 0;
	*target = malloc(1);
	int result;
	while (offset < pages->numberOfPages) {
		frameNumber = getFrameOfPage(initialPage + offset, dtbID);
		*target = realloc(*target, sizeRead + tamanioFrame);
		result = readFrame(*target + sizeRead, frameNumber);
		if(result== INVALID_LINE_NUMBER || result == INVALID_FRAME_NUMBER)
		{
			return ITS_A_TRAP;
		}
		sizeRead += tamanioFrame;
		offset++;
	}
	return sizeRead;
}

int closeFile_TPI(int dtbID, int logicalAddress) {
	char* dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(pagesPerDTBTable, dtbKey)) {
		free(dtbKey);
		return ITS_A_TRAP;
	}
	t_pages* paginas = dictionary_get(pagesPerDTBTable, dtbKey);
	free(dtbKey);
	int pageNumber = logicalAddress / cantLineasPorFrame;
	char* pageKey = string_itoa(pageNumber);
	if (!dictionary_has_key(paginas->pagesPerFiles, pageKey)) {
		free(pageKey);
		return ITS_A_TRAP;
	}
	t_pages_per_file* pages = dictionary_get(paginas->pagesPerFiles, pageKey);
	freeFrames(pages, dtbID);
	dictionary_remove_and_destroy(paginas->pagesPerFiles, pageKey, free);
	free(pageKey);
	return 1;
}


int closeDTBFiles_TPI(int dtbID) {
	char* dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(pagesPerDTBTable, dtbKey)) {
		free(dtbKey);
		return ITS_A_TRAP;
	}
	t_pages* paginas = dictionary_get(pagesPerDTBTable, dtbKey);
	free(dtbKey);
	void dictionaryDestroyer(void* data){
		t_pages_per_file* file = data;
		freeFrames(file,dtbID);
		free(file);
	}
	dictionary_destroy_and_destroy_elements(paginas->pagesPerFiles,dictionaryDestroyer);
	free(paginas);
	return 1;
}

int dump_TPI(int dtbID) {
	char* dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(pagesPerDTBTable, dtbKey))
		return -ITS_A_TRAP;

	t_pages* paginas = dictionary_get(pagesPerDTBTable, dtbKey);
	free(dtbKey);
//	Logger_Log(LOG_INFO, "G.DT %d", dtbID);
	Logger_Log(LOG_INFO, "Número próxima página %d", paginas->nextPageNumber);
	int i = 0;
	void pageDumper(char* key, void * data) {
		t_pages_per_file* pages = data;
		int offset = 0;
		int frameNumber = getFrameOfPage(pages->firstPage + offset, dtbID);
		Logger_Log(LOG_INFO,
				"Archivo número %d : Página inicial = %d : Cantidad de páginas usadas = %d",
				i, pages->firstPage, pages->numberOfPages);
		char* buffer = malloc(tamanioFrame);
		Logger_Log(LOG_INFO, "Contenido Archivo %d", i);
		while (offset < pages->numberOfPages) {
			frameNumber = getFrameOfPage(pages->firstPage + offset, dtbID);
			Logger_Log(LOG_INFO, "Página número %d está en frame %d", pages->firstPage + offset, frameNumber);
//			Logger_Log(LOG_INFO, "Contenido  %d",  pages->firstPage + offset);
			void* buffer = calloc(1, tamanioFrame + 1);
			readFrame(buffer,frameNumber);
			Logger_Log(LOG_INFO, "Contenido\n%s", buffer);
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
//	Logger_Log(LOG_INFO, "FM9 -> Dirección lógica = %d.", logicalAddress);
//	Logger_Log(LOG_INFO, "FM9 -> Dirección física = %d.", lineNum);
	return lineNum;
}

void updateIPTable(int frameNumber, int pageNumber, int dtbID) {
	IPTable[frameNumber].dtbId = dtbID;
	IPTable[frameNumber].page = pageNumber;
}

int getFrameOfPage(int page, int dtbID) {
	for (int i = 0; i < cantFrames; i++) {
		if (IPTable[i].dtbId == dtbID && IPTable[i].page == page)
			return i;
	}
	return ITS_A_TRAP;
}

