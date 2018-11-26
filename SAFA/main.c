///-------------INCLUSION DE BIBLIOTECAS-------------///

//------BIBLIOTECAS EXTERNAS------//

//------BIBLIOTECAS INTERNAS------//
#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
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

	if(argC == 1)
	{
		if(string_equals_ignore_case(args[1], "dam"))
		{
			//Seteo el socket del DAM en esa variable
			elDiego = *((int*)extraData);
		}
		else if(string_equals_ignore_case(args[1], "cpu"))
		{
			AddCPU((int*)extraData);
		}

		if(CPUsCount() > 0 && elDiego != -1)
		{
			corrupt = false;
		}
	}

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


void onPacketArrived(int socketID, int message_type, void* data)
{
	if(message_type == MESSAGETYPE_STRING)
	{
		ThreadableDoStructure* st = CommandInterpreter_MallocThreadableStructure();

		st->commandline = (char*)data;
		st->data = &socketID;
		st->separator = " ";
		st->postDo = (void*)postDo;

		ThreadPoolRunnable* run = ThreadPool_CreateRunnable();

		run->data = (void*)st;
		run->runnable = (void*)CommandInterpreter_DoThreaded;
		//Aca si necesitamos decirle al ThreadPool que libere data (o sea el st) en caso que se liberen todos los jobs
		run->free_data = (void*)CommandInterpreter_FreeThreadableDoStructure;

		ThreadPool_AddJob(threadPool, run);
	}
}

void ClientConnected(int socket)
{
	printf("Cliente se conecto! %d\n", socket);
}

void ClientDisconnected(int socket)
{
	RemoveCPU(socket); //Si no esta, no se va a sacar nada.
	printf("Cliente se fue! %d\n", socket);
}

void ClientError(int socketID, int errorCode)
{
	printf("Cliente %d se reporto con error %s!\n", socketID, strerror(errorCode));
}

void OnPostInterpreter(char* cmd, char* sep, void* args, bool actionFired)
{
	free(cmd);
}

void ProcessLineInput(char* line)
{
	ThreadableDoStructure* st = CommandInterpreter_MallocThreadableStructure();

	st->commandline = line;
	st->data = NULL;
	st->separator = " ";
	st->postDo = (void*)OnPostInterpreter;

	ThreadPoolRunnable* run = ThreadPool_CreateRunnable();

	run->data = (void*)st;
	run->runnable = (void*)CommandInterpreter_DoThreaded;
	run->free_data = (void*)CommandInterpreter_FreeThreadableDoStructure;

	ThreadPool_AddJob(threadPool, run);
}

/*
 * 	ACCION: Levanta el servidor (al SAFA se le conectan el DAM y los CPU, y mantiene sus ambos planificadores internamente
 * 			Registro los comandos del CommandInterpreter, y agrego los listeners al server
 * 			Lo pongo a escuchar conexiones entrantes y, como ultima linea, manejo el cierre del server al terminar
 */
void StartServer()
{

	//Registro de comandos funcionales y de comunicacion
	CommandInterpreter_RegisterCommand("iam", (void*)CommandIAm);
	//Registro de comandos de consola del SAFA
	CommandInterpreter_RegisterCommand("ejecutar", (void*)CommandEjecutar);
	CommandInterpreter_RegisterCommand("status", (void*)CommandStatus);
	CommandInterpreter_RegisterCommand("finalizar", (void*)CommandFinalizar);
	CommandInterpreter_RegisterCommand("metricas", (void*)CommandMetricas);
	CommandInterpreter_RegisterCommand("quit", (void*)CommandQuit);

	SocketServer_Start("SAFA", settings->puerto);
	SocketServer_ActionsListeners actions = INIT_ACTION_LISTENER;

	actions.OnConsoleInputReceived = (void*)ProcessLineInput;
	actions.OnPacketArrived = (void*)onPacketArrived;
	actions.OnClientConnected = (void*)ClientConnected;
	actions.OnClientDisconnect = (void*)ClientDisconnected;
	actions.OnReceiveError = (void*)ClientError;

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
	configurar();
	//Creo el ThreadPool, la lista de CPUs, las variables de Scheduling, la tabla de recursos y el CommandInterpreter
	threadPool = ThreadPool_CreatePool(10, false);
	InitCPUsHolder();
	InitSchedulingGlobalVariables();
	CommandInterpreter_Init();
	CreateResourcesTable();
	Logger_Log(LOG_INFO, "Variables globales inicializadas; listo para levantar el server!");

}

/*
 * 	ACCION: Liberar todas las variables globales inicializadas por el initialize
 */
void freeGlobalVariables()
{

	free(settings->algoritmo);
	free(settings);
	ThreadPool_FreeGracefully(threadPool);
	DestroyCPUsHolder();
	DeleteResources();
	DeleteSchedulingGlobalVariables();
	Logger_Log(LOG_INFO, "Liberada la memoria ocupada por variables globales");

}

int main(int argc, char **argv)
{

	initialize();
	StartServer();
	freeGlobalVariables();
	exit_gracefully(0);

}

