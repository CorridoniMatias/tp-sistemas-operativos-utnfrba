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
 * 		initialized: Flag numerico que indica si el DTB ha sido inicializado; todos lo tienen en 1, salvo el Dummy
 * 		status:	Codigo numerico que representa el estado del DTB (diagrama de 5 estados)
 * 		openedFilesAmount: Cantidad de archivos abiertos por el DTB
 * 		openedFiles: Diccionario que representa la tabla de archivos abiertos por el DTB. Key: path, Value: dirLogica (uint32_t*)
 * 		quantumRemainder: Cantidad de UTs del quantum que le quedan al DTB para ejecutar
 */
struct DTB_s
{
	uint32_t id;
	char* pathEscriptorio;
	uint32_t pathLogicalAddress;
	uint32_t programCounter;					//0 es para la primer linea
	uint32_t initialized;
	int status;
	uint32_t openedFilesAmount;
	t_dictionary* openedFiles;
	uint32_t quantumRemainder;
} typedef DTB;

/*
 * 	Estructura que va a ir a la cola toBeUnlocked, con la info necesaria y polimorfica para pasar DTBs a READY
 * 	CAMPOS:
 * 		id: ID del DTB que quiero mover a READY
 * 		newProgramCounter: Valor actualizado del program counter del DTB en su script
 * 		openedFilesUpdate: Diccionario con los archivos a (posiblemente) actualizar en la tabla de archivos abiertos
 * 						   Puede informar tanto nuevos archivos abiertos como cerrados; CPU pasa todos, DAM nuevo abierto
 */
struct UnlockableInfo_s
{
	uint32_t id;
	uint32_t newProgramCounter;
	t_dictionary* openedFilesUpdate;
} typedef UnlockableInfo;

/*
 * 	Estructura que va a ir a la cola toBeBlocked, con la info necesaria a actualizar en un DTB que pasara a BLOCKED
 * 	CAMPOS:
 * 		id: ID del DTB que quiero mover a BLOCKED
 * 		newProgramCounter: Valor actualizado del program counter del DTB en su script
 *		quantumRemainder: Unidades de quantum que sobraron de la ejecucion del DTB
 *		openedFilesUpdate: Diccionario con los archivos a (posiblemente) actualizar en la tabla de abiertos,
 *						   por si se hubiera hecho una operacion close o un cambio de DL
 */
struct BlockableInfo_s
{
	uint32_t id;
	uint32_t newProgramCounter;
	uint32_t quantumRemainder;
	t_dictionary* openedFilesUpdate;
} typedef BlockableInfo;

/*
 * 	Estructura que sirve para almacenar el id del DTB, el path de su script y la direccion logica del mismo
 * 	una vez que se finalizo la carga de dicho script a memoria (segun aviso el DAM). Sirve para la operacion
 * 	del PLP de PLP_TASK_INITIALIZE_DTB (pasar ese DTB a READY, ya inicializado, y con la direccion logica,
 * 	antes no conocida, del script, la cual sera requerida por el FM9)
 * 	CAMPOS:
 * 		dtbID: ID numerico del DTB ORIGINAL (el que le habia impuesto al Dummy al arrancar) a inicializar y mover a READY
 * 		script: Script asociado a dicho DTB, el cual acaba de ser cargado en memoria
 * 		logicalAddress: Direccion logica del script, la cual debo pasarle al CPU al cual lo asigne, para que se la de al FM9
 */
struct CreatableGDT_s
{
	uint32_t dtbID;
	char* script;
	uint32_t logicalAddress;
} typedef CreatableGDT;

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
#define PLP_TASK_NORMAL_SCHEDULE 11 			//Planificar de NEW a READY, si se puede
#define PLP_TASK_CREATE_DTB 12					//Crear DTBs con scripts que haya en la cola
#define PLP_TASK_INITIALIZE_DTB 13				//Inicializar el DTB cuyo DUMMY fue exitoso

