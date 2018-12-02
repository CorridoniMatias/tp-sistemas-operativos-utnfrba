#include "headers/PagedSegmentation.h"

void createPagedSegmentationStructures() {
	createPagingStructures();
	segmentspagedPerDTBTable = dictionary_create();

	offsetNumberOfDigits = 0;

	//Para contar cantidad de digitos para el offset
	int aux = cantFrames * cantLineasPorFrame * 2 - 1;
	while (aux > 0) {
		aux = aux / 10;
		offsetNumberOfDigits++;
	}
}

void freePagedSegmentationStructures() {
	void destroySegments(void* segment) {
		t_segment_paged* segmento = segment;
		dictionary_clean_and_destroy_elements(segmento->pages, free);
		free(segmento);
	}
	void destroyMasterTable(void* segments) {
		t_segments_paged* segmentos = segments;
		dictionary_clean_and_destroy_elements(segmentos->segments,
				destroySegments);
		free(segmentos);
	}
	dictionary_clean_and_destroy_elements(segmentsPerDTBTable,
			destroyMasterTable);
	freePagingStructures();
}

int addressTranslation_SPA(int logicalAddress, int dtbID) {
	int segmentNumber = getSegmentFromAddress(logicalAddress);
	int segmentOffset = getOffsetFromAddress(logicalAddress);
	int pageNumber = segmentOffset / cantLineasPorFrame;
	int frameOffset = segmentOffset % cantLineasPorFrame;
	char * dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(segmentspagedPerDTBTable, dtbKey))
		return ITS_A_TRAP;
	t_segments_paged* segments = dictionary_get(segmentspagedPerDTBTable,
			dtbKey);
	free(dtbKey);
	char* segmentKey = string_itoa(segmentNumber);
	if (!dictionary_has_key(segments->segments, segmentKey)) {
		free(segmentKey);
		return ITS_A_TRAP;
	}
	t_segment_paged* segment = dictionary_get(segments->segments, segmentKey);
	free(segmentKey);
	if (pageNumber >= segment->limit)
		return ITS_A_TRAP;

	char* pageKey = string_itoa(pageNumber);
	if (!dictionary_has_key(segment->pages, pageKey)) {
		free(pageKey);
		return ITS_A_TRAP;
	}
	int* frame = dictionary_get(segment->pages, pageKey);
	free(pageKey);
	int numLinea = (*frame) * cantLineasPorFrame + frameOffset;
	Logger_Log(LOG_INFO, "FM9 -> Dirección lógica = %d.", logicalAddress);
	Logger_Log(LOG_INFO, "FM9 -> Dirección física = %d.", numLinea);
	return numLinea;

}

int writeData_SPA(void* data, int size, int dtbID) {
	char* dtbKey = string_itoa(dtbID);
	t_segments_paged* segments;
	if (!dictionary_has_key(segmentspagedPerDTBTable, dtbKey)) {
		segments = malloc(sizeof(t_segments_paged));
		segments->segments = dictionary_create();
		segments->nextSegmentNumber = 0;
		segments->nextPageNumber = 0;
		dictionary_put(segmentspagedPerDTBTable, dtbKey, segments);
	} else
		segments = dictionary_get(segmentspagedPerDTBTable, dtbKey);
	free(dtbKey);
	t_segment_paged* segment = malloc(sizeof(t_segment_paged));
	segment->limit = 0;
	segment->pages = dictionary_create();
	t_list* freeFrames = getFreeFrames(size);
	if (freeFrames == NULL)
		return INSUFFICIENT_SPACE;
	int offset = 0, firstPage = segments->nextPageNumber;
	char* pageKey;
	while (!list_is_empty(freeFrames)) {
		int* frameNumber = list_get(freeFrames, 0);
		if (writeFrame(data + offset, *frameNumber) == INVALID_LINE_NUMBER)
			return INVALID_LINE_NUMBER;
		pageKey = string_itoa(segments->nextPageNumber++);
		dictionary_put(segment->pages, pageKey, frameNumber);
		offset += tamanioFrame;
		segment->limit++;
		free(pageKey);
	}
	int segmentNumber = segments->nextSegmentNumber++;
	char* segmentKey = string_itoa(segmentNumber);
	dictionary_put(segments->segments, segmentKey, segment);
	free(segmentKey);
	return segmentNumber * round(pow(10, offsetNumberOfDigits))
			+ firstPage * cantLineasPorFrame;

	//Ir guardando cada pagina en un frame e ir poniendo la key es la tabla de paginas de ese segmento, ir incrementando la cantidad de paginas que tiene ese segmento
	//actualizar el valor del proximo segmento

}
/*
 * Buscar si existe una tabla de segmentos para ese proceso
 * Ver si existe ese segmento
 *
 */

