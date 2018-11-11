#ifndef FREE_FRAMES_LIST_H_
#define FREE_FRAMES_LIST_H_

#include <stdio.h>
#include "kemmens/logger.h"
#include "bibliotecaFM9.h"

//FM9 General Errors:

#define NO_FRAMES_AVAILABLE	-1

size_t tamanioFrame;
size_t cantFrames;
t_list* framesLibres;


//Requerimiento: Inicializa las estructuras de la paginación, que son el tamanio del frame, la cantidad de frames y los frames libres.
/*
 * 		Descripción: Inicializa la variable freeFrames con una lista con todos los frames indicados por la cantidad
 * 		de frames, que se obtiene en base al tamanio de la memoria y el tamanio del frame, ya que cuando inicia el FM9
 * 		no hay ningun frame ocupado.
 */
void createPagingStructures();

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

#endif /* #define FREE_FRAMES_LIST_H_ */
