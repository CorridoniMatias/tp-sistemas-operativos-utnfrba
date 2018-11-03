#ifndef INCS_SCHEDULING_H_
#define INCS_SCHEDULING_H_

#include "commons/collections/queue.h"
#include "kemmens/Serialization.h"
#include "CPUsManager.h"
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

///-------------ESTRUCTURAS DEFINIDAS-------------///

/*
 * 	Estructura que representa un DTB (unidad planificable)
 * 	CAMPOS:
 * 		id: Identificador numerico y unico del DTB
 * 		pathEscriptorio: Ruta del script a ejecutar asociado al DTB
 * 		programCounter: Contador que indica que linea del script se debe leer; 0 indica la primer linea
 * 		initialized: Flag numerico que indica si el DTB ha sido inicializado (puede pasar a READY) o no
 * 		status:	Codigo numerico que representa el estado del DTB (diagrama de 5 estados)
 * 		openedFiles: Array de cadenas que simula la tabla de archivos abiertos por el DTB
 * 		quantumRemainder: Cantidad de UTs del quantum que le quedan al DTB para ejecutar
 */
struct DTB_s
{
	int id;
	char* pathEscriptorio;
	int programCounter;					//0 es para la primer linea
	int initialized;
	int status;
	int openedFilesAmount;				//Cantidad de archivos abiertos
	char** openedFiles;					//Tabla de archivos abiertos
	int quantumRemainder;				//Cantidad de UTs del quantum que le quedan
} typedef DTB;

/*
 * 	Estructura que sirve para almacenar el DTB y el script involucrados en la operacion a ejecutar por el PLP
 * 	Solo se utiliza cuando se le indica al PLP que debe crear un DTB para inicializarlo, o bien inicializar
 * 	el flag del DTB_Dummy y asi poder pasarlo a READY; o bien cuando se va a crear un GDT (no inicializado
 * 	aun) con el script que se indico por consola con la operacion ejecutar
 * 	CAMPOS:
 * 		dtbID: ID numerico del DTB ORIGINAL (no el del Dummy, que es 0) a inicializar y mover a READY
 * 		script: Script a asociar con el nuevo GDT a crear; enviado por consola a traves de ejecutar
 */
struct CreatableGDT_s
{
	int dtbID;
	char* script;
} typedef CreatableGDT;

/*
 * 	Estructura que contiene la info necesaria para el PCP cuando debe mover un DTB de una cola a otra
 * 	y, posiblemente, desalojar el CPU que se le habia asignado (si pasa de READY a BLOCKED)
 * 	Para saber de que cola a que cola mover, se fijara en el taskCode; NO USAR PARA MOVER DE READY A EXEC
 * 	CAMPOS:
 * 		dtbID: ID numerico del DTB involucrado (puede ser el Dummy) a mover de colas
 * 		cpuSocket: Descriptor de socket que identifica al CPU al cual se habia asignado el DTB desalojado
 */
struct AssignmentInfo_s
{
	int dtbID;
	int cpuSocket;
} typedef AssignmentInfo;

///-------------CONSTANTES DEFINIDAS-------------///

//Codigos de tareas del PLP
#define PLP_TASK_NORMAL_SCHEDULE 1 				//Planificar de NEW a READY, si se puede
#define PLP_TASK_CREATE_DTB 2					//Crear el DTB a inicializar mediante la operacion Dummy
#define PLP_TASK_INITIALIZE_DTB 3				//Inicializar el DTB cuyo DUMMY fue exitoso

//Codigos de tareas del PCP
#define PCP_TASK_NORMAL_SCHEDULE 1				//Planificar e ir pasando procesos de READY a EXEC
#define PCP_TASK_LOAD_DUMMY	2					//Pasar el Dummy de BLOCKED (no usado) a READY
#define PCP_TASK_BLOCK_DUMMY 3					//Bloquear el Dummy y actualizar la CPU desalojada
#define PCP_TASK_BLOCK_DTB 4					//Desalojar la CPU correspondiente y pasar su DTB de EXEC a BLOCK
#define PCP_TASK_UNLOCK_DTB 5					//Pasar DTB bloqueado a READY (cuando DAM aviso que termino I/O)

//Estados de los DTB (del diagrama de 5 estados)
#define DTB_STATUS_NEW 1
#define DTB_STATUS_READY 2
#define DTB_STATUS_EXEC 3
#define DTB_STATUS_BLOCKED 4
#define DTB_STATUS_EXIT 5

///-------------VARIABLES GLOBALES-------------///

//Semaforos a emplear; no deberian ser globales al programa main?
pthread_mutex_t mutexPLPtask;
pthread_mutex_t mutexPCPtask;
//ESte no seria externo del main tambien?
sem_t workPLP;									//Semaforo binario, para indicar que es hora de que el PLP trabaje

//Tareas a realizar de los planificadores, son externas en main para que se puedan modificar desde cualquier modulo
int PLPtask;
int PCPtask;

int nextID;										//ID a asignarle al proximo DTB que se cree

t_list* NEWqueue;								//"Cola" NEW, gestionada por PLP con FIFO; es lista para ser modificable
t_queue* READYqueue;							//Cola READY, gestionada por PCP
t_list* EXECqueue;								//"Cola" EXEC, en realidad es una lista (mas manejable), gestionada por PCP
t_list* BLOCKEDqueue;							//"Cola" BLOCKED, en realidad es una lista (mas manejable), gest. por PCP
t_list* EXITqueue;								//"Cola EXIT, en realidad es una lista (mas manejable)

DTB* dummyDTB;									//DTB que se usara como Dummy

//Estas dos son externas en main
CreatableGDT* toBeCreated;						//Estructura con el path del script cuyo DTB quiero crear, y el ID del
												//DTB a inicializar al terminar la correspondiente operacion Dummy; para PLP

AssignmentInfo toBeMoved;						//Estructura con el DTB a mover de cola y el CPU a desalojar (si hace falta)


///-------------FUNCIONES DEFINIDAS------------///


void InitQueuesAndLists();
void InitSemaphores();
void CreateDummy();
void InitGlobalVariables();
void SetPLPTask(int taskCode);
void SetPCPTask(int taskCode);

int ShowOptions();
void GestorDeProgramas();

DTB* CreateDTB(char* script);
void AddToNew(DTB* myDTB);
void AddToReady(DTB* myDTB);
void AddToBlocked(DTB* myDTB);
void AddToExec(DTB* myDTB);
bool IsDTBtoBeInitialized(DTB* myDTB);
bool IsInitialized(DTB* myDTB);
bool IsDummy(DTB* myDTB);
bool IsToBeMoved(DTB* myDTB);
DTB* GetNextDTB();

void PlanificadorLargoPlazo(void* gradoMultiprogramacion);

void PlanificadorCortoPlazo(void* algoritmo);
void* scheduleRR(int quantum);
void* scheduleVRR(int maxQuantum);

#endif /* INCS_SCHEDULING_H_ */
