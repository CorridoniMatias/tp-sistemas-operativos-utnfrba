#ifndef BIBLIOTECACPU_H_
#define BIBLIOTECACPU_H_

#include <unistd.h>
#include "kemmens/config.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "kemmens/SocketClient.h"
#include "kemmens/SocketCommons.h"
#include "kemmens/Serialization.h"
#include "kemmens/SocketMessageTypes.h"
#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
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
void waitSafaOrders();
void executeDummy(DeserializedData dtb, int socketDiego, int socketSafa);


#endif /* BIBLIOTECACPU_H_ */
