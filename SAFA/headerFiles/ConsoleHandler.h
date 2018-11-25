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

/*
 * 	ACCION: Mostrar, para cada cola, la cantidad de procesos en ellas, mas el ID y el script asociado a cada uno
 * 	PARAMETROS:
 * 		PCPalgorithm: Algoritmo actualmente utilizado por el PCP, para saber que cola de READY mirar
 */
void ShowQueuesInformation(char* PCPalgorithm);

/*
 * 	ACCION: Interpretar que hacer cuando a traves de la consola llega un "ejecutar"
 * 			Seria agregar un script a ejecutar, el mismo es el segundo elemento de args; avisarle al PLP
 */
void CommandEjecutar (int argC, char** args, char* callingLine, void* extraData);

void CommandStatus (int argC, char** args, char* callingLine, void* extraData);

void CommandFinalizar (int argC, char** args, char* callingLine, void* extraData);

void CommandMetricas (int argC, char** args, char* callingLine, void* extraData);

void CommandQuit (int argC, char** args, char* callingLine, void* extraData);

void ProcessLineInput(char* line);

#endif /* HEADERFILES_CONSOLEHANDLER_H_ */
