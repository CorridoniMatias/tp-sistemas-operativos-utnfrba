#ifndef BIBLIOTECACPU_H_
#define BIBLIOTECACPU_H_

#include "kemmens/config.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "kemmens/SocketClient.h"

#define RUTA_CONFIG "CPU.config"


#define TAMMAXPUERTO 6
#define TAMMAXIP 20

struct Configuracion_s
{
	char ipSAFA[TAMMAXIP];
	char puertoSAFA[TAMMAXPUERTO];
	char ipDIEGO[TAMMAXIP];
	char puertoDIEGO[TAMMAXPUERTO];
	char ipFM9[TAMMAXIP];
	char puertoFM9[TAMMAXPUERTO];
	int retardo;

} typedef Configuracion;


Configuracion* settings;


void configurar();
int conectarAProceso(char* ip, char* puerto, char* nombreProceso); // Funcion creada por PEPE


#endif /* BIBLIOTECACPU_H_ */
