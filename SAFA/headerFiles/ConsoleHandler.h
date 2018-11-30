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
extern pthread_mutex_t mutexToBeEnded;
extern t_queue* scriptsQueue;					//Cola de scripts a asociar a un DTB, por comando ejecutar
extern t_queue* toBeEnded;						//Cola de DTBs a abortar, por comando finalizar

extern pthread_mutex_t mutexSettings;			//Esta viene de Settings.h

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

/*
 * 	ACCION: Interpretar que hacer cuando a traves de la consola llega un "status"
 * 			Seria mostrar info de las colas y algo de informacion de los DTBs de cada una (Shallow)
 * 			o bien mostrar informacion detallada de un DTB en particular, pasado por parametro (Deep)
 */
void CommandStatus (int argC, char** args, char* callingLine, void* extraData);

/*
 * 	ACCION: Interpretar que hacer cuando a traves de la consola llega un "finalizar"
 * 			Seria terminar la ejecucion de un DTB; lo que hago es encolar el ID del DTB
 * 			en la cola de DTBs a finalizar (mover a EXIT), tarea del PCP
 */
void CommandFinalizar (int argC, char** args, char* callingLine, void* extraData);

/*
 * 	ACCION: Interpretar que hacer cuando a traves de la consola llega un "metricas"
 * 			Seria fijarse en si hay un parametro o ninguno, y ahi detallar las metricas necesarias
 */
void CommandMetricas (int argC, char** args, char* callingLine, void* extraData);

/*
 * 	ACCION: Interpretar que hacer cuando a traves de la consola llega un "quit"
 * 			Seria cerrar el servidor, lo cual hace que se salga de la funcion StartServer
 */
void CommandQuit (int argC, char** args, char* callingLine, void* extraData);

#endif /* HEADERFILES_CONSOLEHANDLER_H_ */
