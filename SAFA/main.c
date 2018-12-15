///-------------INCLUSION DE BIBLIOTECAS-------------///

//------BIBLIOTECAS EXTERNAS------//

//------BIBLIOTECAS INTERNAS------//
#include "kemmens/megekemmen.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "kemmens/SocketMessageTypes.h"
#include "kemmens/SocketServer.h"

//------BIBLIOTECAS PROPIAS------//
#include "headerFiles/ConsoleHandler.h"
#include "headerFiles/ResourceManager.h"
#include "headerFiles/Communication.h"

bool corrupt = true;
int elDiego = -1;
ThreadPool* threadPool;

/*
 * 	Comando del CommandInterpreter, ante un mensaje "iam" (handshake con un componente)
 */
void *CommandIAm (int argC, char** args, char* callingLine, void* extraData)
{
//	printf("\nculo:%s\n", callingLine);
	if(argC == 1)
	{
		if(string_equals_ignore_case(args[1], "dam"))
		{
			//Seteo el socket del DAM en esa variable
			elDiego = *((int*)extraData);
		}
		else if(string_equals_ignore_case(args[1], "cpu"))
		{
//			printf("cpu socket = %d\n",*((int*)extraData));
			AddCPU((int*)extraData);
//			printf("llego cpu\n");
//			printf("%d\n",CPUsCount());
		}

		if(CPUsCount() > 0 && elDiego != -1)
		{
			corrupt = false;
		}
	}

	free(extraData);
	CommandInterpreter_FreeArguments(args);
	return 0;

}

void* postDo(char* cmd, char* sep, void* args, bool fired)
{
	/*if(!fired)
		SocketCommons_SendMessageString((int)args, "Lo recibido no es comando!");*/

	free(cmd);
	return 0;
}

void OnPostInterpreter(char* cmd, char* sep, void* args, bool actionFired)
{
	free(cmd);
}

void ThreadedCommandInterpreter(char* line, void* extraData)
{
	ThreadableDoStructure* st = CommandInterpreter_MallocThreadableStructure();

	st->commandline = line;
	st->data = extraData;
	st->separator = " ";
	st->postDo = (void*)OnPostInterpreter;

	ThreadPoolRunnable* run = ThreadPool_CreateRunnable();

	run->data = (void*)st;
	run->runnable = (void*)CommandInterpreter_DoThreaded;
	run->free_data = (void*)CommandInterpreter_FreeThreadableDoStructure;

	ThreadPool_AddJob(threadPool, run);
}

/*
 * 	ACCION: Maneja la recepcion de mensajes que llegan a traves de un socket, ya sean del DAM o del CPU
 * 			Cada mensaje recibido tiene una funcion asociada a ejecutar, detalladas en Communication.c
 */
void onPacketArrived(int socketID, int message_type, void* data)
{

	ThreadPoolRunnable* run = ThreadPool_CreateRunnable();

	OnArrivedData* arriveData;
	arriveData = SocketServer_CreateOnArrivedData();

	arriveData->calling_SocketID = socketID;
	arriveData->receivedData = data;

	run->data = arriveData;

	//OJO: al no haber un free data para arrive data (run->free_data) podrian haber memory leaks si no se ejecuta el job que se manda al pool!

	switch(message_type)
	{
		case MESSAGETYPE_DAM_SAFA_DUMMY:
			run->runnable = Comms_DAM_DummyFinished;
			break;

		case MESSAGETYPE_DAM_SAFA_ABRIR:
			run->runnable = Comms_DAM_AbrirFinished;
			break;

		case MESSAGETYPE_DAM_SAFA_CREAR:
		case MESSAGETYPE_DAM_SAFA_BORRAR:
		case MESSAGETYPE_DAM_SAFA_FLUSH:
			run->runnable = Comms_DAM_CrearBorrarFlushFinished;
			break;

		case MESSAGETYPE_DAM_SAFA_ERR:
			run->runnable = Comms_DAM_IOError;
			break;

		case MESSAGETYPE_CPU_EOFORABORT:
			run->runnable = Comms_CPU_ErrorOrEOF;
			break;

		case MESSAGETYPE_CPU_BLOCKDUMMY:
			run->runnable = Comms_CPU_DummyAtDAM;
			break;

		case MESSAGETYPE_CPU_BLOCKDTB:
			run->runnable = Comms_CPU_DTBAtDAM;
			break;

		case MESSAGETYPE_CPU_EOQUANTUM:
			run->runnable = Comms_CPU_OutOfQuantum;
			break;

		case  MESSAGETYPE_CPU_WAIT:
			run->runnable = Comms_CPU_WaitResource;
			break;

		case MESSAGETYPE_CPU_SIGNAL:
			run->runnable = Comms_CPU_SignalResource;
			break;

		case MESSAGETYPE_STRING:
		{
//			printf("%s",(char*)data);
			declare_and_init(p_socketfd, int, socketID);
			ThreadedCommandInterpreter((char*)data, (void*)p_socketfd);
			free(run);
			free(arriveData);
			run = NULL;
			break;
		}


		default:
			free(run);
			free(arriveData);
			free(data);
			run = NULL;
			break;
	}

	if(run != NULL)
	{
		ThreadPool_AddJob(threadPool, run);
	}

}

void ClientConnected(int socket)
{
	printf("Se conecto un cliente a traves del socket %d!\n", socket);
}

