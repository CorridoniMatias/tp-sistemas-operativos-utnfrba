#ifndef PAGED_SEGMENTATION_H_
#define PAGED_SEGMENTATION_H_

#include <stdio.h>
#include <commons/collections/dictionary.h>
#include "Paging.h"
#include "FM9Errors.h"
#include "Segmentation.h"


//La base y limit es el número de linea
typedef struct{
	t_dictionary* pages;
	int limit;
}t_segment_paged;

typedef struct{
	t_dictionary * segments;
	int nextSegmentNumber;
	int nextPageNumber;
}t_segments_paged;


/*
 * Es un diccionario cuyo key es el id del dtb y el value es un t_segments que tiene otro dictionary cuyas keys son
 * los números de segmento y el value es un t_segment_paged que tiene la cantidad de páginas y
 * un dictionary con las paginas como key  y los values son el frame donde esta.
 */
t_dictionary* segmentspagedPerDTBTable;

void createPagedSegmentationStructures();

void freePagedSegmentationStructures();

int addressTranslation_SPA(int logicalAddress, int dtbID);

int writeData_SPA(void* data, int size, int dtbID);

int readData_SPA(void* target, int logicalAddress, int dtbID);

int closeFile_SPA(int dtbID, int logicalAddress) ;

int dump_SPA(int dtbID);

#endif /* PAGED_SEGMENTATION_H_ */
