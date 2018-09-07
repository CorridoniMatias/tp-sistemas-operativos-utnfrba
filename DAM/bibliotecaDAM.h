#ifndef BIBLIOTECADAM_H_
#define BIBLIOTECADAM_H_

#include "kemmens/config.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "kemmens/SocketClient.h"

#define RUTA_CONFIG "DAM.config"

#define TAMMAXPUERTO 6
#define TAMMAXIP 20

struct Configuracion_s
{
	char puertoEscucha[TAMMAXPUERTO];
	char ipSAFA[TAMMAXIP];
	char puertoSAFA[TAMMAXPUERTO];
	char ipMDJ[TAMMAXIP];
	char puertoMDJ[TAMMAXPUERTO];
	char ipFM9[TAMMAXIP];
	char puertoFM9[TAMMAXPUERTO];
	int transferSize;
} typedef Configuracion;

ThreadPool* pozoDeHilos;

void configurar(Configuracion* configuracion);

#endif /* BIBLIOTECADAM_H_ */
