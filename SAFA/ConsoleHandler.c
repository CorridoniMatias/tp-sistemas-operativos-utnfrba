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
		printf("ERROR: No puede agregar mas de un parametro al comando!\n");
		printf("       Ingrese simplemente status para ver el estado de las colas\n");
		printf("       Ingrese status idDTB para ver informacion del DTB con dicho ID\n");
		CommandInterpreter_FreeArguments(args);
		return;
	}

	pthread_mutex_lock(&mutexAlgorithm);
	char algorithm[8];								//Aca guardo el algoritmo actual del PCP
	strcpy(algorithm, currentAlgorithm);
	pthread_mutex_unlock(&mutexAlgorithm);

	//Si solo se ingreso status, debo mostrar la informacion principal de las colas
	if(argC == 1)
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
	if(argC == 2)
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

	if(argC != 2)
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

	//No hace falta free del paramID, cuando se haga un pop o destroy de ese elemento, se hara solo
	CommandInterpreter_FreeArguments(args);

}

void CommandMetricas (int argC, char** args, char* callingLine, void* extraData)
{

	if(argC >= 3)
	{
		printf("ERROR: No puede agregar mas de un parametro al comando!\n");
		printf("       Ingrese simplemente metricas para ver metricas generales del sistema\n");
		printf("       Ingrese metricas idDTB para ver las metricas de dicho DTB\n");
		CommandInterpreter_FreeArguments(args);
		return;
	}
	//ToDo: para el punto 1, esperar respuesta de ayudantes (agregar un campo en DTBs?)
	//Para el punto 2, sumar operaciones IO de todos los proceso no en NEW y dividirlos por la cantidad
	//Para el 3, mismo con PCs de los que estan en EXIT; para el 4, sumar IOs dividido la suma de PCs
	//Para el 5.....
	CommandInterpreter_FreeArguments(args);

}

void CommandQuit (int argC, char** args, char* callingLine, void* extraData)
{
	SocketServer_Stop();
	CommandInterpreter_FreeArguments(args);
}
