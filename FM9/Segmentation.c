#include "headers/Segmentation.h"

void createSegmentationStructures() {
	freeSegments = list_create();
	t_segment* freeSegment = malloc(sizeof(t_segment));
	freeSegment->base = 0;
	freeSegment->limit = cantLineas;
	list_add(freeSegments, freeSegment);
	segmentsPerDTBTable = dictionary_create();
	offsetNumberOfDigits = 0;

	//Para contar cantidad de digitos para el offset
	int aux = cantLineas;
	while (aux > 0) {
		aux = aux / 10;
		offsetNumberOfDigits++;
	}
}

void freeSegmentationStructures() {
	list_destroy_and_destroy_elements(freeSegments, free);
	void dictionaryDestroyer(void* segments) {
		dictionary_destroy_and_destroy_elements(
				((t_segments*) segments)->segments, free);
		free(segments);
	}
	dictionary_destroy_and_destroy_elements(segmentsPerDTBTable,
			dictionaryDestroyer);
}

//TESTEAR QUE PASA SI LE MANDO UN SEGMENTO QUE TIENE FRAGMENTACION INTERNA POR LA LINEA
int writeData_SEG(void* data, int size, int dtbID) {
	int lineasNecesarias = size / tamanioLinea;
	if (size % tamanioLinea > 0)
		lineasNecesarias++;
//Logica para conseguir un segmento del tamaño necesario
	bool enoughSpaceSegment(void* seg) {
		t_segment* segment = seg;
		return segment->limit >= lineasNecesarias;
	}

	t_segment* freeSegment = list_remove_by_condition(freeSegments,
			enoughSpaceSegment);

	if(freeSegment==NULL){
		return INSUFFICIENT_SPACE;
	}
	for (int i = 0; i < lineasNecesarias; i++) {
		if (writeLine(((char*) data) + i * tamanioLinea, freeSegment->base + i) == INVALID_LINE_NUMBER) {
			return ITS_A_TRAP;
		}
	}

	t_segment* newSegment = malloc(sizeof(t_segment));
	newSegment->base = freeSegment->base;
	newSegment->limit = lineasNecesarias;

	char* key = string_itoa(dtbID);
	t_segments* segments;
	if (dictionary_has_key(segmentsPerDTBTable, key)) {
		segments = dictionary_get(segmentsPerDTBTable, key);
	} else {
		segments = malloc(sizeof(t_segments));
		segments->segments = dictionary_create();
		segments->nextSegmentNumber = 0;
		dictionary_put(segmentsPerDTBTable, key, segments);
	}
	free(key);
	int segmentNumber = getNewSegmentNumber(segments);
	//IMPLEMENTAR ALGUNA LOGICA PARA PONER NUMERO DE SEGMENTO QUE NO SE REPITA
	char* segmentKey = string_itoa(segmentNumber);
	dictionary_putMAESTRO(segments->segments,segmentKey, newSegment, free);
	free(segmentKey);
	segments->nextSegmentNumber++;

	if (freeSegment->limit == lineasNecesarias) {
		free(freeSegment);
	} else {
		freeSegment->base += lineasNecesarias;
		freeSegment->limit += lineasNecesarias;
		list_add(freeSegments, freeSegment);
	}
	//Ver como hacer para que el segment number sea parte de la direccion lógica
	int virtualAddress = segmentNumber * round(pow(10, offsetNumberOfDigits));
	Logger_Log(LOG_INFO, "FM9 -> Dirección lógica = %d.", virtualAddress);
	return virtualAddress;
}

int readData_SEG(void** target, int logicalAddress, int dtbID) {
	int baseLine = addressTranslation_SEG(logicalAddress, dtbID);
	if (baseLine == ITS_A_TRAP)
		return ITS_A_TRAP;
	char * dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(segmentsPerDTBTable, dtbKey)){
		free(dtbKey);
		return ITS_A_TRAP;
	}
	t_segments* segments = dictionary_get(segmentsPerDTBTable, dtbKey);
	free(dtbKey);
	int segmentNumber = getSegmentFromAddress(logicalAddress);
	char* segmentKey = string_itoa(segmentNumber);
	if (!dictionary_has_key(segments->segments, segmentKey)) {
		free(segmentKey);
		return ITS_A_TRAP;
	}
	t_segment* segment = dictionary_get(segments->segments, segmentKey);
	free(segmentKey);
	int sizeRead=0;
	int linesRead=0;
	*target=malloc(1);
	while(linesRead<segment->limit)
	{
		*target = realloc(*target, sizeRead + tamanioLinea);
		memset(*target+sizeRead,0,tamanioLinea);
		if (readLine(*target + sizeRead, baseLine+linesRead) == INVALID_LINE_NUMBER){
			printf("\n\n\nnumero invalido de linea\n\n\n");
			break;
		}
		sizeRead += tamanioLinea;
		linesRead++;
	}
	return sizeRead;
}

