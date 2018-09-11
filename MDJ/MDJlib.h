#ifndef MDJLIB_H_
#define MDJLIB_H_

#include "kemmens/config.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "kemmens/SocketClient.h"
#include "kemmens/SocketServer.h"

//Macros

#define RUTA_CONFIG "MDJ.config"
#define MAXPORTLENGTH 6				//se cuenta el /0
#define MAXIPLENGTH 20				//se cuenta el /0

struct Configuracion_s
{
	char puertoEscucha[MAXPORTLENGTH];
	char ipDiego[MAXIPLENGTH];
	char puertoDiego[MAXPORTLENGTH];
	int delay;
} typedef Configuracion;

Configuracion* config;

void configurar(Configuracion* configuracion);

void startServer();

void* commandIam (int argc, char** args, char* comando, void* datos);

void clientConnected(int socket);

void clientDisconnected(int unSocket);

void onPacketArrived(int socket, int tipoMensaje, void* datos);

void* postDo(char* cmd, char* sep, void* args, bool fired);

void ClientError(int socketID, int errorCode);

#endif /* MDJLIB_H_ */
