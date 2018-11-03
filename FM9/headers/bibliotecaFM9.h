#ifndef BIBLIOTECAFM9_H_
#define BIBLIOTECAFM9_H_

#include "kemmens/config.h"
#include "kemmens/SocketClient.h"
#include "commons/string.h"

#define RUTA_CONFIG "FM9.config"

struct Configuracion_s
{
	int puerto;
	char modo[3];
	int tamanio;
	int max_linea;
	int tam_pagina;

} typedef Configuracion;


Configuracion* settings;


void configurar();

#endif /* BIBLIOTECAFM9_H_ */