//Codigos de tareas del PCP
#define PCP_TASK_NORMAL_SCHEDULE 21				//Planificar e ir pasando procesos de READY a EXEC
#define PCP_TASK_LOAD_DUMMY	22					//Pasar el Dummy de BLOCKED (no usado) a READY
#define PCP_TASK_FREE_DUMMY 23					//Liberar el Dummy (ponerlo en BLOCKED); actualizar la CPU desalojada (por afuera)
#define PCP_TASK_BLOCK_DTB 24					//Desalojar la CPU correspondiente y pasar su DTB de EXEC a BLOCK
#define PCP_TASK_UNLOCK_DTB 25					//Pasar DTB bloqueado a READY (cuando DAM aviso que termino I/O
												//o una operacion de signal libero un recurso que esperaba)
#define PCP_TASK_END_DTB 26						//Ṕasar DTB a la cola de EXIT (si debio abortarse o termino)

//Estados de los DTB (del diagrama de 5 estados)
#define DTB_STATUS_NEW 31
#define DTB_STATUS_READY 32
#define DTB_STATUS_EXEC 33
#define DTB_STATUS_BLOCKED 34
#define DTB_STATUS_EXIT 35

///-------------VARIABLES GLOBALES-------------///

//Semaforos a emplear; no deberian ser globales al programa main?
pthread_mutex_t mutexPLPtask;
pthread_mutex_t mutexPCPtask;
//ESte no seria externo del main tambien?
sem_t workPLP;									//Semaforo binario, para indicar que es hora de que el PLP trabaje

//Tareas a realizar de los planificadores, son externas en main para que se puedan modificar desde cualquier modulo
int PLPtask;
int PCPtask;

uint32_t nextID;								//ID a asignarle al proximo DTB que se cree
int inMemoryAmount;								//Cantidad de procesos actualmente en memoria; para el grado de multiprogr.

//Estas colas no necesitan mutexes, las usa el SAFA de a uno??
t_queue* NEWqueue;								//Cola NEW, gestionada por PLP con FIFO; es lista para ser modificable
t_queue* READYqueue;							//Cola READY, gestionada por PCP
t_list* EXECqueue;								//"Cola" EXEC, en realidad es una lista (mas manejable), gestionada por PCP
t_list* BLOCKEDqueue;							//"Cola" BLOCKED, en realidad es una lista (mas manejable), gest. por PCP
t_list* EXITqueue;								//"Cola EXIT, en realidad es una lista (mas manejable)

DTB* dummyDTB;									//DTB que se usara como Dummy

extern Configuracion* settings;					//Estructura con la configuracion, externa desde bibliotecaSAFA.h

//Todas son externas en main y los que las usen

CreatableGDT* justDummied;						//Estructura con el path del script, la direccion logica y el id del DTB
												//cuya carga Dummy acaba de terminar (segun informo el DAM)

AssignmentInfo* toBeAssigned;					//Estructura con el mensaje a enviar y el CPU elegido en un ciclo de PCP

t_queue* scriptsQueue;							//Cola de char*s con los scripts que van quedando para ejecutar

t_queue* toBeUnlocked;							//Cola de UnlockableInfo*s con los IDs de los DTBs que deben ser
												//pasados a READY cuando el PCP tenga posibilidad de hacerlo
												//Tocada por mensajes del CPU, del DAM y del ResourceManager

t_queue* toBeBlocked;							//Cola de BlockableInfo*s con los IDs de los DTBs que deben ser pasados a
												//BLOCKED porque volvieron de IO o se bloquearon con un wait recurso
												//Tocada solo por mensajes del CPU (de desalojo)

t_queue* toBeEnded;								//Cola de int*s con los IDs de los DTBs que deben ser pasados a
												//EXIT por haber terminado su archivo o haberse producido un error
												//Tocada solo por mensajes del CPU (error o fin de script)

///-------------FUNCIONES DEFINIDAS------------///

/*
 * 	ACCION: Crea todas las colas y listas necesarias; las de estados de proceso y las de trabajos pendientes
 */
void InitQueuesAndLists();

/*
 * 	ACCION: Crea e inicializa los semaforos utilizados en el modulo
 */
void InitSemaphores();

/*
 * 	ACCION: Crea el DTB Dummy con un estado y un ID basura
 */
void CreateDummy();

/*
 * 	ACCION: Setea y mallocea las variables globales, y crea las colas, listas y semaforos
 */
