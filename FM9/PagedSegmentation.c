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
	printf("\n\n\nnumero segmento %d\n\n\n",segmentNumber);
	int segmentOffset = getOffsetFromAddress(logicalAddress);
	printf("\n\n\noffset segmento %d\n\n\n",segmentOffset);
	int pageNumber = segmentOffset / cantLineasPorFrame;
	printf("\n\n\nnumero pagina %d\n\n\n",pageNumber);
	int frameOffset = segmentOffset % cantLineasPorFrame;
	printf("\n\n\nframeOffset %d\n\n\n",frameOffset);
	char * dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(segmentspagedPerDTBTable, dtbKey)){

		printf("\n\n\nno existe ese proceso\n\n\n");
		free(dtbKey);
		return ITS_A_TRAP;
	}
	t_segments_paged* segments = dictionary_get(segmentspagedPerDTBTable,
			dtbKey);
	free(dtbKey);
	char* segmentKey = string_itoa(segmentNumber);
	if (!dictionary_has_key(segments->segments, segmentKey)) {

		printf("\n\n\nno existe ese segmento\n\n\n");
		free(segmentKey);
		return ITS_A_TRAP;
	}
	t_segment_paged* segment = dictionary_get(segments->segments, segmentKey);
	free(segmentKey);
	if (pageNumber >= segment->limit)
	{
		printf("\n\n\nsegment fault perra\n\n\n");
		return ITS_A_TRAP;
	}
	char* pageKey = string_itoa(pageNumber);
	if (!dictionary_has_key(segment->pages, pageKey)) {

		printf("\n\n\nno existe esa pagina\n\n\n");
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
	printf("\n\n\ncantidad de frames necesarios %d",list_size(freeFrames));
	int offset = 0, firstPage = segments->nextPageNumber;
	printf("\n\n\nfirstPage %d\n\n\n",firstPage);
	char* pageKey;
	while (!list_is_empty(freeFrames)) {
		int* frameNumber = list_remove(freeFrames, 0);
		//Para solucionar la fragmentacion interna en la ultima pagina
		printf("\n\n\nnumero de frame %d\n\n\n",*frameNumber);
		char* buffer = calloc(1, tamanioFrame);
		if(size - offset < tamanioFrame){
			printf("\n\n\nhay fragmentacion interna\n\n\n");
			memcpy(buffer,data+offset,size-offset);
		}
		else
			memcpy(buffer,data+offset,tamanioFrame);
		if (writeFrame(buffer, *frameNumber) <= 0)
			return ITS_A_TRAP;
		free(buffer);
		pageKey = string_itoa(segments->nextPageNumber++);
		printf("\n\n\nnumero pagina %s\n\n\n",pageKey);
		dictionary_put(segment->pages, pageKey, frameNumber);
		offset += tamanioFrame;
		segment->limit++;
		free(pageKey);
	}
	int segmentNumber = segments->nextSegmentNumber++;
	char* segmentKey = string_itoa(segmentNumber);
	dictionary_put(segments->segments, segmentKey, segment);
	free(segmentKey);
	printf("\n\n\n numero segmento %d",segmentNumber);
	printf("\n\n\n cant digitos %d",offsetNumberOfDigits);
	printf("\n\n\n dir segmento %d",segmentNumber * (int)pow((float)10, (float)offsetNumberOfDigits));
	printf("\n\n\n numero pag %d",firstPage);
	printf("\n\n\n cant linear por fram %d",cantLineasPorFrame);

	return segmentNumber * (int)pow((float)10, (float)offsetNumberOfDigits) + firstPage * cantLineasPorFrame;

	//Ir guardando cada pagina en un frame e ir poniendo la key es la tabla de paginas de ese segmento, ir incrementando la cantidad de paginas que tiene ese segmento
	//actualizar el valor del proximo segmento

}
/*
 * Buscar si existe una tabla de segmentos para ese proceso
 * Ver si existe ese segmento
 *
 */

int readData_SPA(void** target, int logicalAddress, int dtbID) {
	int segmentNumber = getSegmentFromAddress(logicalAddress);
	char* dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(segmentspagedPerDTBTable, dtbKey)){
		free(dtbKey);
		return ITS_A_TRAP;
	}
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
	int result = 0, sizeRead = 0;
	*target = malloc(1);
	void pageIterator(char* pageKey, void* frameValue) {
		if (result == INVALID_LINE_NUMBER || result == INVALID_FRAME_NUMBER) {
			return;
		}
		int frameNumber = *((int*) frameValue);
		*target = realloc(*target, sizeRead + tamanioFrame);
		result = readFrame(*target + sizeRead, frameNumber);
		sizeRead += tamanioFrame;
	}
	dictionary_iterator(segment->pages, pageIterator);
	return !result?sizeRead:ITS_A_TRAP;
}

int closeFile_SPA(int dtbID, int logicalAddress) {
	char* dtbKey = string_itoa(dtbID);
	int segmentNumber = getSegmentFromAddress(logicalAddress);
	if (!dictionary_has_key(segmentspagedPerDTBTable, dtbKey)){
		free(dtbKey);
		return ITS_A_TRAP;
	}
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
	if (!dictionary_has_key(segmentspagedPerDTBTable, dtbKey)) {
		free(dtbKey);
		return -1;
	}
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
