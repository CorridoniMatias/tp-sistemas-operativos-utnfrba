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

///-------------FUNCIONES DEFINIDAS------------///

void InitCPUsHolder();

void AddCPU(int* socketID);

void FreeCPU(int socketID);

void RemoveCPU(int socketID);

int CPUsCount();

void DestroyCPUsHolder();

bool IsIdle(void* myCPU);

bool ExistsIdleCPU();


#endif /* CPUSMANAGER_H_ */
