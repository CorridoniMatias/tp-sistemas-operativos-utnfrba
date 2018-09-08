#include "incs/ConsoleHandler.h"

void CommandEjecutar (int argC, char** args, char* callingLine, void* extraData)
{

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
