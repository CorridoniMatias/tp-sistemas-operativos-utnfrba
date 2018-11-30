#ifndef CPUSMANAGER_H_
#define CPUSMANAGER_H_

///-------------INCLUSION DE BIBLIOTECAS-------------///

//------BIBLIOTECAS EXTERNAS------//
#include "stdlib.h"
#include "commons/collections/list.h"

//------BIBLIOTECAS INTERNAS------//

//------BIBLIOTECAS PROPIAS------//

///-------------ESTRUCTURAS DEFINIDAS-------------///

/*
 * 	Estructura que almacena la informacion de cada CPU conectado al sistema
 * 	CAMPOS:
 * 		socket: Socket por el cual se establece la comunicacion con el CPU
 * 		busy: Flag que indica si dicho CPU esta ocupado o no (si esta desalojado, sin DTB)
 */
struct CPU_s
{
	int socket;
	bool busy;
} typedef CPU;

///-------------VARIABLES GLOBALES-------------///

t_list* cpus;										//Cada elemento es un CPU*; extern en Communication.h para poder alterarla
pthread_mutex_t mutexCPUs;							//Por si se agregan CPUs mientras estoy ejecutando algun planificador
													//extern en Scheduling.h para poder declararla ahi

///-------------FUNCIONES DEFINIDAS------------///

/*
 * 	ACCION: Crea la lista de CPUs (para manejar los disponibles) y el mutex de acceso a la misma
 */
void InitCPUsHolder();

/*
 * 	ACCION: Agrega un nuevo CPU a la lista, en estado desocupado
 * 	PARAMETROS:
 * 		socketID: FD del socket a traves del cual comunicarme con el CPU recien ingresado
 */
void AddCPU(int* socketID);

/*
 * 	ACCION: Marca un CPU como desocupado (busy = false)
 * 	PARAMETROS:
 * 		socketID: FD del socket de comunicacion con el CPU, el cual lo identifica dentro de la lista
 */
void FreeCPU(int socketID);

/*
 * 	ACCION: Intenta quitar un CPU de la lista de conectados del sistema; si devuelve 1, es porque dio uno de baja
 * 	PARAMETROS:
 * 		socketID: FD del socket de comunicacion con el CPU, el cual lo identifica dentro de la lista
 */
int RemoveCPU(int socketID);

/*
 * 	ACCION: Retorna la cantidad de CPUs conectados al sistema en un momento
 */
int CPUsCount();

/*
 * 	ACCION: Closure que determina si un CPU esta libre, para usar en un list_find
 * 	PARAMETROS:
 * 		myCPU: Automatico, provisto por el list_find o la funcion de orden superior que lo llame
 */
bool IsIdle(void* myCPU);

/*
 * 	ACCION: Devuelve true si existe algun CPU libre, para verificar si vale la pena planificar (en modulo de Scheduling)
 */
bool ExistsIdleCPU();

/*
 * 	ACCION: Destroyer individual de cada CPU, para aplicar en el DestroyCPUsHolder
 * 	PARAMETROS:
 * 		aCPU: Parametro automatico del list_destroy, que apunta al elemento CPU
 */
void CPUDestroyer(void* aCPU);

/*
 * 	ACCION: Destruye la lista de CPUs, sus elementos (CPU*s) y el mutex de acceso a la misma
 */
void DestroyCPUsHolder();


#endif /* CPUSMANAGER_H_ */
