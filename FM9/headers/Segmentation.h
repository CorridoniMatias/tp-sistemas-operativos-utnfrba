#ifndef SEGMENTATION_H_
#define SEGMENTATION_H_

#include "Storage.h"
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>

//La base y limit es el número de linea
typedef struct{
	int base;
	int limit;
} segment;

//Es una lista de segments.
t_list* freeSegments;
//Esto es un diccionario que cada key es el id del DTB y el value es otro diccionario,
//cuyas keys son los numeros de segmentos y el value seria un segment.
t_dictionary* segmentsPerDTBTable;

void createSegmentationStructures();

#endif /* #define SEGMENTATION_H_ */
