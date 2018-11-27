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
void Comms_DAM_DummyFinished(void* arriveData);

/*
 * 	ACCION: Funcion para meter en el threadPool, maneja un aviso de que termino una operacion Abrir de un DTB (avisa DAM)
 * 			La cadena recibida deberia tener el formato idDTB|pathNuevoArchivoAbierto|direccionLogicaArchivo
 */
void Comms_DAM_AbrirFinished(void* arriveData);

/*
 * 	ACCION: Funcion para meter en el threadPool, maneja un aviso de que termino una operacion Abrir, Borrar o Flush (avisa DAM)
 * 			El dato recibido deberia ser simplemente un uint32_t*, para todas; uso esta funcion para las tres
 */
void Comms_DAM_CrearBorrarFlushFinished(void* arriveData);

/*
 *	ACCION: Funcion para meter en el threadPool, maneja un aviso de que debe moverse un DTB a EXIT tras un
 * 			aviso del DAM (hubo un error en una operacion de entrada/salida). Solo recibe el id del DTB
 */
void Comms_DAM_IOError(void* arriveData);

/*
 * 	ACCION: Funcion para meter en el threadPool, maneja un aviso de que debe moverse un DTB a EXIT tras un
 * 			aviso del CPU (termino un script, o se debe abortar). Solo recibe el id del DTB. Utiliza el
 * 			socket (guardado en el data que llego) que lo comunico para desalojar dicho DTB.
 */
void Comms_CPU_ErrorOrEOF(void* arriveData);

/*
 * 	ACCION: Funcion para meter en el threadPool, maneja un aviso del CPU de que la inicializacion del script
 * 			del DTB Dummy ya esta en manos del DAM, con lo cual deberia desalojarse ese CPU y liberarse el
 * 			Dummy, moviendolo a la cola de BLOCKED (donde reside en espera de ser cargado con un nuevo script y
 * 			movido de nuevo a READY). Solo recibe el ID del DTB
 */
void Comms_CPU_DummyAtDAM(void* arriveData);

/*
 * 	ACCION: Funcion para meter en el threadPool, maneja un aviso del CPU de que el DTB que estaba ejecutando
 * 			requiere hacer una operacion de entrada/salida, con lo cual ahora es responsabilidad del DAM (el cual
 * 			avisara cuando la misma termine) y debe desalojarse el CPU. Caso contrario, puede que haya habido
 * 			un error en una operacion de wait (mediada directamente entre CPU y SAFA) y este deba ser
 * 			tambien desalojado; ambos casos llevan el DTB a BLOCKED. Solo se requiere el ID del DTB
 */
void Comss_CPU_DTBAtDAM(void* arriveData);

#endif /* HEADERFILES_COMMUNICATION_H_ */
