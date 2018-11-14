#include "headers/Segmentation.h"

void createSegmentationStructures() {
	freeSegments = list_create();
	segment* freeSegment = malloc(sizeof(segment));
	freeSegment->base = 0;
	freeSegment->limit = cantLineas;
	list_add(freeSegments, freeSegment);
	segmentsPerDTBTable = dictionary_create();
}

void freeSegmentationStructures() {
	list_destroy_and_destroy_elements(freeSegments, free);
	void dictionaryDestroyer(void* segments) {
		dictionary_destroy_and_destroy_elements(segments, free);
	}
	dictionary_destroy_and_destroy_elements(segmentsPerDTBTable, dictionaryDestroyer);
}
