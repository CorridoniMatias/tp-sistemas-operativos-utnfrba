#ifndef HEADERFILES_SCHEDULING_H_
#define HEADERFILES_SCHEDULING_H_

///-------------INCLUSION DE BIBLIOTECAS-------------///

//------BIBLIOTECAS EXTERNAS------//
#include "commons/collections/queue.h"
#include "commons/collections/dictionary.h"
#include <semaphore.h>
#include <unistd.h>

//------BIBLIOTECAS INTERNAS------//
#include "kemmens/Serialization.h"
#include "kemmens/Utils.h"

//------BIBLIOTECAS PROPIAS------//
#include "../headerFiles/CPUsManager.h"
#include "../headerFiles/Settings.h"
#include "../headerFiles/ResourceManager.h"

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
 * 		ioOperations: Cantidad de operaciones de entrada/salida (MDJ) realizadas por el DTB; para el algoritmo Propio (IOBF)
 * 		sentencesWhileAtNEW: Cantidad de sentencias que se ejecutaron en el sistema mientras estaba en NEW; para la metrica 1
 * 		spawnTime: Instante en el cual ingreso al sistema (en el cual ingreso a la cola NEW)
 * 		firstResponseTime: Instante en el cual recibe la primera respuesta del sistema (primera respuesta de IO)
 * 		arrivalAtREADYtime: Instante (con nanosegundos) en el cual es movido a la cola READY
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
	uint32_t ioOperations;
	int sentencesWhileAtNEW;
	time_t spawnTime;
	time_t firstResponseTime;
	struct timespec arrivalAtREADYtime;
	t_list* resourcesKept;
} typedef DTB;

/*
 * 	Estructura que va a ir a la cola toBeUnlocked, con la info necesaria y polimorfica para pasar DTBs a READY
 * 	CAMPOS:
 * 		id: ID del DTB que quiero mover a READY
 * 		overwritePC: Booleano para saber si debe sobreescribirse el PC al mover de nuevo a READY o no (si informo el DAM)
 * 		newProgramCounter: Valor actualizado del program counter del DTB en su script
 * 		appendOFs: Booleano para saber si solo se deben añadir archivos (sin pisar el diccionario existente)
 * 				   Debe valer true si se busca desbloquear tras un signal recurso, o si fue una operacion Abrir exitosa
 * 		openedFilesUpdate: Diccionario con los archivos a (posiblemente) actualizar en la tabla de archivos abiertos
 * 						   Puede informar tanto nuevos archivos abiertos como cerrados; CPU pasa todos, DAM nuevo abierto
 */
struct UnlockableInfo_s
{
	uint32_t id;
	bool overwritePC;
	uint32_t newProgramCounter;
	bool appendOFs;
	t_dictionary* openedFilesUpdate;
} typedef UnlockableInfo;

/*
 * 	Estructura que va a ir a la cola toBeBlocked, con la info necesaria a actualizar en un DTB que pasara a BLOCKED
 * 	CAMPOS:
 * 		id: ID del DTB que quiero mover a BLOCKED
 * 		newProgramCounter: Valor actualizado del program counter del DTB en su script
 *		quantumRemainder: Unidades de quantum que sobraron de la ejecucion del DTB
 *		dummyComeback: Flag para saber si es una vuelta del Dummy a BLOCKED y no hace falta actualizar su diccionario
 *		openedFilesUpdate: Diccionario con los archivos a (posiblemente) actualizar en la tabla de abiertos,
 *						   por si se hubiera hecho una operacion close o un cambio de DL
 */
