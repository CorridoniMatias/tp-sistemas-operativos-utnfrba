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
#include "kemmens/Utils.h"


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
	uint32_t dtb;
	uint32_t programCounter;
	uint32_t quantum;
    t_dictionary* dictionary;
    int commandResult;
	int socketSAFA;
	int socketFM9;
	int socketDIEGO;
} typedef Operation;

Configuracion* settings;


void configurar();
int conectarAProceso(char* ip, char* puerto, char* nombreProceso); // Funcion creada por PEPE
void waitSafaOrders();
void executeDummy(DeserializedData* dtb, int socketDiego, int socketSafa);
char* askLineToFM9(uint32_t idDtb, uint32_t logicDir, uint32_t pc, int fm9);
void CommandAbrir(int argC, char** args, char* callingLine, void* extraData);
void CommandConcentrar(int argC, char** args, char* callingLine, void* extraData);
void CommandAsignar(int argC, char** args, char* callingLine, void* extraData);
void CommandWait(int argC, char** args, char* callingLine, void* extraData);
void CommandSignal(int argC, char** args, char* callingLine, void* extraData);
void CommandFlush(int argC, char** args, char* callingLine, void* extraData);
void CommandClose(int argC, char** args, char* callingLine, void* extraData);
void CommandCrear(int argC, char** args, char* callingLine, void* extraData);
void CommandBorrar(int argC, char** args, char* callingLine, void* extraData);
t_dictionary* BuildDictionary(void* flattened, int amount);
bool openFileVerificator(t_dictionary* dictionary,char* path);
SerializedPart FlattenPathsAndAddresses(t_dictionary* openFilesTable);
void Start_commands();




#endif /* BIBLIOTECACPU_H_ */
