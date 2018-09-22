#ifndef MDJLIB_H_
#define MDJLIB_H_

#include "kemmens/config.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "kemmens/SocketClient.h"
#include "kemmens/SocketServer.h"
#include <stdio.h>
#include <string.h>

#include "Config.h"

ThreadPool* threadPool;
//Macros

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
