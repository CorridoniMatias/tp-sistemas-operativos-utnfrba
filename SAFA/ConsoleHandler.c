#include "headerFiles/ConsoleHandler.h"

void ShowQueuesInformation(char* PCPalgorithm)
{

	printf("***COLA DE NEW***\n");
	printf("Cantidad de procesos: %d\n", queue_size(NEWqueue));
	//Para READY, me fijo cual es el algoritmo actual (parametro) asi se que cola o lista contar
	printf("***COLA DE READY***\n");
	if((strcmp(PCPalgorithm, "RR")))
	{
		printf("-Cantidad de procesos: %d\n", queue_size(READYqueue_RR));
	}
	else if((strcmp(PCPalgorithm, "VRR")))
	{
		printf("-Cantidad de procesos: %d\n", list_size(READYqueue_VRR));
	}
	else if((strcmp(PCPalgorithm, "PROPIO")))
	{
		printf("-Cantidad de procesos: %d\n", list_size(READYqueue_Own));
	}
	printf("***COLA DE EXEC***\n");
	printf("-Cantidad de procesos: %d\n", list_size(EXECqueue));
	printf("***COLA DE BLOCKED***\n");
	printf("-Cantidad de procesos: %d\n", list_size(BLOCKEDqueue));
	printf("***COLA DE EXIT***\n");
	printf("-Cantidad de procesos: %d\n", list_size(EXITqueue));
}

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
		printf("       Ejemplo: ejecutar miScript.txt\n");
		CommandInterpreter_FreeArguments(args);
		return;
	}

	//Agrego el script a ejecutar en la cola de Scripts que maneja el PLP; tomo el recaudo de ser el unico alterandola
	pthread_mutex_lock(&mutexScriptsQueue);
	queue_push(scriptsQueue, args[1]);
	pthread_mutex_unlock(&mutexScriptsQueue);

	//Le aviso al PLP que tiene trabajo para hacer, previamente seteando su tarea en crear un DTB con ese script
	SetPLPTask(PLP_TASK_CREATE_DTB);
	sem_post(&workPLP);

	CommandInterpreter_FreeArguments(args);
	return;

}

void CommandStatus (int argC, char** args, char* callingLine, void* extraData)
{

	if(argC >= 3)
	{
		printf("ERROR: No puede agregar mas de un parametro al comando\n");
		printf("       Ingrese simplemente status para ver el estado de las colas\n");
		printf("       Ingrese status idDTB para ver informacion del DTB con dicho ID\n");
		CommandInterpreter_FreeArguments(args);
		return;
	}

	//Si solo se ingreso status, debo mostrar la informacion principal de las colas
	if(argC == 1)
	{
		//Aprovecho esperar a que no se este trabajando sobre READY para mostrar la info de las colas
		pthread_mutex_lock(&mutexREADY);
		printf("\n*****INFORMACION DE COLAS*****\n\n");
		//Tambien mutexeo el algoritmo, por las dudas
		pthread_mutex_lock(&mutexAlgorithm);
		ShowQueuesInformation(currentAlgorithm);
		printf("\nALGORITMO DE PLANIFICACION DEL PCP: %s\n", currentAlgorithm);
		pthread_mutex_unlock(&mutexAlgorithm);
		pthread_mutex_unlock(&mutexREADY);
	}

	if(argC == 2)
	{

	}

	CommandInterpreter_FreeArguments(args);
	return;

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
