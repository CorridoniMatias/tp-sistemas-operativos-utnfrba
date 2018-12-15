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
	pthread_mutex_init(&freeSegmentsLock, NULL);
	pthread_mutex_init(&segmentsLock, NULL);
	Logger_Log(LOG_INFO, "FM9 -> Estructuras de segmentación creadas.");
	Logger_Log(LOG_INFO, "FM9 -> Cantidad de dígitos de la dirección lógica empleados para el offset de segmento %d.", offsetNumberOfDigits);
}

void freeSegmentationStructures() {
	pthread_mutex_lock(&freeSegmentsLock);
	list_destroy_and_destroy_elements(freeSegments, free);
	pthread_mutex_unlock(&freeSegmentsLock);
	void dictionaryDestroyer(void* data) {
		t_segments* segments = data;
		dictionary_destroy_and_destroy_elements(segments->segments, free);
		free(segments);
	}
	pthread_mutex_lock(&segmentsLock);
	dictionary_destroy_and_destroy_elements(segmentsPerDTBTable, dictionaryDestroyer);
	pthread_mutex_unlock(&segmentsLock);
	pthread_mutex_destroy(&freeSegmentsLock);
	pthread_mutex_destroy(&segmentsLock);
	Logger_Log(LOG_INFO, "FM9 -> Estructuras de segmentación liberadas.");
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

	pthread_mutex_lock(&freeSegmentsLock);
	t_segment* freeSegment = list_remove_by_condition(freeSegments, enoughSpaceSegment);
	pthread_mutex_unlock(&freeSegmentsLock);

	if(freeSegment==NULL){
		return INSUFFICIENT_SPACE;
	}
	for (int i = 0; i < lineasNecesarias; i++) {
		if (writeLine(((char*) data) + i * tamanioLinea, freeSegment->base + i) == INVALID_LINE_NUMBER) {
//			printf("\n\n\nnumero de linea %d\n\n\n",freeSegment->base + i);
			return ITS_A_TRAP;
		}
	}

	t_segment* newSegment = malloc(sizeof(t_segment));
	newSegment->base = freeSegment->base;
	newSegment->limit = lineasNecesarias;

	char* key = string_itoa(dtbID);
	t_segments* segments;
	pthread_mutex_lock(&segmentsLock);
	if (dictionary_has_key(segmentsPerDTBTable, key)) {
		segments = dictionary_get(segmentsPerDTBTable, key);
	} else {
		segments = malloc(sizeof(t_segments));
		segments->segments = dictionary_create();
		segments->nextSegmentNumber = 0;
		dictionary_put(segmentsPerDTBTable, key, segments);
	}
	pthread_mutex_unlock(&segmentsLock);
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
		freeSegment->limit -= lineasNecesarias;
		addFreeSegment(freeSegment);
	}
	//Ver como hacer para que el segment number sea parte de la direccion lógica
//	printf("\n\n\n\nnumero de segmento %d\n\n\n",segmentNumber);
	int virtualAddress = segmentNumber * powi(10, offsetNumberOfDigits);
//	Logger_Log(LOG_INFO, "FM9 -> Dirección lógica = %d.", virtualAddress);
	return virtualAddress;
}

int readData_SEG(void** target, int logicalAddress, int dtbID) {
	int baseLine = addressTranslation_SEG(logicalAddress, dtbID);
	if (baseLine == ITS_A_TRAP)
		return ITS_A_TRAP;
	char * dtbKey = string_itoa(dtbID);
	pthread_mutex_lock(&segmentsLock);
	if (!dictionary_has_key(segmentsPerDTBTable, dtbKey)){
		pthread_mutex_unlock(&segmentsLock);
		free(dtbKey);
		return ITS_A_TRAP;
	}
	t_segments* segments = dictionary_get(segmentsPerDTBTable, dtbKey);
	pthread_mutex_unlock(&segmentsLock);
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
//			printf("\n\n\nnumero invalido de linea\n\n\n");
			return ITS_A_TRAP;
		}
		sizeRead += tamanioLinea;
		linesRead++;
	}
	return sizeRead;
}

int addressTranslation_SEG(int logicalAddress, int dtbID) {
	char * dtbKey = string_itoa(dtbID);
	pthread_mutex_lock(&segmentsLock);
	if (!dictionary_has_key(segmentsPerDTBTable, dtbKey)){
		pthread_mutex_unlock(&segmentsLock);
		free(dtbKey);
		return ITS_A_TRAP;
	}
	t_segments* segments = dictionary_get(segmentsPerDTBTable, dtbKey);
	pthread_mutex_unlock(&segmentsLock);
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

//	Logger_Log(LOG_INFO, "FM9 -> Dirección lógica = %d.", logicalAddress);
	int numLinea = segment->base + getOffsetFromAddress(logicalAddress);
//	Logger_Log(LOG_INFO, "FM9 -> Dirección física = %d.", numLinea);

	return numLinea;
}