int addressTranslation_SEG(int logicalAddress, int dtbID) {
	char * dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(segmentsPerDTBTable, dtbKey)){
		free(dtbKey);
		return ITS_A_TRAP;
	}
	t_segments* segments = dictionary_get(segmentsPerDTBTable, dtbKey);
	free(dtbKey);
	int segmentNumber = getSegmentFromAddress(logicalAddress);
	char* segmentKey = string_itoa(segmentNumber);
	if (!dictionary_has_key(segments->segments, segmentKey)) {
		free(segmentKey);
		return ITS_A_TRAP;
	}
	t_segment* segment = dictionary_get(segments->segments, segmentKey);
	free(segmentKey);
	if (getOffsetFromAddress(logicalAddress) >= segment->limit)
		return ITS_A_TRAP;

	Logger_Log(LOG_INFO, "FM9 -> Dirección lógica = %d.", logicalAddress);
	int numLinea = segment->base + getOffsetFromAddress(logicalAddress);
	Logger_Log(LOG_INFO, "FM9 -> Dirección física = %d.", numLinea);

	return numLinea;
}

int getSegmentFromAddress(int logicalAddress) {
	return logicalAddress / round(pow(10, offsetNumberOfDigits));
}

int getOffsetFromAddress(int logicalAddress) {
	return logicalAddress % ((int) round(pow(10, offsetNumberOfDigits)));
}

int getNewSegmentNumber(t_segments* segments) {
	return segments->nextSegmentNumber;
}

int dump_SEG(int dtbID) {
	char* dtbKey = string_itoa(dtbID);
	if (!dictionary_has_key(segmentsPerDTBTable, dtbKey))
		return -1;

	t_segments* segments = dictionary_get(segmentsPerDTBTable, dtbKey);
	free(dtbKey);
	Logger_Log(LOG_INFO, "G.DT %d", dtbID);
	void segmentDumper(char* key, void * data) {
		t_segment* segment = data;
		Logger_Log(LOG_INFO, "Segmento %s : Base = %d : Límite = %d", key,
				segment->base, segment->limit);
		char* buffer = malloc(tamanioLinea);
		Logger_Log(LOG_INFO, "Contenido Segmento %s", key);
		for (int i = 0; i < segment->limit; i++) {
			readLine(buffer, segment->base + i);
		}
		free(buffer);
	}
	dictionary_iterator(segments->segments, segmentDumper);
	return 1;

}

int closeFile_SEG(int dtbID, int virtualAddress) {
	char* dtbKey = string_itoa(dtbID);
	if (dictionary_has_key(segmentsPerDTBTable, dtbKey))
		return -1;

	t_segments* segments = dictionary_get(segmentsPerDTBTable, dtbKey);

	int segmentNumber = getSegmentFromAddress(virtualAddress);
	char* segmentKey = string_itoa(segmentNumber);
	if (!dictionary_has_key(segments->segments, segmentKey)) {
		return -1;
	}
	t_segment* segment = dictionary_remove(segments->segments, segmentKey);
	addFreeSegment(segment);
	free(segment);
	return 1;
}

void addFreeSegment(t_segment* segment) {
	list_add(freeSegments, freeSegments);
	sortFreeSegments();
	freeSegmentCompaction();
}

void sortFreeSegments() {
	bool comparator(void* firstElement, void* secondElement) {
		t_segment* firstSegment = firstElement;
		t_segment* secondSegment = secondElement;
		return firstSegment->base < secondSegment->base;
	}
	list_sort(freeSegments, comparator);
}

void freeSegmentCompaction() {
	t_segment* firstSegment;
	t_segment* secondSegment;
	int index = 0;
	while (index < list_size(freeSegments)) {
		firstSegment = list_get(freeSegments, index);
		secondSegment = list_get(freeSegments, index + 1);
		if (firstSegment->base + firstSegment->limit == secondSegment->base) {
			firstSegment->limit += secondSegment->limit;
			list_remove_and_destroy_element(freeSegments, index + 1, free);
		}
		else
			index++;
	}
}
