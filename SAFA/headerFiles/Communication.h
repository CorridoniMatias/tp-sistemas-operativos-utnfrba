#ifndef HEADERFILES_COMMUNICATION_H_
#define HEADERFILES_COMMUNICATION_H_

///-------------INCLUSION DE BIBLIOTECAS-------------///

//------BIBLIOTECAS EXTERNAS------//

//------BIBLIOTECAS INTERNAS------//

//------BIBLIOTECAS PROPIAS------//
#include "../headerFiles/Scheduling.h"

///-------------VARIABLES GLOBALES-------------///

//--VARIABLES DE OTROS MODULOS--//
extern t_list* cpus;									//Lista de CPUs conectados; declarada en CPUsManager.h
extern sem_t assignmentPending;							//Semaforo que indica que ya se asigno un DTB a un CPU
CreatableGDT* justDummied;								//Estructuras, no llevan extern aca sino en modulo originario
AssignmentInfo* toBeAssigned;
extern t_queue* toBeUnlocked;
extern t_queue* toBeBlocked;
extern t_queue* toBeEnded;

///-------------FUNCIONES DEFINIDAS------------///

#endif /* HEADERFILES_COMMUNICATION_H_ */
