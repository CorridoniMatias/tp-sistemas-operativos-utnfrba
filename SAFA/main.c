#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "commons/collections/list.h"
#include "kemmens/SocketServer.h"

bool corrupt = true;
ThreadPool* threadPool;
int elDiego = -1;
t_list* cpus;


void *CommandIAm (int argC, char** args, char* callingLine, void* extraData)
{
	if(argC == 1)
	{
		if(string_equals_ignore_case(args[0], "dam"))
		{
			elDiego = (int)extraData;
		} else if(string_equals_ignore_case(args[0], "cpu"))
		{
			list_add(cpus, extraData);
		}

		if(list_size(cpus) > 0 && elDiego != -1)
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
		st->data = (void*)socketID;
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

void processLineInput(char* line)
{
	printf("Comando: %s\n", line);

	CommandInterpreter_Do(line, " ", NULL);

	free(line);
}

void ClientConnected(int socket)
{
	printf("Cliente se conecto! %d\n", socket);
}

void ClientDisconnected(int socket)
{
	//TODO: Verificar si se desconecto un CPU y sacarlo de la lista de cpus.
	printf("Cliente se fue! %d\n", socket);
}

void ClientError(int socketID, int errorCode)
{
	printf("Cliente %d se reporto con error %s!\n", socketID, strerror(errorCode));
}


void StartServer()
{
	CommandInterpreter_Init();
	threadPool = ThreadPool_CreatePool(10, false);

	CommandInterpreter_RegisterCommand("iam", (void*)CommandIAm);

	SocketServer_Start("SAFA","192.168.1.1",8000);
	SocketServer_ActionsListeners actions = INIT_ACTION_LISTENER;

	actions.OnConsoleInputReceived = (void*)processLineInput;
	actions.OnPacketArrived = (void*)onPacketArrived;
	actions.OnClientConnected = (void*)ClientConnected;
	actions.OnClientDisconnect = (void*)ClientDisconnected;
	actions.OnReceiveError = (void*)ClientError;

	SocketServer_ListenForConnection(actions);
	Logger_Log(LOG_INFO, "Server Shutdown.");
}


int main(int argc, char **argv)
{
	Logger_CreateLog("./SAFA.log", "SAFA", true);
	Logger_Log(LOG_INFO, "Proceso SAFA iniciado...");
	StartServer();
	exit_gracefully(0);
}

