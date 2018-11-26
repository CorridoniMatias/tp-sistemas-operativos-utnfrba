#ifndef HEADERFILES_COMMUNICATION_H_
#define HEADERFILES_COMMUNICATION_H_

///-------------INCLUSION DE BIBLIOTECAS-------------///

//------BIBLIOTECAS EXTERNAS------//

//------BIBLIOTECAS INTERNAS------//
#include "kemmens/SocketServer.h"

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

/*
 * 	ACCION: Funcion para meter en el threadPool, maneja un aviso de que termino la operacion dummy de un DTB (avisa DAM)
 * 			La cadena recibida deber tener el formato idDTBdummiado|pathScriptAsociado|direccionLogicaAsignadaAlPath
 */
void Comms_DummyFinished(void* arriveData);

/*
 * 	ACCION: Funcion para meter en el threadPool, maneja un aviso de que termino una operacion Abrir de un DTB (avisa DAM)
 * 			La cadena recibida deberia tener el formato idDTB|pathNuevoArchivoAbierto|direccionLogicaArchivo
 */
void Comms_AbrirFinished(void* arriveData);

/*
 * 	ACCION: Funcion para meter en el threadPool, maneja un aviso de que termino una operacion Abrir, Borrar o Flush (avisa DAM)
 * 			El dato recibido deberia ser simplemente un uint32_t*, para todas; uso esta funcion para las tres
 */
void Comms_CrearBorrarFlushFinished(void* arriveData);

/*
 * 	ACCION: Funcion para meter en el threadPool, maneja un aviso de que debe moverse un DTB a EXIT (ya sea que el DAM o el CPU
 * 			avisan de un error, o este ultimo de que termino el script). Todas solo reciben el id del DTB
 */
void Comms_KillDTBRequest(void* arriveData);

#endif /* HEADERFILES_COMMUNICATION_H_ */
