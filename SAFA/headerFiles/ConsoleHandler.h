#ifndef HEADERFILES_CONSOLEHANDLER_H_
#define HEADERFILES_CONSOLEHANDLER_H_

///-------------INCLUSION DE BIBLIOTECAS-------------///

//------BIBLIOTECAS EXTERNAS------//

//------BIBLIOTECAS INTERNAS------//
#include "kemmens/CommandInterpreter.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/SocketServer.h"

//------BIBLIOTECAS PROPIAS------//
#include "../headerFiles/Scheduling.h"

///-------------VARIABLES GLOBALES-------------///

ThreadPool* threadPool;

//Variables externas, declaradas primero en Scheduling.h
extern pthread_mutex_t mutexREADY;
extern t_queue* scriptsQueue;					//Cola de scripts a asociar a un DTB, por comando ejecutar
extern t_queue* toBeEnded;						//Cola de DTBs a abortar, por comando finalizar

///-------------FUNCIONES DEFINIDAS------------///

void CommandEjecutar (int argC, char** args, char* callingLine, void* extraData);

void CommandStatus (int argC, char** args, char* callingLine, void* extraData);

void CommandFinalizar (int argC, char** args, char* callingLine, void* extraData);

void CommandMetricas (int argC, char** args, char* callingLine, void* extraData);

void CommandQuit (int argC, char** args, char* callingLine, void* extraData);

void ProcessLineInput(char* line);

#endif /* HEADERFILES_CONSOLEHANDLER_H_ */
