#ifndef HEADERFILES_SCHEDULING_H_
#define HEADERFILES_SCHEDULING_H_

#include "commons/collections/queue.h"
#include "commons/collections/dictionary.h"
#include "kemmens/Serialization.h"
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "../headerFiles/bibliotecaSAFA.h"
#include "../headerFiles/CPUsManager.h"

///-------------ESTRUCTURAS DEFINIDAS-------------///

/*
 * 	Estructura que representa un DTB (unidad planificable)
 * 	CAMPOS:
 * 		id: Identificador numerico y unico del DTB
 * 		pathEscriptorio: Ruta del script a ejecutar asociado al DTB, como cadena (mejor para CPU y MDJ)
 * 		pathLogicalAddress: Direccion logica del path asociado, empleada por el FM9 (el la define al cargar tras el Dummy)
 * 		programCounter: Contador que indica que linea del script se debe leer; 0 indica la primer linea
 * 		initialized: Flag numerico que indica si el DTB ha sido inicializado (puede pasar a READY) o no
 * 		status:	Codigo numerico que representa el estado del DTB (diagrama de 5 estados)
 * 		openedFilesAmount: Cantidad de archivos abiertos por el DTB
 * 		openedFiles: Diccionario que representa la tabla de archivos abiertos por el DTB. Key: path, Value: dirLogica
 * 		quantumRemainder: Cantidad de UTs del quantum que le quedan al DTB para ejecutar
 */
struct DTB_s
{
	int id;
	char* pathEscriptorio;
	uint32_t pathLogicalAddress;
	int programCounter;					//0 es para la primer linea
	int initialized;
	int status;
	int openedFilesAmount;
	t_dictionary* openedFiles;
	int quantumRemainder;
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
struct DeassignmentInfo_s
{
	int dtbID;
	int cpuSocket;
} typedef DeassignmentInfo;

/*
 * 	Estructura que contiene el socket de comunicacion con el CPU elegido al planificar, y el mensaje a enviarle
 * 	segun el algoritmo que se este empleando. Va a ser una variable global, externa desde aca y usada tambien en main
 */
struct AssignmentInfo_s
{
	void* message;
	int cpuSocket;
} typedef AssignmentInfo;

///-------------CONSTANTES DEFINIDAS-------------///

//Codigos de tareas del PLP
#define PLP_TASK_NORMAL_SCHEDULE 1 				//Planificar de NEW a READY, si se puede
#define PLP_TASK_CREATE_DTB 2					//Crear el DTB a inicializar mediante la operacion Dummy
#define PLP_TASK_INITIALIZE_DTB 3				//Inicializar el DTB cuyo DUMMY fue exitoso

//Codigos de tareas del PCP
#define PCP_TASK_NORMAL_SCHEDULE 11				//Planificar e ir pasando procesos de READY a EXEC
#define PCP_TASK_LOAD_DUMMY	12					//Pasar el Dummy de BLOCKED (no usado) a READY
#define PCP_TASK_BLOCK_DUMMY 13					//Bloquear el Dummy y actualizar la CPU desalojada
#define PCP_TASK_BLOCK_DTB 14					//Desalojar la CPU correspondiente y pasar su DTB de EXEC a BLOCK
#define PCP_TASK_UNLOCK_DTB 15					//Pasar DTB bloqueado a READY (cuando DAM aviso que termino I/O)
#define PCP_TASK_ABORT_DTB 16					//Ṕasar DTB a la cola de EXIT (si debio abortarse por algo)

//Estados de los DTB (del diagrama de 5 estados)
#define DTB_STATUS_NEW 21
#define DTB_STATUS_READY 22
#define DTB_STATUS_EXEC 23
#define DTB_STATUS_BLOCKED 24
#define DTB_STATUS_EXIT 25

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

extern Configuracion* settings;					//Estructura con la configuracion, externa desde bibliotecaSAFA.h

//Estas tres son externas en main
CreatableGDT* toBeCreated;						//Estructura con el path del script cuyo DTB quiero crear, y el ID del
												//DTB a inicializar al terminar la correspondiente operacion Dummy; para PLP

DeassignmentInfo* toBeMoved;						//Estructura con el DTB a mover de cola y el CPU a desalojar (si hace falta)

AssignmentInfo* toBeAssigned;					//Estructura con el mensaje a enviar y el CPU elegido en un ciclo de PCP


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
void* FlattenPathsAndAddresses(char** openedFiles);
void* GetMessageForCPU(DTB* chosenDTB);
void* ScheduleRR(int quantum);
void* ScheduleVRR(int maxQuantum);

t_dictionary* BuildDictionary(void* flattened, int amount);

#endif /* HEADERFILES_SCHEDULING_H_ */
