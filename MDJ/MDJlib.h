#ifndef MDJLIB_H_
#define MDJLIB_H_

#include "kemmens/config.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "kemmens/SocketClient.h"
#include "kemmens/SocketServer.h"
#include <stdio.h>
#include <string.h>

ThreadPool* threadPool;
//Macros

#define RUTA_CONFIG "mdj.config"
#define RUTA_METADATA "/Metadata/Metadata.bin"
#define MAXPATHLENGTH 50				//se cuenta el /0

struct Configuracion_s
{
	int tamanioBloque;
	int cantidadBloques;
	char* puntoMontaje;
	int puertoEscucha;
	int delay;
} typedef Configuracion;

Configuracion* config;

void configurar();

void freeGlobals();

void startServer();

void initGlobals();

void* CommandIAm (int argc, char** args, char* comando, void* datos);

void clientConnected(int socket);

void clientDisconnected(int unSocket);

void onPacketArrived(int socket, int tipoMensaje, void* datos);

void* postDo(char* cmd, char* sep, void* args, bool fired);

void ClientError(int socketID, int errorCode);

#endif /* MDJLIB_H_ */
