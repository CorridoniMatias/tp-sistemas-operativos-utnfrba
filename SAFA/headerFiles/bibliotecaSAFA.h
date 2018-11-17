#ifndef BIBLIOTECASAFA_H_
#define BIBLIOTECASAFA_H_

#include "kemmens/config.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "kemmens/SocketClient.h"

#define RUTA_CONFIG "SAFA.config"


#define TAMMAXPUERTO 6
#define TAMMAXIP 20

struct Configuracion_s
{
	int puerto;
	char algortimo[5];
	int quantum;
	int multiprogramacion;
	int retardo;

} typedef Configuracion;


Configuracion* settings;

void configurar();

#endif /* BIBLIOTECASAFA_H_ */
