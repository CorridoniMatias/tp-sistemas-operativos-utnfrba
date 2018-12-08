#ifndef BIBLIOTECAFM9_H_
#define BIBLIOTECAFM9_H_

#include "kemmens/config.h"
#include "kemmens/SocketClient.h"
#include "commons/string.h"
#include "Segmentation.h"
#include "Paging.h"
#include "InvertedPageTable.h"
#include "PagedSegmentation.h"

#define RUTA_CONFIG "FM9.config"

//Modos de memoria
#define SEG 1
#define TPI 2
#define SPA 3

struct Configuracion_s
{
	int puerto;
	int modo;
	int tamanio;
	int max_linea;
	int tam_pagina;

} typedef Configuracion;


Configuracion* settings;

typedef struct{
	void (*createStructures)();
	void (*freeStructures)();
	int (*virtualAddressTranslation)(int logicalAddress, int dtdID);
	int (*writeData)(void* data, int size, int dtdID);
	int (*readData)(void** target, int logicalAddress, int dtdID);
	int (*closeFile)(int dtdID,int logicalAddress);
	int (*dump)(int dtdID);
//	int (*size)(int,int);
	//No es necesaria esta creo, con hacer la traduccion esta bien;
//	int (*readLine)(void*,int,int);
} t_memoryFunctions;

t_memoryFunctions* memoryFunctions;
void configurar();

#endif /* BIBLIOTECAFM9_H_ */
