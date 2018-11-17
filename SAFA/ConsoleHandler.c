#include "headerFiles/ConsoleHandler.h"

void CommandEjecutar (int argC, char** args, char* callingLine, void* extraData)
{

	if(argC < 2)
	{
		printf("ERROR: Debe indicar la ruta del script a ejecutar\n");
		CommandInterpreter_FreeArguments(args);
		return;
	}

	else if(argC > 2)
	{
		printf("ERROR: Debe ingresar solo la ruta del script a ejecutar como parametro\n");
		CommandInterpreter_FreeArguments(args);
		return;
	}

	//Agrego el script a ejecutar en la cola de Scripts que maneja el PLP, y le aviso
	queue_push(scriptsQueue, args[1]);
	SetPLPTask(PLP_TASK_CREATE_DTB);
	sem_post(&workPLP);

	CommandInterpreter_FreeArguments(args);

}

void CommandStatus (int argC, char** args, char* callingLine, void* extraData)
{

	CommandInterpreter_FreeArguments(args);
}

void CommandFinalizar (int argC, char** args, char* callingLine, void* extraData)
{

	CommandInterpreter_FreeArguments(args);
}

void CommandMetricas (int argC, char** args, char* callingLine, void* extraData)
{

	CommandInterpreter_FreeArguments(args);
}

void CommandQuit (int argC, char** args, char* callingLine, void* extraData)
{
	SocketServer_Stop();
	CommandInterpreter_FreeArguments(args);
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
