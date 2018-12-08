#ifndef SEGMENTATION_H_
#define SEGMENTATION_H_

#include "Storage.h"
#include <commons/collections/list.h>
#include <commons/collections/dictionary.h>
#include <stdbool.h>
#include <math.h>
#include "kemmens/Utils.h"

//La base y limit es el número de linea
typedef struct{
	int base;
	int limit;
} t_segment;

//Cantidad de digitos usados para el offset
int offsetNumberOfDigits;
//Es una lista de segments.
t_list* freeSegments;

typedef struct{
	t_dictionary * segments;
	int nextSegmentNumber;
}t_segments;

//Esto es un diccionario que cada key es el id del DTB y el value es un t_segments,
//cuyas keys del diccionario son los numeros de segmentos y el value seria un segment.
t_dictionary* segmentsPerDTBTable;


//Requerimiento: Inicializa las estructuras de la paginación, que son el tamanio del frame, la cantidad de frames y los frames libres.
/*
 * 		Descripción: Inicializa la variable freeFrames con una lista con todos los frames indicados por la cantidad
 * 		de frames, que se obtiene en base al tamanio de la memoria y el tamanio del frame, ya que cuando inicia el FM9
 * 		no hay ningun frame ocupado.
 */
void createSegmentationStructures();

//Requerimiento: Liberar las estructuras de segmentación.
/*
 * 	Descripción: Libera la memoria ocupada por la lista de segmentos libres y el diccionario de los segmentos por proceso.
 */
void freeSegmentationStructures();

//Requerimiento: Guardar datos en memoria creando un segmento
/*
 * 	Parámetros: [Data, Size]
 *
 *	Descripción: Ante un pedido de guardado en storage la segmentación verifica que haya segmentos libres del espacio
 *	necesario, en caso afirmativo, crea los segmentos y guarda los datos en storage.
 *
 *	Retorna: si se pudo escribir devuelve la dirección lógica del segmento, caso contrario INSUFFICIENT_SPACE.
 */

int addressTranslation_SEG(int virtualAddress, int dtbID);

int writeData_SEG(void* data, int size, int dtbID);

int readData_SEG(void** target, int virtualAddress, int dtbID);

int closeFile_SEG(int dtbID, int virtualAddress);

int dump_SEG(int dtbID);

int getSegmentFromAddress(int virtualAddress);

int getOffsetFromAddress(int virtualAddress);

int getNewSegmentNumber();

void addFreeSegment(t_segment* segment);

void sortFreeSegments();

void freeSegmentCompaction();

int sizeOfSegment(int dtbID, int virtualAddress);


#endif /* SEGMENTATION_H_ */
