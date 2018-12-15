#include "headerFiles/ConsoleHandler.h"

void ShowQueuesInformation(char* PCPalgorithm)
{

	printf("***COLA DE NEW***\n");
	printf("-Cantidad de procesos: %d\n", queue_size(NEWqueue));
	printf("-Procesos:\n");
	list_iterate(NEWqueue->elements, ShowDTBInfo_Shallow);

	//Para READY, me fijo cual es el algoritmo actual (parametro) asi se que cola o lista contar
	printf("***COLA DE READY***\n");
	if((strcmp(PCPalgorithm, "RR")))
	{
		printf("-Cantidad de procesos: %d\n", queue_size(READYqueue_RR));
		printf("-Procesos:\n");
		list_iterate(READYqueue_RR->elements, ShowDTBInfo_Shallow);
	}
	else if((strcmp(PCPalgorithm, "VRR")))
	{
		printf("-Cantidad de procesos: %d\n", list_size(READYqueue_VRR));
		printf("-Procesos:\n");
		list_iterate(READYqueue_VRR, ShowDTBInfo_Shallow);
	}
	else if((strcmp(PCPalgorithm, "PROPIO")))
	{
		printf("-Cantidad de procesos: %d\n", list_size(READYqueue_Own));
		printf("-Procesos:\n");
		list_iterate(READYqueue_Own, ShowDTBInfo_Shallow);
	}

	printf("***COLA DE EXEC***\n");
	printf("-Cantidad de procesos: %d\n", list_size(EXECqueue));
	printf("-Procesos:\n");
	list_iterate(EXECqueue, ShowDTBInfo_Shallow);

	printf("***COLA DE BLOCKED***\n");
	printf("-Cantidad de procesos: %d\n", list_size(BLOCKEDqueue));
	printf("-Procesos:\n");
	list_iterate(BLOCKEDqueue, ShowDTBInfo_Shallow);

	printf("***COLA DE EXIT***\n");
	printf("-Cantidad de procesos: %d\n", list_size(EXITqueue));
	printf("-Procesos:\n");
	list_iterate(EXITqueue, ShowDTBInfo_Shallow);

}

void CommandEjecutar (int argC, char** args, char* callingLine, void* extraData)
{

	if(argC < 1)
	{
		printf("ERROR: Debe indicar la ruta del script a ejecutar\n");
		CommandInterpreter_FreeArguments(args);
		return;
	}

	else if(argC > 1)
	{
		printf("ERROR: Debe ingresar solo la ruta del script a ejecutar como parametro\n");
		printf("       Ejemplo: ejecutar miScript.txt\n");
		CommandInterpreter_FreeArguments(args);
		return;
	}

	//Agrego el script a ejecutar en la cola de Scripts que maneja el PLP; tomo el recaudo de ser el unico alterandola
	pthread_mutex_lock(&mutexScriptsQueue);
	char * path = string_duplicate(args[1]);
	queue_push(scriptsQueue, path);
	Logger_Log(LOG_DEBUG, "SAFA::CONSOLE Introducido el path %s, agregado a la cola de scripts (%d scripts)", path, queue_size(scriptsQueue));
	pthread_mutex_unlock(&mutexScriptsQueue);

	//Le aviso al PLP que tiene trabajo para hacer, previamente seteando su tarea en crear un DTB con ese script
	AddPLPTask(PLP_TASK_CREATE_DTB);

	CommandInterpreter_FreeArguments(args);
	return;

}

void CommandStatus (int argC, char** args, char* callingLine, void* extraData)
{

	if(argC >= 2)
	{
		printf("ERROR: No puede agregar mas de un parametro al comando!\n");
		printf("       Ingrese simplemente status para ver el estado de las colas\n");
		printf("       Ingrese status idDTB para ver informacion del DTB con dicho ID\n");
		CommandInterpreter_FreeArguments(args);
		return;
	}

	pthread_mutex_lock(&mutexSettings);
	char algorithm[8];								//Aca guardo el algoritmo actual del PCP
	strcpy(algorithm, settings->algoritmo);
	pthread_mutex_unlock(&mutexSettings);

	//Si solo se ingreso status, debo mostrar la informacion principal de las colas
	if(argC == 0)
	{
		//Aprovecho esperar a que no se este trabajando sobre READY para mostrar la info de las colas
		pthread_mutex_lock(&mutexREADY);
		printf("\n*****INFORMACION DE COLAS*****\n\n");
		//Tambien mutexeo el algoritmo, por las dudas
		ShowQueuesInformation(algorithm);
		printf("\nALGORITMO DE PLANIFICACION DEL PCP: %s\n", algorithm);
		pthread_mutex_unlock(&mutexREADY);
	}

	//Si se ingreso como parametro un ID de DTB, busco dicho DTB y muestro su informacion
	if(argC == 1)
	{

		//Los elementos de args son cadenas, debo tomarla y convertirla a entero
		uint32_t paramID = (uint32_t) atoi(args[1]);
		DTB* wanted = GetDTBbyID(paramID, algorithm);
		if(wanted)
		{
			ShowDTBInfo_Deep(wanted);
		}
		else
		{
			printf("ERROR: No se hallo ningun DTB con ese ID\n");
		}

	}

	CommandInterpreter_FreeArguments(args);
	return;

}

