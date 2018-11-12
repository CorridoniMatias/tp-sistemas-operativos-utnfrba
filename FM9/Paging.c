#include "headers/Paging.h"

void createPagingStructures() {

	Logger_Log(LOG_INFO, "FM9 -> Creando estructuras de paginado.");

	tamanioFrame = settings->tam_pagina;
	cantFrames = settings->tamanio / tamanioFrame;
	framesLibres = list_create();
	int* frame;
	for (int i = 0; i < cantFrames; i++) {
		frame = malloc(sizeof(int));
		*frame = i;
		list_add(framesLibres, frame);
//		Logger_Log(LOG_DEBUG, "FM9 -> Frame %d guardado en lista de frames.", *frame);
	}

//	Logger_Log(LOG_DEBUG, "FM9 -> Se guardaron %d frames libres.", framesLibres->elements_count);

	Logger_Log(LOG_INFO, "FM9 -> Se crearon estructuras de paginado.");
}

void freePagingStructures(){

	void liberarFrame (void* frame){
		static int i = 0;
		free(frame);
		Logger_Log(LOG_DEBUG, "FM9 -> Se libero el frame %d de la lista.", i);
		i++;
	};

	list_destroy_and_destroy_elements(framesLibres,liberarFrame);

	Logger_Log(LOG_INFO, "FM9 -> Lista de frames libres liberada.");
}

void addFreeFrame(int numFrame)
{
	int* frame = malloc(sizeof(int));
	*frame = numFrame;
	list_add(framesLibres, frame);

	Logger_Log(LOG_INFO, "FM9 -> Se agrego el frame %d a la lista de frames libres.", *frame);
}

int getFreeFrame(){
	if(list_is_empty(framesLibres))
		return NO_FRAMES_AVAILABLE;
	int* frame = list_remove(framesLibres, 0);

//	Logger_Log(LOG_DEBUG, "FM9 -> El primer frame libre es el %d.", *frame);

	int numFrame = *frame;
	free(frame);

	Logger_Log(LOG_INFO, "FM9 -> Se devolvio el frame %d y se saco de la lista de frames libres.", numFrame);

	return numFrame;
}