void InitGlobalVariables();

/*
 * 	ACCION: Setea la tarea a realizar por el PLP, garantizando mutua exclusion, y le avisa al mismo que ha de trabajar
 * 	CAMPOS:
 * 		taskCode: Codigo de la tarea a realizar; ver mas arriba
 */
void SetPLPTask(int taskCode);

/*
 * 	ACCION: Setea la tarea a realizar por el PCP, garantizando mutua exclusion
 * 	CAMPOS:
 * 		taskCode: Codigo de la tarea a realizar; ver mas arriba
 */
void SetPCPTask(int taskCode);

/*
 * 	ACCION: Muestra las opciones de la consola del S-AFA
 */
int ShowOptions();

/*
 * 	Este deberia suplantarse por el OnConsoleInput del CommandInterpreter
 */
void GestorDeProgramas();

/*
 * 	ACCION: Crea un DTB nuevo con un cierto script asociado, sin estado y solo pre-malloceado
 * 	CAMPOS:
 * 		script: Script de lenguaje Escriptorio que tendra asociado el DTB
 */
DTB* CreateDTB(char* script);

/*
 * 	ACCION: Alterar el estado interno de un DTB, y ponerlo en la cola de NEW
 * 	CAMPOS:
 * 		myDTB: DTB, en forma de estructura, que quiero mover
 */
void AddToNew(DTB* myDTB);

/*
 * 	ACCION: Alterar el estado interno de un DTB, y ponerlo en la cola de READY
 * 	CAMPOS:
 * 		myDTB: DTB, en forma de estructura, que quiero mover
 */
void AddToReady(DTB* myDTB);

/*
 * 	ACCION: Alterar el estado interno de un DTB, y ponerlo en la cola de BLOCKED
 * 	CAMPOS:
 * 		myDTB: DTB, en forma de estructura, que quiero mover
 */
void AddToBlocked(DTB* myDTB);

/*
 * 	ACCION: Alterar el estado interno de un DTB, y ponerlo en la cola de EXEC
 * 	CAMPOS:
 * 		myDTB: DTB, en forma de estructura, que quiero mover
 */
void AddToExec(DTB* myDTB);

/*
 * 	ACCION: Alterar el estado interno de un DTB, y ponerlo en la cola de EXIT
 * 	CAMPOS:
 * 		myDTB: DTB, en forma de estructura, que quiero mover
 */
void AddToExit(DTB* myDTB);

/*
 * 	ACCION: Closure para verificar si cada DTB de una lista es el Dummy o no (por el flag)
 * 	CAMPOS:
 * 		myDTB: DTB elemento de la lista, pasado automaticamente por parametro al aplicarle una funcion de orden superior
 */
bool IsDummy(DTB* myDTB);

/*
 * 	ACCION: Obtener un puntero al proximo DTB a ejecutar, el primero de la cola READY
 */
DTB* GetNextDTB();


/*
 * 	ACCION: Funcion para el hilo del PLP, con todas sus posibles acciones a llevar a cabo
 * 	CAMPOS:
 * 		gradoMultiprogramacion: Grado de multiprogramacion del sistema al ejecutar el PLP, determinara el pase de procesos
 */
void PlanificadorLargoPlazo(void* gradoMultiprogramacion);
/*
 * 	ACCION: Cargar los datos del DTB elegido para inicializar en la estructura del Dummy; NO LO MUEVE A READY
 * 	CAMPOS:
 * 		id: ID del DTB que estaba en NEW y fue elegido para hacersele la carga en memoria
 * 		path: Path del script asociado a dicho DTB; sera el que el DAM debe hablar para cargar en memoria
 */
void SetDummy(uint32_t id, char* path);

/*
 * 	ACCION: Funcion para el hilo del PCP, con todas sus posibles acciones a llevar a cabo
 * 	CAMPOS:
 * 		algoritmo: Nombre del algoritmo de planificacion que se esta usando; debe ser un puntero, por si cambia en tiempo real
 */
void PlanificadorCortoPlazo(void* algoritmo);