void ClientDisconnected(int socket)
{

	int foundAndRemoved;
	//Intento sacar el CPU con ese socket (si es que habia uno) y, por las dudas, lo guardo en un int
	foundAndRemoved = RemoveCPU(socket);
	printf("Se desconecto el cliente a traves del socket %d!\n", socket);
	if(foundAndRemoved == 1)
	{
		printf("El cliente desconectado era un CPU!");
	}

}

void ClientError(int socketID, int errorCode)
{
	printf("Se reporto un error del cliente %d: %s!\n", socketID, strerror(errorCode));
}


void ProcessLineInput(char* line)
{
	ThreadedCommandInterpreter(line, NULL);
}

/*
 * 	ACCION: Levanta el servidor (al SAFA se le conectan el DAM y los CPU, y mantiene sus ambos planificadores internamente
 * 			Registro los comandos del CommandInterpreter, y agrego los listeners al server
 * 			Lo pongo a escuchar conexiones entrantes, arranco los hilos de los planificadores y, como ultima
 * 			linea, me encargo del cierre del servidor
 */
void StartServer()
{

	//Registro de comandos funcionales y de comunicacion
	CommandInterpreter_RegisterCommand("iam", (void*)CommandIAm);
	//Registro de comandos de consola del SAFA
	CommandInterpreter_RegisterCommand("ejecutar", CommandEjecutar);
	CommandInterpreter_RegisterCommand("status", CommandStatus);
	CommandInterpreter_RegisterCommand("finalizar", CommandFinalizar);
	CommandInterpreter_RegisterCommand("metricas", CommandMetricas);
	CommandInterpreter_RegisterCommand("quit", CommandQuit);

	SocketServer_Start("SAFA", settings->puerto);
	SocketServer_ActionsListeners actions = INIT_ACTION_LISTENER;

	actions.OnConsoleInputReceived = (void*)ProcessLineInput;
	actions.OnPacketArrived = (void*)onPacketArrived;
	actions.OnClientConnected = (void*)ClientConnected;
	actions.OnClientDisconnect = (void*)ClientDisconnected;
	actions.OnReceiveError = (void*)ClientError;

	ThreadPoolRunnable* plpJob = ThreadPool_CreateRunnable();
	plpJob->data = NULL;
	plpJob->free_data = NULL;
	plpJob->runnable = (void*)PlanificadorLargoPlazo;
	ThreadPool_AddJob(threadPool, plpJob);
	Logger_Log(LOG_INFO, "Planificador de Largo Plazo ya operativo");

	ThreadPoolRunnable* pcpJob = ThreadPool_CreateRunnable();
	pcpJob->data = NULL;
	pcpJob->free_data = NULL;
	pcpJob->runnable = (void*)PlanificadorCortoPlazo;
	ThreadPool_AddJob(threadPool, pcpJob);
	Logger_Log(LOG_INFO, "Planificador de Corto Plazo ya operativo");

	SocketServer_ListenForConnection(actions);

	Logger_Log(LOG_INFO, "Server Shutdown.");

}

/*
 * 	ACCION: Inicializa las variables globales y los elementos globales de los modulos
 */
void initialize()
{

	//Creo el logger y logeo dicho evento; voy a logear aca para ir debugeando
	Logger_CreateLog("./SAFA.log", "SAFA", true);
	Logger_Log(LOG_INFO, "Proceso SAFA iniciado...");
	//Armo la estructura de configuracion en base al archivo
	Configurar();
	//Creo el ThreadPool, la lista de CPUs, las variables de Scheduling, la tabla de recursos y el CommandInterpreter
	InitCPUsHolder();
	Logger_Log(LOG_INFO, "Inicializado deposito de CPUs");
	InitSchedulingGlobalVariables();
	Logger_Log(LOG_INFO, "Inicializados los planificadores");
	CommandInterpreter_Init();
	Logger_Log(LOG_INFO, "Inicializado interprete de comandos");
	CreateResourcesTable();
	Logger_Log(LOG_INFO, "Creada tabla de recursos");
	threadPool = ThreadPool_CreatePool(20, false);
	Logger_Log(LOG_INFO, "Creado ThreadPool para 10 threads");

	//Agrego al ThreadPool la tarea de monitorear el archivo de configuracion; tiene un read, debe ir en hilo aparte
	ThreadPoolRunnable* watcherJob = ThreadPool_CreateRunnable();
	watcherJob->data = NULL;
	watcherJob->free_data = NULL;
	watcherJob->runnable = MonitorConfigFile;
	ThreadPool_AddJob(threadPool, watcherJob);
	Logger_Log(LOG_INFO, "Ya se esta monitoreando el archivo de configuracion");

	Logger_Log(LOG_INFO, "Variables globales inicializadas; listo para levantar el server!");

}

/*
 * 	ACCION: Liberar todas las variables globales inicializadas por el initialize
 */
void freeGlobalVariables()
{

	ThreadPool_FreeGracefully(threadPool);
	DestroyCPUsHolder();
	DeleteResources();
	DeleteSchedulingGlobalVariables();
	DeleteSettingsVariables();
	Logger_Log(LOG_INFO, "Liberada la memoria ocupada por variables globales");

}

int main(int argc, char **argv)
{

	initialize();
	StartServer();
	freeGlobalVariables();
	exit_gracefully(0);

}