struct BlockableInfo_s
{
	uint32_t id;
	uint32_t newProgramCounter;
	uint32_t quantumRemainder;
	bool dummyComeback;
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
 * 	Estructura usada por el hilo del PCP, para poder guardar el nombre y el flag de cambio de su algoritmo
 * 	y asi poder consultarla directamente, sin necesidad de hacer una region critica enorme o varios lock-unlock
 */
struct AlgorithmStatus_s
{
	char name[8];
	int changeType;
	uint32_t quantum;
	int delay;

} typedef AlgorithmStatus;

///-------------CONSTANTES DEFINIDAS-------------///

//Codigos de tareas del PLP
#define PLP_TASK_NORMAL_SCHEDULE 11 			//Planificar de NEW a READY, si se puede
#define PLP_TASK_CREATE_DTB 	 12				//Crear DTBs con scripts que haya en la cola
#define PLP_TASK_INITIALIZE_DTB  13				//Inicializar el DTB cuyo DUMMY fue exitoso

//Codigos de tareas del PCP
#define PCP_TASK_NORMAL_SCHEDULE 21				//Planificar e ir pasando procesos de READY a EXEC
#define PCP_TASK_LOAD_DUMMY		 22				//Pasar el Dummy de BLOCKED (no usado) a READY
#define PCP_TASK_FREE_DUMMY 	 23				//Liberar el Dummy (ponerlo en BLOCKED); actualizar la CPU desalojada (por afuera)
#define PCP_TASK_BLOCK_DTB 		 24				//Desalojar la CPU correspondiente y pasar su DTB de EXEC a BLOCK
#define PCP_TASK_UNLOCK_DTB 	 25				//Pasar DTB bloqueado a READY (cuando DAM aviso que termino I/O
												//o una operacion de signal libero un recurso que esperaba)
#define PCP_TASK_END_DTB 		26				//Pasar DTB a la cola de EXIT (si debio abortarse o termino)

//Estados de los DTB (del diagrama de 5 estados)
#define DTB_STATUS_NEW 			31
#define DTB_STATUS_READY 		32
#define DTB_STATUS_EXEC 		33
#define DTB_STATUS_BLOCKED 		34
#define DTB_STATUS_EXIT 		35
#define DTB_STATUS_DUMMYING		36

///-------------VARIABLES GLOBALES-------------///

//--VARIABLES EXTERNAS--//
Configuracion* settings;						//Estructura con la configuracion, externa desde Settings.h
extern pthread_mutex_t mutexSettings;			//Para garantizar acceso unico a la configuracion; propio de Settings.h
extern int algorithmChange;						//Valor que registra de que a que algoritmo se paso, por si debo mover colas

//--SEMAFOROS A EMPLEAR--//
pthread_mutex_t mutexPLPtasksQueue;
pthread_mutex_t mutexPCPtasksQueue;
pthread_mutex_t mutexNEW;						//Mutex sobre la cola NEW, para cuando modifico sus elementos
pthread_mutex_t mutexREADY;						//Garantiza mutua exclusion sobre las colas READY (la actual);
												//extern en ConsoleHandler.h, ya que este lo usara al hacer metricas/status
pthread_mutex_t mutexScriptsQueue;				//Mutua exclusion sobre la cola de scripts; usado tanto en este mismo
												//modulo como en ConsoleHandler.h, al hacer el comando "ejecutar"
pthread_mutex_t mutexToBeBlocked;				//Usado en Communication.h (extern)
pthread_mutex_t mutexToBeUnlocked;				//Usado en Communication.h y ResourceManager.h (extern)
pthread_mutex_t mutexToBeEnded;					//Usado en Communication.h y ConsoleHandler.h (extern)
pthread_mutex_t mutexBeingDummied;				//Totalmente interno al modulo, para evitar concurrencia sobre lista de DTBs siendo dummizados
pthread_mutex_t mutexDummiedQueue;				//Para tener mutua exclusion sobre la cola de Dummies satifsactorios (extern en Communication.h)
pthread_mutex_t mutexEXEC;						//Para tener mutua exclusion sobre EXEC (extern en ResourceManager.h)

extern pthread_mutex_t mutexCPUs;				//Proveniente de CPUsManager.h, para manejar de a uno la lista de CPUs

sem_t workPLP;									//Semaforo binario, para indicar que es hora de que el PLP trabaje;
												//no es extern ya que lo modifico con una funcion de aca (SetPLPTask)
sem_t workPCP;									//Semaforo binario, para indicar que es hora de que el PCP trabaje;
												//tampoco es extern, lo modifico con una funcion de aca (SetPCPTask)

//--TAREAS A REALIZAR POR LOS PLANIFICADORES--//

t_queue* PLPtasksQueue;							//Cola de ints (codigos de tarea del PLP) con las tareas a realizar por el PLP
t_queue* PCPtasksQueue;							//Cola de ints (codigos de tarea del PCP) con las tareas a realizar por el PCP

//--VARIABLES EMPLEADAS POR PROCESOS PROPIOS DEL MODULO--//
uint32_t nextID;								//ID a asignarle al proximo DTB que se cree
int inMemoryAmount;								//Cantidad de procesos actualmente en memoria; para el grado de multiprogr.
int algorithmChange;							//Codigo del cambio de algoritmo sufrido; constantes definidas arriba


//--VARIABLES DE ESTRUCTURAS DE PROCESOS PROPIOS--//
DTB* dummyDTB;									//DTB que se usara como Dummy
t_list* beingDummied;							//Lista de DTBs que estan haciendo el Dummy (deberia ser solo uno)
												//Contiene info relevante de los mismos, para no perderla

t_queue* dummiedQueue;							//Cola de CreatableGDTs con informacion de los procesos que ya realizaron la operacion Dummy
												//extern en Communication.h, originaria de aca (para DummyFinished)

//--COLAS DE PLANIFICACION GLOBALES--//
t_queue* NEWqueue;								//Cola NEW, gestionada por PLP con FIFO; es lista para ser modificable
t_list* EXECqueue;								//"Cola" EXEC, en realidad es una lista (mas manejable), gestionada por PCP
t_list* BLOCKEDqueue;							//"Cola" BLOCKED, en realidad es una lista (mas manejable), gest. por PCP
t_list* EXITqueue;								//"Cola EXIT, en realidad es una lista (mas manejable)

//--COLAS DE PLANIFICACION DE PROCESOS LISTOS--//
//Colas de READY, estas si irian con mutex ya que son usadas por ambos planificadores a la vez; una para cada algoritmo
t_queue* READYqueue_RR;
t_list* READYqueue_VRR;							//Lista para buscar por remanente de quantum y simular doble cola
t_list* READYqueue_Own;							//Lista para poder ordenar por prioridad IO


//--COLAS DE TAREAS A REALIZAR--//
t_queue* scriptsQueue;							//Cola de char*s con los scripts que van quedando para ejecutar; ext en CH.h

t_queue* toBeUnlocked;							//Cola de UnlockableInfo*s con los IDs de los DTBs que deben ser
												//pasados a READY cuando el PCP tenga posibilidad de hacerlo
												//Tocada por mensajes del CPU, del DAM y del ResourceManager;
												//extern en ResourceManager.h y Communication.h

t_queue* toBeBlocked;							//Cola de BlockableInfo*s con los IDs de los DTBs que deben ser pasados a
												//BLOCKED porque volvieron de IO o se bloquearon con un wait recurso
												//Tocada solo por mensajes del CPU (de desalojo); ext. en Communication.h

t_queue* toBeEnded;								//Cola de int*s con los IDs de los DTBs que deben ser pasados a
												//EXIT por haber terminado su archivo o haberse producido un error
												//Tocada solo por mensajes del CPU (error o fin de script), del
												//DAM (errores) y de la consola (comando finalizar);
												//extern en Communication.h (mensajes de CPU o DAM) y CH.h (comando finalizar)

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
void InitSchedulingGlobalVariables();

/*
 * 	ACCION: Agrega una tarea a realizar por el PLP a la cola, sin concurrencia, y avisandole a este que debe trabajar
 * 	PARAMETROS:
 * 		taskCode: Codigo de la tarea a realizar; ver mas arriba
 */
void AddPLPTask(int taskCode);

/*
 * 	ACCION: Agrega una tarea a realizar por el PCP a la cola, sin concurrencia, y avisandole a este que debe trabajar
 * 	PARAMETROS:
 * 		taskCode: Codigo de la tarea a realizar; ver mas arriba
 */
void AddPCPTask(int taskCode);

/*
 * 	ACCION: Crea un DTB nuevo con un cierto script asociado, sin estado y solo pre-malloceado; contadores e instante nulos
 * 	PARAMETROS:
 * 		script: Script de lenguaje Escriptorio que tendra asociado el DTB
 */
DTB* CreateDTB(char* script);

/*
 * 	ACCION: Alterar el estado interno de un DTB, y ponerlo en la cola de NEW
 * 	PARAMETROS:
 * 		myDTB: DTB, en forma de estructura, que quiero mover
 */
void AddToNew(DTB* myDTB);

/*
 * 	ACCION: Alterar el estado interno de un DTB, y ponerlo en la cola de READY
 * 	PARAMETROS:
 * 		myDTB: DTB, en forma de estructura, que quiero mover
 * 		currentAlgorithm: Algoritmo que se esta usando actualmente, para saber que cola de READY revisar
 */
void AddToReady(DTB* myDTB, char* currentAlgorithm);

/*
 * 	ACCION: Alterar el estado interno de un DTB, y ponerlo en la cola de BLOCKED
 * 	PARAMETROS:
 * 		myDTB: DTB, en forma de estructura, que quiero mover
 */
void AddToBlocked(DTB* myDTB);

/*
 * 	ACCION: Alterar el estado interno de un DTB, y ponerlo en la cola de EXEC
 * 	PARAMETROS:
 * 		myDTB: DTB, en forma de estructura, que quiero mover
 */
void AddToExec(DTB* myDTB);

/*
 * 	ACCION: Alterar el estado interno de un DTB, y ponerlo en la cola de EXIT
 * 	PARAMETROS:
 * 		myDTB: DTB, en forma de estructura, que quiero mover
 */
void AddToExit(DTB* myDTB);

/*
 * 	ACCION: Closure para verificar si cada DTB de una lista es el Dummy o no (por el flag)
 * 	PARAMETROS:
 * 		myDTB: DTB elemento de la lista, pasado automaticamente por parametro al aplicarle una funcion de orden superior
 */
bool IsDummy(void* myDTB);

/*
 * 	ACCION: Closure para mostrar solo id y script asociado de un DTB; aplicar ante el comando status de consola
 * 	PARAMETROS:
 * 		aDTB: DTB, elemento de alguna cola de planificacion (parametro automatico)
 */
void ShowDTBInfo_Shallow(void* aDTB);

/*
 * 	ACCION: Closure para mostrar ruta y direccion logica de cada archivo abierto (diccionario) de un DTB
 * 	PARAMETROS:
 * 		path: Ruta del archivo, key del diccionario, parametro automatico
 * 		address: Direccion logica, value del diccionario, parametro automatico
 */
void ShowPathAndAddress(char* path, void* address);

/*
 * 	ACCION: Retornar el puntero al DTB que se desea; util para el comando status de la consola
 * 	PARAMETROS:
 * 		desiredID: ID del DTB cuya informacion se quiere saber
 * 		algorithm: Algoritmo actual de planificacion del PCP, para saber en cual cola de READY buscar
 */
DTB* GetDTBbyID(uint32_t desiredID, char* algorithm);

/*	ACCION: Mostrar toda la informacion (campo por campo) de un DTB; ante un status con parametro
 * 	PARAMETROS:
 * 		target: DTB (es un puntero posta) que coincide con el del parametro del comando; obtenido por busqueda
 */
void ShowDTBInfo_Deep(DTB* target);

/*
 * 	ACCION: Obtener un puntero al proximo DTB a ejecutar, el primero de la cola READY
 */
DTB* GetNextReadyDTB();

/*
 * 	ACCION: Cargar los datos del DTB elegido para inicializar en la estructura del Dummy; NO LO MUEVE A READY
 * 	PARAMETROS:
 * 		id: ID del DTB que estaba en NEW y fue elegido para hacersele la carga en memoria
 * 		path: Path del script asociado a dicho DTB; sera el que el DAM debe hablar para cargar en memoria
 */
void SetDummy(uint32_t id, char* path);

/*
 * 	ACCION: Funcion para el hilo del PLP, con todas sus posibles acciones a llevar a cabo
 */
void PlanificadorLargoPlazo();

/*
 * 	ACCION: Actualizar los contadores de sentencias esperadas en NEW de todos los DTBs de esa cola (para metrica 1)
 * 	PARAMETROS:
 * 		amount: Cantidad de sentencias ejecutadas que se deben agregar a cada DTB
 */
void AggregateSentencesWhileAtNEW(int amount);

/*
 * 	ACCION: Funcion para el hilo del PCP, con todas sus posibles acciones a llevar a cabo
 * 	PARAMETROS:
 * 		algoritmo: Nombre del algoritmo de planificacion que se esta usando; debe ser un puntero, por si cambia en tiempo real
 */
void PlanificadorCortoPlazo();

/*
 * 	ACCION: Closure para poder ordenar una lista por prioridad de DTBs, de manera descendiente (algoritmo IOBF, propio)
 * 			Desempata por el instante de llegada a READY
 * 	PARAMETROS:
 * 		dtbOne, dtbTwo: DTBs a comparar por su cantidad de operaciones de E/S y su llega a READY, son elementos de la lista
 */
bool DescendantPriority(void* dtbOne, void* dtbTwo);

/*
 * 	ACCION: Funcion para informar si no hay DTBs en READY y no tiene sentido planificar con el PCP
 * 	PARAMETROS:
 * 		algorithm: Nombre textual del algoritmo usado para el PCP; ayuda a decidir que cola/lista de READY revisar
 */
bool NoReadyDTBs(char* algorithm);

/*
 * 	ACCION: Acomodar las colas (copiar y mover DTBs entre ellas) de READY previo a un ciclo del PCP,
 * 			por si hubiera habido un cambio de algoritmo mediante factores externos; deja vacia la cola antigua
 * 	PARAMETROS:
 * 		changeCode: Codigo del tipo de cambio de algoritmo (de cual a cual fue)
 */
void MoveQueues(int changeCode);

/*
 * 	ACCION: Actualiza y sobreescribe toda clave de una tabla de archivos de un DTB con los datos de un diccionario
 * 	PARAMETROS:
 * 		toBeUpdated: DTB cuyo diccionario quiero actualizar
 * 		currentOFs: Diccionario con los datos ya actualizados, desde donde voy a copiar
 * 		justAddOne: Booleano para saber si es una adicion simple o una copia entera (ver si vaciar anterior o no)
 */
void UpdateOpenedFiles(DTB* toBeUpdated, t_dictionary* currentOFs, bool justAddOne);

/*
 * 	ACCION: Generar cadena del formato arch1:dl1,arch2:dl2,...,archN,dlN; en base a un diccionario
 * 	PARAMETROS:
 * 		openFilesTable: Tabla de archivos abiertos de la cual hallar los paths y las direcciones logicas
 */
SerializedPart FlattenPathsAndAddresses(t_dictionary* openFilesTable);

/*
 * 	ACCION: Obtener la cadena serializada (y su tamanio) con todos los datos a mandarle a un CPU para indicarle una ejecucion
 * 	PARAMETROS:
 * 		chosenDTB: DTB del cual sacar los datos para el mensaje serializado
 */
SerializedPart* GetMessageForCPU(DTB* chosenDTB);

/*
 * 	ACCION:	Planificar con el algoritmo Round Robin para obtener el proximo DTB a ejecutar,
 * 			moverlo a la cola de EXEC y obtener el mensaje a enviarle al CPU que luego se asigne
 * 	PARAMETROS:
 * 		quantum: Quantum definido por el sistema como maximo de ejecucion
 */
SerializedPart* ScheduleRR(int quantum);

/*
 * 	ACCION: Planificar con el algoritmo Virtual Round Robin para obtener el proximo DTB a ejecutar,
 * 			moverlo a la cola de EXEC y obtener el mensaje a enviarle al CPU que luego se asigne
 * 	PARAMETROS:
 * 		maxQuantum: Maximo quantum posible, segun definio el sistema, que podria asignarsele al DTB elegido
 */
SerializedPart* ScheduleVRR(int maxQuantum);

/*
 * 	ACCION: Planificar con el algoritmo propio asignado por la catedra para obtener el proximo DTB a ejecutar,
 * 			moverlo a la cola de EXEC y obtener el mensaje a enviarle al CPU que luego se asigne.
 * 			Dicho algoritmo es un Round Robin con ordenamiento por prioridades, privilegiando a aquellos
 * 			con mas operaciones de IO hechas (en base a dicho campo). RR para seguir mandando quantum
 * 	PARAMETROS:
 * 		maxQuantum: Quantum definido por el sistema como maximo de ejecucion
 */
SerializedPart* ScheduleIOBF(int quantum);

/*
 * 	ACCION: Devuelve la cantidad total de sentencias ejecutadas en el sistema, este como este
 * 			cada DTB. Usada para la metrica 4 (porcentaje de sentencias que fueron al DAM)
 */
int Metrics_TotalRunSentencesAmount();

/*
 * 	ACCION: Devuelve la cantidad de sentencias ejecutadas del sistema que fueron de IO (llamadas al DAM). Para metrica 4
 */
int Metrics_TotalIOSentencesAmount();

/*
 * 	ACCION: Devuelve la cantidad total de DTBs en el sistema, sin contar NEW y hasta EXIT inclusive. Para metrica 2
 */
int Metrics_TotalDTBPopulation();

/*
 * 	ACCION: Devuelve la METRICA 2. Cantidad promedio de sentencias ejecutadas que usan al DAM (segun cantidad de procesos)
 */
float Metrics_AverageIOSentences();

/*
 * 	ACCION: Devuelve la METRICA 3. Cantidad promedio de sentencias ejecutadas que llevan a un DTB a EXIT (ya sea error o fin)
 */
float Metrics_AverageExitingSentences();

/*
 * 	ACCION: Devuelve la METRICA 4. Porcentaje del total de sentencias ejecutadas que correspondian a operaciones de IO
 */
float Metrics_IOSentencesPercentage();

/*
 * 	ACCION: Devuelve la suma de todos los tiempos de espera de los DTBs del sistema; usado para metrica 5
 */
float Metrics_SumAllResponseTimes();

/*
 * 	ACCION: Devuelve la cantidad de DTBs que recibieron la primer respuesta del sistema
 * 			Con esta cantidad se como calcular la metrica 5 (es el divisor en la cuenta)
 */
int Metrics_ResponsedDTBPopulation();

/*
 * 	ACCION: Devuelve la METRICA 5. Tiempo de respuesta promedio del sistema.
 * 	Definir que es el tiempo Promedio, cuando se declara el tiempo de respuesta
 */
float Metrics_AverageResponseTime();

/*
 * 	ACCION: Elimina apropiadamente todos los semaforos utilizados
 */
void DeleteSemaphores();

/*
 * 	ACCION: Closure para poder liberar la memoria de los scripts de la scriptsQueue
 * 	PARAMETROS:
 * 		script: Script (char*) que representa un elemento de dicha cola, pasado automaticamente como parametro en el destroy
 */
void ScriptDestroyer(void* script);

/*
 * 	ACCION: Closure para poder liberar la memoria de los data* de las tablas de archivos abiertos
 * 	PARAMETROS:
 * 		addressPtr: Puntero que representa el value del diccionario que esta siendo destruido; parametro automatico
 */
void LogicalAddressDestroyer(void* addressPtr);

/*
 * 	ACCION: Closure para poder liberar a memoria de cada BlockableInfo de la cola de DTBs a bloquear
 * 	PARAMETROS:
 * 		BI: Estructura de info a bloquear (debe castearse), parametro automatico en el destroy
 */
void BlockableInfoDestroyer(void* BI);

/*
 * 	ACCION: Closure para poder liberar a memoria de cada UnlockableInfo de la cola de DTBs a desbloquear
 * 	PARAMETROS:
 * 		UI: Estructura de info a desbloquear (debe castearse), parametro automatico en el destroy
 */
void UnlockableInfoDestroyer(void* UI);

/*
 * 	ACCION: Closure para poder liberar a memoria de la cola de IDs de procesos a enviar a EXIT
 * 	PARAMETROS:
 * 		endableID: Puntero con el ID del DTB a eliminar de la cola de procesos a terminar; parametro automatico
 */
void EndableDestroyer(void* endableID);

/*
 * 	ACCION: Closure para liberar la memoria de una estructura de DTB de una cola o lista de DTBs
 * 	PARAMETROS:
 * 		aDTB: void*, como cualquier parametro de destroyer, automatico, para saber cual liberar
 */
void DTBDestroyer(void* aDTB);

/*
 * 	ACCION: Closure para liberar la memoria de una estructura de carga Dummy satisfactoria (CreatableGDT) de la cola dummiedQueue
 * 	PARAMETROS:
 * 		aDummied: void*, formato defecto de los destroyer, automatico, para saber cual liberar
 */
void CreatableGDTDestroyer(void* aDummied);

/*
 * 	ACCION: Eliminar todas las colas y listas, destruyendo y liberando su memoria
 */
void DeleteQueuesAndLists();

/*
 * 	ACCION: Eliminar toda variable global, semaforo, cola y lista empleada en el modulo
 */
void DeleteSchedulingGlobalVariables();

#endif /* HEADERFILES_SCHEDULING_H_ */