/*
 * 	ACCION: Actualiza y sobreescribe toda clave de una tabla de archivos de un DTB con los datos de un diccionario
 * 	CAMPOS:
 * 		toBeUpdated: DTB cuyo diccionario quiero actualizar
 * 		currentOFs: Diccionario con los datos ya actualizados, desde donde voy a copiar
 */
void UpdateOpenedFiles(DTB* toBeUpdated, t_dictionary* currentOFs);

/*
 * 	ACCION: Generar cadena del formato arch1:dl1,arch2:dl2,...,archN,dlN; en base a un diccionario
 * 	CAMPOS:
 * 		openFilesTable: Tabla de archivos abiertos de la cual hallar los paths y las direcciones logicas
 */
void* FlattenPathsAndAddresses(t_dictionary* openFilesTable);

/*
 * 	ACCION: Obtener la cadena serializada con todos los datos a mandarle a un CPU para indicarle una ejecucion
 * 	CAMPOS:
 * 		chosenDTB: DTB del cual sacar los datos para el mensaje serializado
 */
void* GetMessageForCPU(DTB* chosenDTB);

/*
 * 	ACCION:	Planificar con el algoritmo Round Robin para obtener el proximo DTB a ejecutar,
 * 			moverlo a la cola de EXEC y obtener el mensaje a enviarle al CPU que luego se asigne
 * 	CAMPOS:
 * 		quantum: Quantum definido por el sistema como maximo de ejecucion
 */
void* ScheduleRR(int quantum);

/*
 * 	ACCION: Planificar con el algoritmo Virtual Round RObin para obtener el proximo DTB a ejecutar,
 * 			moverlo a la cola de EXEC y obtener el mensaje a enviarle al CPU que luego se asigne
 * 	CAMPOS:
 * 		maxQuantum: Maximo quantum posible, segun definio el sistema, que podria asignarsele al DTB elegido
 */
void* ScheduleVRR(int maxQuantum);


/*
 * 	ACCION: Elimina apropiadamente todos los semaforos utilizados
 */
void DeleteSemaphores();

/*
 * 	ACCION: Closure para poder liberar la memoria de los scripts de la scriptsQueue
 * 	CAMPOS:
 * 		script: Script (char*) que representa un elemento de dicha cola, pasado automaticamente como parametro en el destroy
 */
void ScriptDestroyer(void* script);

/*
 * 	ACCION: Closure para poder liberar la memoria de los data* de las tablas de archivos abiertos
 * 	CAMPOS:
 * 		addressPtr: Puntero que representa el value del diccionario que esta siendo destruido; parametro automatico
 */
void LogicalAddressDestroyer(void* addressPtr);

/*
 * 	ACCION: Closure para poder liberar a memoria de cada BlockableInfo de la cola de DTBs a bloquear
 * 	CAMPOS:
 * 		BI: Estructura de info a bloquear (debe castearse), parametro automatico en el destroy
 */
void BlockableInfoDestroyer(void* BI);

/*
 * 	ACCION: Closure para poder liberar a memoria de cada UnlockableInfo de la cola de DTBs a desbloquear
 * 	CAMPOS:
 * 		UI: Estructura de info a desbloquear (debe castearse), parametro automatico en el destroy
 */
void UnlockableInfoDestroyer(void* UI);

/*
 * 	ACCION: Closure para poder liberar a memoria de la cola de IDs de procesos a enviar a EXIT
 * 	CAMPOS:
 * 		endableID: Puntero con el ID del DTB a eliminar de la cola de procesos a terminar; parametro automatico
 */
void EndableDestroyer(void* endableID);

/*
 * 	ACCION: Closure para liberar la memoria de una estructura de DTB de una cola o lista de DTBs
 * 	CAMPOS:
 * 		aDTB: void*, como cualquier parametro de destroyer, automatico, para saber cual liberar
 */
void DTBDestroyer(void* aDTB);

/*
 * 	ACCION: Eliminar todas las colas y listas, destruyendo y liberando su memoria
 */
void DeleteQueuesAndLists();

/*
 * 	ACCION: Eliminar toda variable global, semaforo, cola y lista empleada en el modulo
 */
void DeleteGlobalVariables();

#endif /* HEADERFILES_SCHEDULING_H_ */