int dump_SEG(int dtbID) {
	char* dtbKey = string_itoa(dtbID);
	pthread_mutex_lock(&segmentsLock);
	if (!dictionary_has_key(segmentsPerDTBTable, dtbKey)) {
		pthread_mutex_unlock(&segmentsLock);
		free(dtbKey);
		return ITS_A_TRAP;
	}
	t_segments* segments = dictionary_get(segmentsPerDTBTable, dtbKey);
	pthread_mutex_unlock(&segmentsLock);
	free(dtbKey);
//	Logger_Log(LOG_INFO, "G.DT %d", dtbID);
	Logger_Log(LOG_INFO, "Número próximo segmento %d", segments->nextSegmentNumber);
	loggLines = false;
	void segmentDumper(char* key, void * data) {
		t_segment* segment = data;
		Logger_Log(LOG_INFO, "Segmento %s - Base = %d - Límite = %d", key, segment->base, segment->limit);
		char* buffer = calloc(1,tamanioLinea + 1);
//		int size = 0;
		for (int i = 0; i < segment->limit; i++) {
			readLine(buffer, segment->base + i);
//			size += tamanioLinea;
			Logger_Log(LOG_INFO, "Contenido linea %d\n%s", segment->base + i, buffer);
		}
//		buffer = realloc(buffer, size + 1);
//		buffer[size] = 0;
		free(buffer);
	}
	dictionary_iterator(segments->segments, segmentDumper);
	loggLines = true;
	return 1;

}

int closeFile_SEG(int dtbID, int virtualAddress) {
	char* dtbKey = string_itoa(dtbID);
	pthread_mutex_lock(&segmentsLock);
	if (!dictionary_has_key(segmentsPerDTBTable, dtbKey)){
		pthread_mutex_unlock(&segmentsLock);
		free(dtbKey);
		return ITS_A_TRAP;}
	pthread_mutex_unlock(&segmentsLock);
	t_segments* segments = dictionary_get(segmentsPerDTBTable, dtbKey);
	free(dtbKey);
	int segmentNumber = getSegmentFromAddress(virtualAddress);
//	printf("\n\n\n\nnumero de segmento %d\n\n\n",segmentNumber);
	char* segmentKey = string_itoa(segmentNumber);
	if (!dictionary_has_key(segments->segments, segmentKey)) {
		free(segmentKey);
		return ITS_A_TRAP;
	}
	t_segment* segment = dictionary_remove(segments->segments, segmentKey);
	free(segmentKey);
	addFreeSegment(segment);
//	free(segment);
	return 1;
}

int closeDTBFiles_SEG(int dtbID) {
	char* dtbKey = string_itoa(dtbID);
	pthread_mutex_lock(&segmentsLock);
	if (!dictionary_has_key(segmentsPerDTBTable, dtbKey)) {
		pthread_mutex_unlock(&segmentsLock);
		free(dtbKey);
		return ITS_A_TRAP;
	}
	pthread_mutex_unlock(&segmentsLock);
	t_segments* segments = dictionary_remove(segmentsPerDTBTable, dtbKey);
	void dictionaryDestroyer(void* data){
		t_segment* segment = data;
		addFreeSegment(segment);
	}
	free(dtbKey);
	dictionary_destroy_and_destroy_elements(segments->segments,dictionaryDestroyer);
	free(segments);
	return 1;
}

void addFreeSegment(t_segment* segment) {
	pthread_mutex_lock(&freeSegmentsLock);
	list_add(freeSegments, segment);
	sortFreeSegments();
	freeSegmentCompaction();
	pthread_mutex_unlock(&freeSegmentsLock);
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
	while (index + 1 < list_size(freeSegments)) {
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

int getSegmentFromAddress(int logicalAddress) {
	return logicalAddress / powi(10, offsetNumberOfDigits);
}

int getOffsetFromAddress(int logicalAddress) {
	return logicalAddress % powi(10, offsetNumberOfDigits);
}

int getNewSegmentNumber(t_segments* segments) {
	return segments->nextSegmentNumber;
}

int powi(int base, int power){
	int aux = base;
	for(int i = 1; i < power; i++){
		aux *= base;
	}
	return aux;
}
