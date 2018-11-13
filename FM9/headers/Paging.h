#ifndef FREE_FRAMES_LIST_H_
#define FREE_FRAMES_LIST_H_

#include <stdio.h>
#include <commons/collections/list.h>
#include "kemmens/logger.h"
#include "bibliotecaFM9.h"
#include "Storage.h"

//FM9 General Errors:

#define NO_FRAMES_AVAILABLE	-1
#define INVALID_FRAME_NUMBER -2
#define INSUFFICIENT_FRAMES_AVAILABLE -3

int tamanioFrame;
int cantLineasPorFrame;
int cantFrames;
t_list* framesLibres;


//Requerimiento: Inicializa las estructuras de la paginación, que son el tamanio del frame, la cantidad de frames y los frames libres.
/*
 * 		Descripción: Inicializa la variable freeFrames con una lista con todos los frames indicados por la cantidad
 * 		de frames, que se obtiene en base al tamanio de la memoria y el tamanio del frame, ya que cuando inicia el FM9
 * 		no hay ningun frame ocupado.
 */
void createPagingStructures();

//Requerimiento: Liberar lista de frames libres.
/*
 *
 * 		Descripción: Libera la memoria ocupada por la lista de frames.
 */
void freePagingStructures();

//Requerimiento: Agregar el número de un frame libre a la lista de frames.
/*
 * 	Parámetros: [NumFrame]
 *
 *	Descripción: Agrega a la lista de frames libres el número de un frame para que después pueda ser ocupado nuevamente.
 */
void addFreeFrame(int numFrame);

//Requerimiento: Obtener un frame libre.
/*
 *
 *	Descripción: Devuelve el primer frame libre de la lista, eliminandolo de la misma
 *
 *	Retorna: El número de frame libre, en caso de no haber ninguno devuelve NO_FRAMES_AVAILABLE.
 */
int getFreeFrame();

//Requerimiento: Escribir una página en un frame.
/*
 * 	Parámetros: [Page, NumFrame]
 *
 *	Descripción: Guarda los bytes de page en el frame numFrame.
 *
 * 	Retorna: si se pudo escribir 1, caso contrario INVALID_FRAME_NUMBER o INVALID_LINE_NUMBER.
 */
int writeFrame(void* page, int numFrame);

//Requerimiento: Leer datos de un Frame
/*
 * 	Parámetros: [Target, NumFrame]
 *
 *	Descripción: Ante un pedido de lectura de un frame, copia en target el contenido de ese número de frame.
 *
 *	Retorna: si se pudo leer 1, caso contrario INVALID_FRAME_NUMBER o INVALID_LINE_NUMBER.
 */
int readFrame(void* target, int numFrame);

//Requerimiento: Saber cuantos frames se necesitarían para guardar un tamaño size y si hay esa cantidad libres.
/*
 * 	Parámetros: [Size]
 *
 *	Descripción: Calcula en base al tamaño de los frames cuantos se necesitarían para guardar información que
 *	ocupe size y se fija en la lista de frames libres si hay esa cantidad.
 *
 *	Retorna: La cantidad de frames necesarios, en caso de no haya esa cantidad de frames libres devuelve
 *  INSUFFICIENT_FRAMES_AVAILABLE.
 */
int framesNeededAreAvailable(int size);

#endif /* #define FREE_FRAMES_LIST_H_ */
