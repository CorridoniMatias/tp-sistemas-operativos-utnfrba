#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "kemmens/SocketServer.h"
#include "incs/CPUsManager.h"
#include "incs/ConsoleHandler.h"

bool corrupt = true;
int elDiego = -1;

void *CommandIAm (int argC, char** args, char* callingLine, void* extraData)
{
	if(argC == 1)
	{
		if(string_equals_ignore_case(args[1], "dam"))
		{
			elDiego = *(int*)extraData;
		} else if(string_equals_ignore_case(args[1], "cpu"))
		{
			AddCPU((int*)extraData);
		}

		if(CPUsCount() > 0 && elDiego != -1)
			corrupt = false;
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


void StartServer()
{
	CommandInterpreter_Init();
	InitCPUsHolder();
	threadPool = ThreadPool_CreatePool(10, false);

	CommandInterpreter_RegisterCommand("iam", (void*)CommandIAm);
	CommandInterpreter_RegisterCommand("ejecutar", (void*)CommandEjecutar);
	CommandInterpreter_RegisterCommand("status", (void*)CommandStatus);
	CommandInterpreter_RegisterCommand("finalizar", (void*)CommandFinalizar);
	CommandInterpreter_RegisterCommand("metricas", (void*)CommandMetricas);
	CommandInterpreter_RegisterCommand("quit", (void*)CommandQuit);

	SocketServer_Start("SAFA", 8080);
	SocketServer_ActionsListeners actions = INIT_ACTION_LISTENER;

	actions.OnConsoleInputReceived = (void*)ProcessLineInput;
	actions.OnPacketArrived = (void*)onPacketArrived;
	actions.OnClientConnected = (void*)ClientConnected;
	actions.OnClientDisconnect = (void*)ClientDisconnected;
	actions.OnReceiveError = (void*)ClientError;

	SocketServer_ListenForConnection(actions);
	DestroyCPUsHolder();
	Logger_Log(LOG_INFO, "Server Shutdown.");
}


int main(int argc, char **argv)
{
	Logger_CreateLog("./SAFA.log", "SAFA", true);
	Logger_Log(LOG_INFO, "Proceso SAFA iniciado...");
	StartServer();
	ThreadPool_FreeGracefully(threadPool);
	exit_gracefully(0);
}

