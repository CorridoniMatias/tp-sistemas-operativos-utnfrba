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
		dictionary_destroy_and_destroy_elements(((t_segments*) segments)->segments, free);
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

	for (int i = 0; i < lineasNecesarias; i++) {
		if (writeLine(((char*) data) + i * tamanioLinea,
				freeSegment->base + i) == INVALID_LINE_NUMBER) {
			return INVALID_LINE_NUMBER;
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
	if (dictionary_has_key(segments->segments, segmentKey)){
		Logger_Log(LOG_DEBUG, "FM9 -> se encontro la key repetida shittttttttttttttt.");
		free(dictionary_get(segments->segments,segmentKey));
	}
	dictionary_put(segments->segments, segmentKey, newSegment);
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

int readDataSegmentation(void* target, int virtualAddress, int dtbID) {
	int lineNumber = segmentationAddressTranslation(virtualAddress, dtbID);
	if (lineNumber == -1)
		return -1;
	return readLine(target, lineNumber);
}

int segmentationAddressTranslation(int virtualAddress, int dtbID) {
	char * key = string_itoa(dtbID);
	if (!dictionary_has_key(segmentsPerDTBTable, key))
		return -1;
	t_segments* segments = dictionary_get(segmentsPerDTBTable, key);
	free(key);
	int segmentNumber = getSegmentFromAddress(virtualAddress);
	char* segmentKey = string_itoa(segmentNumber);
	if (!dictionary_has_key(segments->segments, segmentKey)) {
		free(segmentKey);
		return -1;
	}
	t_segment* segment = dictionary_get(segments->segments, segmentKey);
	free(segmentKey);
	if (getOffsetFromAddress(virtualAddress) > segment->limit)
		return -1;

	Logger_Log(LOG_INFO, "FM9 -> Dirección lógica = %d.", virtualAddress);
	int numLinea = segment->base + getOffsetFromAddress(virtualAddress);
	Logger_Log(LOG_INFO, "FM9 -> Dirección física = %d.", numLinea);

	return numLinea;
}

int getSegmentFromAddress(int virtualAddress) {
	return virtualAddress / round(pow(10, offsetNumberOfDigits));
}

int getOffsetFromAddress(int virtualAddress) {
	return virtualAddress % ((int) round(pow(10, offsetNumberOfDigits)));
}

int getNewSegmentNumber(t_segments* segments) {
	return segments->nextSegmentNumber;
}
