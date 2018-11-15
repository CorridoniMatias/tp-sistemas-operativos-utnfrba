#ifndef BIBLIOTECACPU_H_
#define BIBLIOTECACPU_H_

#include <unistd.h>
#include "kemmens/SocketMessageTypes.h"
#include "kemmens/config.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "kemmens/SocketClient.h"
#include "kemmens/SocketCommons.h"
#include "kemmens/Serialization.h"
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

//ESTO ES EL STRUCT DE EXTRADATA QUE LE PASO AL COMMAND INTERPRETER
struct Operation_s
{
	int dtb;
	int programCounter;
	int quantum;
    t_dictionary* dictionary;
	int socketSAFA;
	int socketFM9;
	int socketDIEGO;
} typedef Operation;

Configuracion* settings;


void configurar();
int conectarAProceso(char* ip, char* puerto, char* nombreProceso); // Funcion creada por PEPE
void waitSafaOrders();
void executeDummy(DeserializedData dtb, int socketDiego, int socketSafa);
char* askLineToFM9(DeserializedData dtb, int socketfm9);
void* CommandAbrir();
void* CommandConcentrar();
void* CommandAsignar();
void* CommandWait();
void* CommandSignal();
void* CommandFlush();
void* CommandClose();
void* CommandCrear();
void* CommandBorrar();
t_dictionary* BuildDictionary(void* flattened, int amount);
bool openFileVerificator(t_dictionary* dictionary,char* path);



#endif /* BIBLIOTECACPU_H_ */