void CommandFinalizar (int argC, char** args, char* callingLine, void* extraData)
{

	if(argC != 1)
	{
		printf("ERROR: Debe indicar el ID del DTB a finalizar como parametro\n");
		printf("       Ejemplo: finalizar dtbID\n");
		CommandInterpreter_FreeArguments(args);
		return;
	}

	//Los elementos de args son cadenas, debo tomarla y convertirla a entero
	uint32_t* paramID = (uint32_t*) malloc(sizeof(uint32_t));
	*paramID = (uint32_t) atoi(args[1]);
	pthread_mutex_lock(&mutexToBeEnded);
	queue_push(toBeEnded, paramID);
	pthread_mutex_unlock(&mutexToBeEnded);
	pthread_mutex_lock(&mutexCPUs);
	CPU* chosenCPU = list_find(cpus, IsIdle);
	chosenCPU->busy = true;
	Logger_Log(LOG_DEBUG, "SAFA::CPUS->Hallada CPU libre! Socket: %d. Quedan %d CPUs libres", chosenCPU->socket, IdleCPUsAmount());
	pthread_mutex_unlock(&mutexCPUs);
	Logger_Log(LOG_DEBUG, "SAFA::CPUS->Se mando a CPU la orden de liberar la memoria del GDT %d", atoi(args[1]));
	declare_and_init(dtbID, uint32_t, atoi(args[1]))
	bool found = SocketServer_ReserveSocket(chosenCPU->socket);
	SocketCommons_SendData(chosenCPU->socket, MESSAGETYPE_CPU_FREEGDT, dtbID, sizeof(uint32_t));
	if(found){
		OnArrivedData* arrivedData = SocketServer_WakeMeUpWhenDataIsAvailableOn(chosenCPU->socket);
		SocketServer_CleanOnArrivedData(arrivedData);
		FreeCPU(chosenCPU->socket);
	}
	AddPCPTask(PCP_TASK_END_DTB);
	//No hace falta free del paramID, cuando se haga un pop o destroy de ese elemento, se hara solo
	CommandInterpreter_FreeArguments(args);

}

void CommandMetricas (int argC, char** args, char* callingLine, void* extraData)
{

	if(argC >= 2)
	{
		printf("ERROR: No puede agregar mas de un parametro al comando!\n");
		printf("       Ingrese simplemente metricas para ver metricas generales del sistema\n");
		printf("       Ingrese metricas idDTB para ver las metricas de dicho DTB\n");
		CommandInterpreter_FreeArguments(args);
		return;
	}

	//Si se paso como parametro el ID de un DTB, muestro solo la emtrica 1
	else if(argC == 1)
	{

		//Busco el DTB con ese ID y su informacion; si esta (!NULL) muestro la metrica; si no, lo informo
		pthread_mutex_lock(&mutexSettings);
		DTB* wanted = GetDTBbyID(atoi(args[1]), settings->algoritmo);
		pthread_mutex_unlock(&mutexSettings);

		if(wanted)
		{
			printf("***METRICAS DEL DTB DE ID %d***\n", atoi(args[1]));
			printf("Estando en NEW, espero a que se ejecutaran %d sentencias\n", wanted->sentencesWhileAtNEW);
		}
		else
		{
			printf("**NO EXISTE NINGUN DTB CON EL ID ESPECIFICADO**\n");
			return;
		}

	}

	else
	{
		printf("***METRICAS DEL SISTEMA***\n");
		//Uso estos mutex para acceder univocamente a las colas, sin que nadie me joda
		//TODO: Poner un mejor semaforo
		pthread_mutex_lock(&mutexREADY);
		printf("--Cantidad promedio de sentencias ejecutadas que usaron el DAM: %f\n", Metrics_AverageIOSentences());
		printf("--Cantidad promedio de sentencias que llevan a un DTB a EXIT: %f\n", Metrics_AverageExitingSentences());
		printf("--Porcentaje de respuestas ejecutadas que fueron al DAM: %f%%\n", Metrics_IOSentencesPercentage());
		printf("--Tiempo de respuesta promedio del sistema: %f s\n", Metrics_AverageResponseTime());
		pthread_mutex_unlock(&mutexREADY);
	}

	CommandInterpreter_FreeArguments(args);

}

void CommandQuit (int argC, char** args, char* callingLine, void* extraData)
{
	SocketServer_Stop();
	AddPCPTask(P_EXIT);
	AddPLPTask(P_EXIT);
	CommandInterpreter_FreeArguments(args);
}
