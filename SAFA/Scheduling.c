#include "incs/Scheduling.h"

////////////////////////////////////////////////////////////

void InitQueuesAndLists()
{

	NEWqueue = queue_create();
	READYqueue = queue_create();
	BLOCKEDqueue = list_create();
	EXECqueue = list_create();
	EXITqueue = list_create();

}

void InitGlobalVariables()
{

	nextID = 1;										//Arrancan en 1, la 0 es reservada para el Dummy

	pthread_mutex_init(mutexPLPtask, NULL);
	pthread_mutex_init(mutexPCPtask, NULL);
	sem_init(&workPLP, 0, 0);

	InitQueuesAndLists();

	toBeCreated = (CreatableGDT*) malloc(sizeof(CreatableGDT));
toBeCreated->script = malloc(1);									//Medio paragua, para poder hacer reallocs

}

void SetPLPTask(int taskCode)
{
	pthread_mutex_lock(&mutexPLPtask);
	mutexPLPtask = taskCode;
	pthread_mutex_unlock(&mutexPLPtask);
	sem_post(&workPLP);
}

////////////////////////////////////////////////////////////

int showOptions()
{

	int chosen;
	do
	{
		printf("------Consola del Gestor de Programas G.DT!!------\n");
		printf("Seleccione la operacion a realizar\n");
		printf("1. Ejecutar\n");
		printf("2. Status\n");
		printf("3. Finalizar\n");
		printf("4. Metricas\n");
		scanf("%d", &chosen);
	} while(chosen != 1);		//Por ahora, solo probamos con ejecutar
	return chosen;

}

void GestorDeProgramas()
{

	int chosenOption;
	while(1)
	{

		chosenOption = ShowOptions();
		char* path;

		if(chosenOption == 1)
		{
			printf("Ahora, ingrese el path del Escriptorio a ejecutar\n");
			scanf("%s", path);
			int pathLength = strlen(path) + 1;

			//Realloco el toBeCreated, porque su char* cambio; y tambien realloco este
			toBeCreated = realloc(toBeCreated, sizeof(int) + pathLength);
			toBeCreated->script = realloc(toBeCreated->script, pathLength);
			strcpy(toBeCreated->script, path);

			//Actualizo la tarea a realizar del PLP, y activo el semaforo binario
			SetPLPTask(PLP_TASK_CREATE_DTB);
		}

	}

}

////////////////////////////////////////////////////////////

DTB* CreateDTB(char* script)
{

	int dtbSize = (sizeof(int) * 5) + strlen(script) + 1;		//Medio cabeza, mejorar; 5 por los 5 enteros fijos
	DTB* newDTB = (DTB*) malloc(dtbSize);

	newDTB->id = nextID++;
	newDTB->initialized = 0;
	newDTB->pathEscriptorio = malloc(strlen(script) + 1);
	strcpy(newDTB->pathEscriptorio, script);
	newDTB->programCounter = 0;
	newDTB->status = -1;										//Valor basura, todavia no esta en NEW

	return newDTB;

}

void AddToNew(DTB* myDTB)
{
	myDTB->status = DTB_STATUS_NEW;
	//Todavia no prendo el flag initialized, falta la operacion Dummy
	list_add(NEWqueue, myDTB);
}

bool IsDTBtoBeInitialized(DTB* myDTB)
{

	if(myDTB->id == toBeCreated->dtbID)
	{
		return true;
	}
	else
	{
		return false;
	}

}

void PlanificadorLargoPlazo(void* gradoMultiprogramacion)
{

	while(1)
	{

		sem_wait(&workPLP);

		if(PLPtask == PLP_TASK_NORMAL_SCHEDULE)
		{
			//Verificar que el grado de multiprogramacion no supere la cantidad actual
			//Verificar que haya procesos en NEW
			//Verificar que estos esten ya inicializados (pasaron por el Dummy)
		}

		else if(PLPtask == PLP_TASK_CREATE_DTB)
		{
			DTB* newDTB;
			newDTB = CreateDTB(toBeCreated->script);
			AddToNew(newDTB);
			//Vuelvo a poner la tarea del PLP en Planificacion Normal (1)
			SetPLPTask(PLP_TASK_NORMAL_SCHEDULE);
		}

		else if(PLPtask == PLP_TASK_INITIALIZE_DTB)
		{
			//Busco el DTB que tiene el mismo ID que indicaba el Dummy; le pongo el flag en 1
			//Para ello, debo sacarlo de la lista, modificarlo, y ponerlo al final de nuevo (es una cola)
			DTB* toBeInitialized = list_remove_by_condition(NEWqueue, IsDTBtoBeInitialized);
			toBeInitialized->initialized = 1;
			AddToNew(toBeInitialized);
			//Vuelvo a poner la tarea del PLP en Planificacion Normal (1)
			SetPLPTask(PLP_TASK_NORMAL_SCHEDULE);
		}

	}

}

////////////////////////////////////////////////////////////

void PlanificadorCortoPlazo(void* algoritmo)
{

	while(1)
	{

		if(PCPtask == PCP_TASK_NORMAL_SCHEDULE)
		{

		}

		else if(PCPtask == PCP_TASK_LOAD_DUMMY)
		{

		}

		else if(PCPtask == PCP_TASK_BLOCK_DUMMY)
		{

		}

		else if(PCPtask == PCP_TASK_BLOCK_DTB)
		{

		}

		else if(PCPtask == PCP_TASK_UNLOCK_DTB)
		{

		}

	}

}