int readData_SPA(void* target, int logicalAddress, int dtbID) {
	int segmentNumber = getSegmentFromAddress(logicalAddress);
	char* dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(segmentspagedPerDTBTable, dtbKey))
		return ITS_A_TRAP;
	t_segments_paged* segments = dictionary_get(segmentspagedPerDTBTable,
			dtbKey);
	free(dtbKey);
	char* segmentKey = string_itoa(segmentNumber);
	if (!dictionary_has_key(segments->segments, segmentKey)) {
		free(segmentKey);
		return ITS_A_TRAP;
	}
	t_segment_paged* segment = dictionary_get(segments->segments, segmentKey);
	free(segmentKey);
	int sizeRead = 0;
	target = malloc(1);
	void pageIterator(char* pageKey, void* frameValue) {
		int frameNumber = *((int*) frameValue);
		target = realloc(target, sizeRead + tamanioFrame);
		if (readFrame(target + sizeRead, frameNumber) == INVALID_LINE_NUMBER)
			return;
		sizeRead += tamanioFrame;
	}
	dictionary_iterator(segment->pages, pageIterator);
	return sizeRead;
}

int closeFile_SPA(int dtbID, int logicalAddress) {
	char* dtbKey = string_itoa(dtbID);
	int segmentNumber = getSegmentFromAddress(logicalAddress);
	if (!dictionary_has_key(segmentspagedPerDTBTable, dtbKey))
		return ITS_A_TRAP;
	t_segments_paged* segments = dictionary_get(segmentspagedPerDTBTable,
			dtbKey);
	free(dtbKey);
	char* segmentKey = string_itoa(segmentNumber);
	if (!dictionary_has_key(segments->segments, segmentKey)) {
		free(segmentKey);
		return ITS_A_TRAP;
	}
	t_segment_paged* segment = dictionary_remove(segments->segments,
			segmentKey);
	free(segmentKey);
	void pageIterator(void* frameValue) {
		int* frameNumber = frameValue;
		addFreeFrame(*frameNumber);
		free(frameNumber);
	}
	dictionary_clean_and_destroy_elements(segment->pages, pageIterator);
	free(segment);
	return 1;
}

int dump_SPA(int dtbID){
	char* dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(segmentspagedPerDTBTable, dtbKey))
		return -1;
	t_segments_paged* segments = dictionary_get(segmentspagedPerDTBTable, dtbKey);
	free(dtbKey);
	Logger_Log(LOG_INFO, "G.DT %d", dtbID);
	Logger_Log(LOG_INFO, "Número próximo segmento %d", segments->nextSegmentNumber);
	Logger_Log(LOG_INFO, "Número próxima página %d", segments->nextPageNumber);

	void pageDumper(char* key, void* data){
		int frameNumber = *((int*)data);
		Logger_Log(LOG_INFO, "Página número %s está en frame %d", key, frameNumber);
		void* buffer = malloc(tamanioFrame);
		Logger_Log(LOG_INFO, "Contenido Página %s", key);
		readFrame(buffer,frameNumber);
		free(buffer);
	}

	void segmentDumper(char* key, void * data) {
		t_segment_paged* segment = data;
		Logger_Log(LOG_INFO, "Segmento %s : Cantidad de páginas = %d", key, segment->limit);
		Logger_Log(LOG_INFO, "Contenido Segmento %s", key);
		dictionary_iterator(segment->pages,pageDumper);

	}
	dictionary_iterator(segments->segments, segmentDumper);
	return 1;
}
