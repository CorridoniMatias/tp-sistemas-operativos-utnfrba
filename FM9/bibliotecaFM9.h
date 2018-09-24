#ifndef BIBLIOTECAFM9_H_
#define BIBLIOTECAFM9_H_

#include "kemmens/config.h"
#include "kemmens/SocketClient.h"

#define RUTA_CONFIG "FM9.config"


#define TAMMAXPUERTO 6

struct Configuracion_s
{
	int puerto;
	//char puertoEscucha[TAMMAXPUERTO];
	char modo[3];
	int tamanio;
	int max_linea;
	int tam_pagina;

} typedef Configuracion;


Configuracion* settings;


void configurar();

#endif /* BIBLIOTECAFM9_H_ */
