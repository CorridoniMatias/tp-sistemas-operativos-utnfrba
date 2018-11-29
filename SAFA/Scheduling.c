#include "headerFiles/Scheduling.h"

////////////////////////////////////////////////////////////

void InitQueuesAndLists()
{

	scriptsQueue = queue_create();
	NEWqueue = queue_create();
	READYqueue_RR = queue_create();
	READYqueue_VRR = list_create();
	READYqueue_Own = list_create();
	BLOCKEDqueue = list_create();
	EXECqueue = list_create();
	EXITqueue = list_create();
	toBeUnlocked = queue_create();
	toBeBlocked = queue_create();
	toBeEnded = queue_create();

}

void InitSemaphores()
{

	pthread_mutex_init(&mutexPLPtask, NULL);
	pthread_mutex_init(&mutexPCPtask, NULL);
	pthread_mutex_init(&mutexREADY, NULL);
	pthread_mutex_init(&mutexScriptsQueue, NULL);
	pthread_mutex_init(&mutexToBeBlocked, NULL);
	pthread_mutex_init(&mutexToBeUnlocked, NULL);
	pthread_mutex_init(&mutexToBeEnded, NULL);
	sem_init(&workPLP, 0, 0);
	sem_init(&assignmentPending, 0, 0);

}

void CreateDummy()
{

	dummyDTB = (DTB*) malloc(sizeof(DTB));

	dummyDTB->id = 0;									//Valor basura, siempre tendra el del DTB que este cargando
	dummyDTB->initialized = 0;
	dummyDTB->programCounter = 0;
	dummyDTB->status = -1;								//Valor basura, todavia no esta en ninguna cola
	dummyDTB->ioOperations = 0;							//Nunca las tendra; ya lo preseteo asi

	return;

}

void InitSchedulingGlobalVariables()
{

	nextID = 1;										//Arrancan en 1, la 0 es reservada para el Dummy
	inMemoryAmount = 0;								//Al principio, no hay nadie en memoria; se iran cargando

	InitSemaphores();
	InitQueuesAndLists();

	justDummied = (CreatableGDT*) malloc(sizeof(CreatableGDT));
	justDummied->script = malloc(1);				//Medio paragua, para poder hacer reallocs

	toBeAssigned = (AssignmentInfo*) malloc(sizeof(AssignmentInfo));
	toBeAssigned->message = malloc(1);

	CreateDummy();									//Malloceo y creo el DummyDTB

	return;

}

void SetPLPTask(int taskCode)
{

	pthread_mutex_lock(&mutexPLPtask);
	PLPtask = taskCode;
	pthread_mutex_unlock(&mutexPLPtask);
	sem_post(&workPLP);

}

void SetPCPTask(int taskCode)
{

	pthread_mutex_lock(&mutexPCPtask);
	PCPtask = taskCode;
	pthread_mutex_unlock(&mutexPCPtask);

}

////////////////////////////////////////////////////////////

int ShowOptions()
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
		path = malloc(100);			//Fantasma, para no tenerlo sin inicializar

		if(chosenOption == 1)
		{
			printf("Ahora, ingrese el path del Escriptorio a ejecutar\n");
			scanf("%s", path);
			int pathLength = strlen(path) + 1;

			//Realloco el toBeCreated, porque su char* cambio; y tambien realloco este
			justDummied = realloc(justDummied, sizeof(int) + pathLength);
			justDummied->script = realloc(justDummied->script, pathLength);
			strcpy(justDummied->script, path);

			//Actualizo la tarea a realizar del PLP, y activo el semaforo binario
			SetPLPTask(PLP_TASK_CREATE_DTB);
		}

		free(path);

	}

}

////////////////////////////////////////////////////////////

DTB* CreateDTB(char* script)
{

	int dtbSize = (sizeof(int) * 5) + strlen(script) + 1;		//Medio cabeza, mejorar; 5 por los 5 enteros fijos
	DTB* newDTB = (DTB*) malloc(dtbSize);

	newDTB->id = nextID++;
	//Le pongo 1 para saber que no es el Dummy; solo el dummy lo tiene en 0
	newDTB->initialized = 1;
	newDTB->pathEscriptorio = malloc(strlen(script) + 1);
	strcpy(newDTB->pathEscriptorio, script);
	newDTB->programCounter = 0;
	//Le pongo un malloc fantasma para luego poder realocar
	newDTB->openedFiles = malloc(1);
	//De entrada no tiene ningun archivo abierto
	newDTB->openedFilesAmount = 0;
	//Le pongo un valor basura, para su primera ejecucion (por si fuera con VRR)
	newDTB->quantumRemainder = -1;
	newDTB->status = -1;										//Valor basura, todavia no esta en NEW
	newDTB->ioOperations = 0;									//Al arrancar no tiene ninguna

	return newDTB;

}

void AddToNew(DTB* myDTB)
{

	myDTB->status = DTB_STATUS_NEW;
	//Todavia no prendo el flag initialized, falta la operacion Dummy
	queue_push(NEWqueue, myDTB);

}

void AddToReady(DTB* myDTB, char* currentAlgorithm)
{

	myDTB->status = DTB_STATUS_READY;
	//Uso mutexes para no concurrir a la cola READY ni hacer esto durante un cambio de algoritmo
	pthread_mutex_lock(&mutexREADY);
	//Me fijo el algoritmo actual (parametro) y asi se a cual de todas las colas agregarlo
	if((strcmp(currentAlgorithm, "RR")) == 0)
	{
		queue_push(READYqueue_RR, myDTB);
	}
	else if((strcmp(currentAlgorithm, "VRR")) == 0)
	{
		list_add(READYqueue_VRR, myDTB);
	}
	else if((strcmp(currentAlgorithm, "PROPIO")) == 0)
	{
		list_add(READYqueue_Own, myDTB);
		//Ni bien agrego un DTB nuevo, ordeno la lista asi se "inserta ordenado"
		list_sort(READYqueue_Own, DescendantPriority);
	}
	pthread_mutex_unlock(&mutexREADY);

}

void AddToBlocked(DTB* myDTB)
{

	myDTB->status = DTB_STATUS_BLOCKED;
	list_add(BLOCKEDqueue, myDTB);

}

void AddToExec(DTB* myDTB)
{

	myDTB->status = DTB_STATUS_EXEC;
	list_add(EXECqueue, myDTB);

}

void AddToExit(DTB* myDTB)
{

	myDTB->status = DTB_STATUS_EXIT;
	list_add(EXITqueue, myDTB);
	//No me olvido de disminuir la cantidad de procesos en memoria del sistema
	inMemoryAmount--;

}

bool IsDummy(DTB* myDTB)
{

	if(myDTB->initialized == 0)
	{
		return true;
	}
	else
	{
		return false;
	}

}

DTB* GetNextReadyDTB()
{

	DTB* nextToExecute;
	nextToExecute = queue_pop(READYqueue_RR);
	return nextToExecute;

}

////////////////////////////////////////////////////////////

void ShowDTBInfo_Shallow(void* aDTB)
{

	DTB* realDTB = (DTB*) aDTB;
	printf("--------------\n");
	printf(" -ID: %d\n", realDTB->id);
	printf(" -Path: %s\n", realDTB->pathEscriptorio);

}

void ShowPathAndAddress(char* path, void* address)
{

	printf("------\n");
	printf(" -Path: %s\n", path);
	printf(" -Direccion logica: %d\n", *((uint32_t*)address));

}

DTB* GetDTBbyID(uint32_t desiredID, char* algorithm)
{

	//Closure interna, devuelve si un DTB es el ingresado como parametro o no; para buscar
	bool IsDesiredDTB(void* aDtb)
	{
		DTB* castDTB = (DTB*) aDtb;
		if(castDTB->id == desiredID)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	//Busco en cada cola de planificacion; si no lo encuentro en una, voy a la siguiente
	DTB* wanted = list_find(NEWqueue->elements, IsDesiredDTB);

	if(!wanted)
	{
		if((strcmp(algorithm, "RR")))
		{
			wanted = list_find(READYqueue_RR->elements, IsDesiredDTB);
		}
		else if((strcmp(algorithm, "VRR")))
		{
			wanted = list_find(READYqueue_VRR, IsDesiredDTB);
		}
		else if((strcmp(algorithm, "PROPIO")))
		{
			wanted = list_find(READYqueue_Own, IsDesiredDTB);
		}
	}

	if(!wanted)
	{
		wanted = list_find(EXECqueue, IsDesiredDTB);
	}

	if(!wanted)
	{
		wanted = list_find(BLOCKEDqueue, IsDesiredDTB);
	}

	if(!wanted)
	{
		wanted = list_find(EXITqueue, IsDesiredDTB);
	}

	//NOTA: Si no se encontro al DTB en ninguna cola, queda en NULL!
	return wanted;

}

void ShowDTBInfo_Deep(DTB* target)
{

	printf("--ID: %d\n", target->id);
	printf("--Path del script asociado: %s\n", target->pathEscriptorio);
	printf("--Direccion logica del script asociado: %d\n", target->pathLogicalAddress);
	printf("--Program Counter: %d\n", target->programCounter);
	printf("--Flag de inicializacion: %d\n", target->initialized);
	printf("--Estado (cola):");

	switch(target->status)
	{
		case DTB_STATUS_NEW :
			printf("NEW\n");
			break;

		case DTB_STATUS_READY :
			printf("READY\n");
			break;

		case DTB_STATUS_EXEC :
			printf("EXEC\n");
			break;

		case DTB_STATUS_BLOCKED :
			printf("BLOCKED\n");
			break;

		case DTB_STATUS_EXIT :
			printf("EXIT\n");
			break;
	}

	printf("--Quantum restante de ultima ejecucion: %d\n", target->quantumRemainder);
	printf("--Archivos abiertos--\n");
	printf(" -TOTAL: %d\n", target->openedFilesAmount);
	dictionary_iterator(target->openedFiles, ShowPathAndAddress);
	printf("--Cantidad de operaciones IO realizadas: %d\n", target->ioOperations);

}

////////////////////////////////////////////////////////////

void PlanificadorLargoPlazo()
{

	while(1)
	{

		sem_wait(&workPLP);

		//Me agarro el grado de multiprogramacion, antes que alguien lo cambie
		pthread_mutex_lock(&mutexSettings);
		int multiprogrammingDegree = settings->multiprogramacion;
		pthread_mutex_unlock(&mutexSettings);

		if(PLPtask == PLP_TASK_NORMAL_SCHEDULE)
		{

			//Si el grado de multiprogramacion no lo permite, o no hay procesos en NEW, voy a la proxima iteracion del while
			if(inMemoryAmount >= multiprogrammingDegree || queue_is_empty(NEWqueue))
			{
				sleep(3);				//Retardo ficticio, para debuggear; puede servir, para esperar
				continue;
			}

			//Si el Dummy esta en estado BLOCKED (no en la cola, sino esperando a ser asignado), agarro el primero de la cola
			if(dummyDTB->status == DTB_STATUS_BLOCKED)
			{
				//Saco el primero de la cola, y seteo su informacion en el Dummy; borro su referencia?
				DTB* queuesFirst = queue_pop(NEWqueue);
				//Aumento en uno la cantidad de procesos en memoria, asi ya le guardo un lugar
				inMemoryAmount++;
				//Aca seteo su info en el Dummy, y le indico al PCP que lo pase a READY
				SetDummy(queuesFirst->id, queuesFirst->pathEscriptorio);
				free(queuesFirst);
			}
			//Si el Dummy esta en READY o en EXEC, esta ocupado; espero un poco mas y reactivo el semaforo
			else if(dummyDTB -> status == DTB_STATUS_READY || dummyDTB -> status == DTB_STATUS_EXEC)
			{
				sleep(3);
				SetPLPTask(PLP_TASK_NORMAL_SCHEDULE);
			}

		}

		else if(PLPtask == PLP_TASK_CREATE_DTB)
		{
			//Para que no se recorra la cola mientras se estan agregando scripts a ejecutar (por las dudas)
			pthread_mutex_lock(&mutexScriptsQueue);
			//Hasta vaciar la cola de scripts, los voy sacando y guardando en DTBs que agrego a NEW
			while(!queue_is_empty(scriptsQueue))
			{
				DTB* newDTB;
				char* newPath = (char*) queue_pop(scriptsQueue);
				//No le hago free aca, cuando elimine todos lo hare
				newDTB = CreateDTB(newPath);
			    AddToNew(newDTB);
			}
			pthread_mutex_unlock(&mutexScriptsQueue);

			//Vuelvo a poner la tarea del PLP en Planificacion Normal (1)
			SetPLPTask(PLP_TASK_NORMAL_SCHEDULE);
		}

		else if(PLPtask == PLP_TASK_INITIALIZE_DTB)
		{
			//En base a lo que me mando el DAM (protocolo 220), creo el DTB con esa info
			DTB* toBeInitialized = CreateDTB(justDummied->script);
			toBeInitialized->id = justDummied->dtbID;
			toBeInitialized->pathLogicalAddress = justDummied->logicalAddress;
			//Bajo el contador en uno, el CreateDTB lo aumento por defecto
			nextID--;

			//Mutexeo las settings para tomar el algoritmo en ese momento y aislarme de un cambio en simultaneo
			pthread_mutex_lock(&mutexSettings);
			AddToReady(toBeInitialized, settings->algoritmo);
			pthread_mutex_unlock(&mutexSettings);
			//Vuelvo a poner la tarea del PLP en Planificacion Normal (1)
			SetPCPTask(PCP_TASK_NORMAL_SCHEDULE);
			SetPLPTask(PLP_TASK_NORMAL_SCHEDULE);
		}

	}

}

void SetDummy(uint32_t id, char* path)
{

	//Realloco el dummyDTB, porque su char* cambio; cargo el path del DTB a inicializar (no lo hace el PCP)
	int pathLength = strlen(path) + 1;
	int newSize = (sizeof(int) * 6) + pathLength;		//Medio cabeza, mejorar; 6 por los 6 enteros fijos
	dummyDTB->id = id;
	dummyDTB = realloc(dummyDTB, newSize);
	dummyDTB->pathEscriptorio = realloc(dummyDTB->pathEscriptorio, pathLength);
	strcpy(dummyDTB->pathEscriptorio, path);

	//Le aviso al PCP que debe hacer la tarea de LOAD_DUMMY (pasarlo a READY, nada mas)
	SetPCPTask(PCP_TASK_LOAD_DUMMY);

}

////////////////////////////////////////////////////////////

void PlanificadorCortoPlazo()
{

	AlgorithmStatus schedulingRules;				//Para consultar aca y reducir la region critica
	while(1)
	{

		//Excluyentemente, guardo la informacion de la configuracion para planificar; al principio de cada ciclo
		pthread_mutex_lock(&mutexSettings);
		strcpy(schedulingRules.name, settings->algoritmo);
		//Copio el cambio de algoritmo que hubo; si hubo uno, luego de copiar dejo la variable global en UNALTERED,
		//de manera que en la proxima iteracion ya sepa que no hubo cambio (a menos que inotify lo registre)
		schedulingRules.changeType = algorithmChange;
		schedulingRules.delay = settings->retardo;
		schedulingRules.quantum = settings->quantum;
		pthread_mutex_unlock(&mutexSettings);

		//Si hubo un cambio de algoritmo, hago la mudanza de colas segun el tipo del mismo; excluyo las colas READY
		//Luego de la mudanza, dejo en UNALTERED de manera que en la proxima iteracion
		//ya sepa que no hubo cambio (a menos que inotify lo registre)
		if(schedulingRules.changeType != ALGORITHM_CHANGE_UNALTERED)
		{
			pthread_mutex_lock(&mutexREADY);
			MoveQueues(schedulingRules.changeType);
			pthread_mutex_unlock(&mutexREADY);
			algorithmChange = ALGORITHM_CHANGE_UNALTERED;
		}

		if(PCPtask == PCP_TASK_NORMAL_SCHEDULE)
		{

			//Si no hay ningun CPU libre o ningun DTB en READY, espero dos segundos y voy a la proxima iteracion
			if(!ExistsIdleCPU() || NoReadyDTBs(schedulingRules.name))
			{
				sleep(2);
				continue;
			}

			//Agarro el primer CPU libre que haya, lo saco de la lista y lo pongo aca
			CPU* chosenCPU = list_remove_by_condition(cpus, IsIdle);
			toBeAssigned->cpuSocket = chosenCPU->socket;

			SerializedPart* messageToSend;

			//Elegir el DTB adecuado, y obtener el mensaje a enviarle al CPU; ponerlo en EXEC; mutexear las colas READY
			pthread_mutex_lock(&mutexREADY);
			if((strcmp(schedulingRules.name, "RR")) == 0)
			{
				messageToSend = ScheduleRR(schedulingRules.quantum);
			}
			else if((strcmp(schedulingRules.name, "VRR")) == 0)
			{
				messageToSend = ScheduleVRR(schedulingRules.quantum);
			}
			else if((strcmp(schedulingRules.name, "PROPIO")) == 0)
			{
				messageToSend = ScheduleIOBF(schedulingRules.quantum);
			}
			pthread_mutex_unlock(&mutexREADY);

			//No hare un free del SP* messageToSend de cada llamado a esta funcion (si no, joderia al campo del struct)
			//global; pero si, tras cada envio de mensaje, deberia hacer el cleanup del campo del struct (el mensaje ya enviado)
			toBeAssigned->message = messageToSend;

			//Hago signal sobre el semaforo de asignacion pendiente de envio, para que el main sepa que se debe enviar
			sem_post(&assignmentPending);

		}

		//Esto es solo pasar el Dummy a READY para poder planificarlo desde ahi
		else if(PCPtask == PCP_TASK_LOAD_DUMMY)
		{
			dummyDTB->status = DTB_STATUS_READY;
			AddToReady(dummyDTB, schedulingRules.name);
			SetPCPTask(PCP_TASK_NORMAL_SCHEDULE);
		}

		else if(PCPtask == PCP_TASK_FREE_DUMMY)
		{

			//Previo a ello, deberia haber liberado el CPU ni bien este me hablo
			//Saco el Dummy de la lista de EXEC, y lo paso a la de BLOCKED (modifico su estado)
			//El DAM, por su lado, me va a avisar cuando la carga del archivo termine => PLP_TASK_INITIALIZE_DTB
			list_remove_by_condition(EXECqueue, (void*)IsDummy);
			AddToBlocked(dummyDTB);
			SetPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

		//Tras aviso desde CPU de que lo desaloje; se da cuando el GDT requiere un Abrir, Flush, Crear o Borrar
		//Debe recorrer la cola entera de toBeBlocked, por si hubiera varios acumulados que deban ser movidos
		else if(PCPtask == PCP_TASK_BLOCK_DTB)
		{

			//Cuando el CPU me aviso que lo desaloje, deberia haberlo liberado con FreeCPU desde otro modulo
			//Puntero a uint32_t, de ahi voy leyendo el ID del proximo DTB a bloquear (segun la cola)
			BlockableInfo* nextToBlock = (BlockableInfo*) malloc(sizeof(BlockableInfo));
			pthread_mutex_lock(&mutexToBeBlocked);
			while(!queue_is_empty(toBeBlocked))
			{
				nextToBlock = (BlockableInfo*) queue_pop(toBeBlocked);

				//Funcion anidada, para poder comparar cada DTB de EXEC con el ultimo ID (local al while) hallado
				bool IsDTBToBeBlocked(void* aDTB)
				{
					DTB* realDTB = (DTB*) aDTB;
					if(realDTB->id == nextToBlock->id)
					{
						return true;
					}
					else
					{
						return false;
					}
				}

				//Saco de EXEC el DTB que tenia la misma ID que el que acaba de salir de la cola de aBloquear
				DTB* target = list_remove_by_condition(EXECqueue, (void*)IsDTBToBeBlocked);

				//Le actualizo el PC, el quantum sobrante, y los archivos abiertos (por las dudas)
				target->programCounter = nextToBlock->newProgramCounter;
				target->quantumRemainder = nextToBlock->quantumRemainder;
				if(!nextToBlock->dummyComeback)
				{
					UpdateOpenedFiles(target, nextToBlock->openedFilesUpdate, false);
				}
				AddToBlocked(target);
			}
			pthread_mutex_unlock(&mutexToBeBlocked);

			//No me olvido de este free! Ni de avisar al planificador que, tras bloquear todos, vuelva a planificar
			free(nextToBlock);
			SetPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

		//Tras aviso desde DAM
		else if(PCPtask == PCP_TASK_UNLOCK_DTB)
		{

			//Puntero al proximo DTB a desbloquear, segun vaya sacando de la cola
			UnlockableInfo* nextToUnlock = (UnlockableInfo*) malloc(sizeof(UnlockableInfo));
			pthread_mutex_lock(&mutexToBeUnlocked);
			while(!queue_is_empty(toBeUnlocked))
			{

				nextToUnlock = (UnlockableInfo*) queue_pop(toBeUnlocked);

				//Closure anidada, para poder hallar de las colas el DTB con el mismo ID que el recien sacado de la cola
				bool IsDTBToBeUnlocked(void* aDTB)
				{
					DTB* realDTB = (DTB*) aDTB;
					if(realDTB->id == nextToUnlock->id)
					{
						return true;
					}
					else
					{
						return false;
					}
				}

				//Busco el DTB en EXEC (por si es fin de quantum) que tiene dicho ID; si no esta ahi, lo busco en BLOCKED
				DTB* target = list_remove_by_condition(EXECqueue, IsDTBToBeUnlocked);
				if(target)
				{
					//Si y solo si lo encontre en EXEC, le pongo el sobrante en 0 (porque se termino su quantum
					//sin que se lo haya movido a BLOCKED, es la unica chance de registrarlo)
					target->quantumRemainder = 0;
				}
				else
				{
					target = list_remove_by_condition(BLOCKEDqueue, IsDTBToBeUnlocked);
				}

				//Le actualizo el program counter (se debe haber movido) y los archivos abiertos, y lo paso a READY
				//Ojo, solo le altero el PC si el del nextToUnlock no es nulo (un Abrir exitoso lo pone en -1)
				if(nextToUnlock->newProgramCounter != -1)
				{
					target->programCounter = nextToUnlock->newProgramCounter;
				}
				UpdateOpenedFiles(target, nextToUnlock->openedFilesUpdate, nextToUnlock->singleOFaddition);
				AddToReady(target, schedulingRules.name);

			}
			pthread_mutex_unlock(&mutexToBeUnlocked);

			//No me olvido de este free! Ni de avisar al planificador que, tras desbloquear todos, vuelva a planificar
			free(nextToUnlock);
			SetPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

		else if(PCPtask == PCP_TASK_END_DTB)
		{

			//Puntero a un uint32_t, que va a guardar el ID de cada DTB a abortar/finalizar de la cola
			uint32_t* nextToEnd = (uint32_t*) malloc(sizeof(uint32_t));
			pthread_mutex_lock(&mutexToBeEnded);
			while(!queue_is_empty(toBeEnded))
			{

				nextToEnd = (uint32_t*) queue_pop(toBeEnded);

				//Funcion anidada, para poder comparar cada DTB de las colas con el ultimo ID (local al while) hallado
				bool IsDTBToBeEnded(void* aDTB)
				{
					DTB* realDTB = (DTB*) aDTB;
					if(realDTB->id == *nextToEnd)
					{
						return true;
					}
					else
					{
						return false;
					}
				}

				//Busco el DTB en BLOCKED (aviso DAM,o finalizar), en READY (comando finalizar) y en EXEC (aviso CPU)
				DTB* target = list_remove_by_condition(BLOCKEDqueue, IsDTBToBeEnded);
				if(!target)
				{
					pthread_mutex_lock(&mutexREADY);
					if((strcmp(schedulingRules.name, "RR")) == 0)
						target = list_remove_by_condition(READYqueue_RR->elements, IsDTBToBeEnded);
					if((strcmp(schedulingRules.name, "VRR")) == 0)
						target = list_remove_by_condition(READYqueue_VRR, IsDTBToBeEnded);
					if((strcmp(schedulingRules.name, "PROPIO")) == 0)
						target = list_remove_by_condition(READYqueue_Own, IsDTBToBeEnded);
					pthread_mutex_unlock(&mutexREADY);
				}
				if(!target)
				{
					target = list_remove_by_condition(EXECqueue, IsDTBToBeEnded);
				}

				//Lo muevo a la "cola" de EXIT, actualizando su estado
				AddToExit(target);

			}
			pthread_mutex_unlock(&mutexToBeEnded);

			//Libero este puntero, y ademas le aviso al PCP que siga planificando normal
			free(nextToEnd);
			SetPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

		//Aplico retardo de planificacion; divido por mil, ya que son milisegundos
		sleep((settings->retardo) / 1000);

	}

}

bool DescendantPriority(void* dtbOne, void* dtbTwo)
{

	DTB* firstDTB = (DTB*) dtbOne;
	DTB* secondDTB = (DTB*) dtbTwo;
	if(firstDTB->ioOperations >= secondDTB->ioOperations)
	{
		return true;
	}
	else
	{
		return false;
	}

}

void MoveQueues(int changeCode)
{

	switch(changeCode)
	{

		case ALGORITHM_CHANGE_RR_TO_VRR :
			queue_to_list(READYqueue_RR, READYqueue_VRR);
			queue_clean(READYqueue_RR);
			break;

		case ALGORITHM_CHANGE_RR_TO_OWN :
			queue_to_list(READYqueue_RR, READYqueue_Own);
			list_sort(READYqueue_Own, DescendantPriority);
			queue_clean(READYqueue_RR);
			break;

		case ALGORITHM_CHANGE_VRR_TO_RR :
			list_to_queue(READYqueue_VRR, READYqueue_RR);
			list_clean(READYqueue_VRR);
			break;

		case ALGORITHM_CHANGE_VRR_TO_OWN :
			list_copy(READYqueue_VRR, READYqueue_Own);
			list_sort(READYqueue_Own, DescendantPriority);
			list_clean(READYqueue_VRR);
			break;

		//NOTA: Al pasar de IOBF a otro algoritmo, el orden de la cola es el que adquirieron por las prioridades
		//Sin embargo, al agregar DTBs a dicha cola, no se insertara y se ordenara, sino que ira al final
		case ALGORITHM_CHANGE_OWN_TO_RR :
			list_to_queue(READYqueue_Own, READYqueue_RR);
			list_clean(READYqueue_Own);
			break;

		case ALGORITHM_CHANGE_OWN_TO_VRR :
			list_copy(READYqueue_Own, READYqueue_VRR);
			list_clean(READYqueue_VRR);
			break;

	}

	return;

}

bool NoReadyDTBs(char* algorithm)
{

	//Asumo, de entrada, que las colas no estan vacias y si hay DTBs en READY
	bool noneExist = false;

	//Segun el algoritmo pasado por parametro, veo en que cola fijarme; si esta vacia, pongo la respuesta en true
	if((strcmp(algorithm, "RR")) == 0)
	{
		if(queue_is_empty(READYqueue_RR))
		{
			noneExist = true;
		}
	}

	else if((strcmp(algorithm, "VRR")) == 0)
	{
		if(list_is_empty(READYqueue_VRR))
		{
			noneExist = true;
		}
	}

	else if((strcmp(algorithm, "PROPIO")) == 0)
	{
		if(list_is_empty(READYqueue_Own))
		{
			noneExist = true;
		}
	}

	return noneExist;

}

void UpdateOpenedFiles(DTB* toBeUpdated, t_dictionary* currentOFs, bool justAddOne)
{

	//SI Y SOLO SI no estoy haciendo una simple adicion de un archivo abierto (por una operacion Abrir exitosa)
	if(!justAddOne)
	{
		//Limpio el diccionario y borro todos sus elementos, tal vez cerro alguno y no quiero que quede de mas
		dictionary_clean_and_destroy_elements(toBeUpdated->openedFiles, LogicalAddressDestroyer);
	}

	//Haya vaciado el diccionario anterior o no, actualizo el diccionario, es el mismo codigo
	//Closure anidada, para que haga el put en el diccionario del DTB pasado por parametro
	void UpdateSingleFile(char* path, void* address)
	{
		//Uso el putMAESTRO por las dudas, las commons se la comen
		dictionary_putMAESTRO(toBeUpdated->openedFiles, path, address, LogicalAddressDestroyer);
	}

	//Recorro el diccionario parametro (el actualizado) entero, y ejecuto la closure para que sobreescriba cada una
	dictionary_iterator(currentOFs, UpdateSingleFile);

}

void* FlattenPathsAndAddresses(t_dictionary* openFilesTable)
{

	void* result = malloc(1);
	int offset = 0, totalSize = 0;
	int nextSize;

	void CopyPath(char* path, void* address)
	{
		nextSize = strlen(path);							//Obtengo el largo del path
		totalSize += (nextSize + 2 + sizeof(int));			//Sumo a totalSize, y sumo 2 mas por los : y la ,
		result = realloc(result, totalSize);				//Realloco memoria
		memcpy(result + offset, path, nextSize);			//Copio el path y muevo el offset
		offset += nextSize;
		memcpy(result + offset, ":", 1);					//Copio el : (separa path de DL) y muevo el offset
		offset++;
		memcpy(result + offset, address, sizeof(int));		//Copio la DL y muevo el offset
		offset += sizeof(int);
		memcpy(result + offset, ",", 1);					//Copio la , (separa registros) y muevo el offset
		offset++;
	}

	dictionary_iterator(openFilesTable, CopyPath);			//Llamo al closure de arriba para hacerlo con todos los registros

	memcpy(result + offset - 1, ";", 1);					//Pongo el ; al final de la cadena

	return result;											//Queda : "arch1:d1,arch2:d2,...,archN:dN;"

}

SerializedPart* GetMessageForCPU(DTB* chosenDTB)
{

	uint32_t* idToSend = malloc(sizeof(uint32_t));
	*idToSend = chosenDTB->id;
	uint32_t* flagToSend = malloc(sizeof(uint32_t));
	*flagToSend = chosenDTB->initialized;
	uint32_t* pathAddressToSend = malloc(sizeof(uint32_t));
	*pathAddressToSend = chosenDTB->pathLogicalAddress;
	uint32_t* pcToSend = malloc(sizeof(uint32_t));
	*pcToSend = chosenDTB->programCounter;
	uint32_t* quantumToSend = malloc(sizeof(uint32_t));
	*quantumToSend = chosenDTB->quantumRemainder;
	//Cantidad de archivos abiertos
	uint32_t* ofaToSend = malloc(sizeof(uint32_t));
	*ofaToSend = chosenDTB->openedFilesAmount;

	//Estructuras con los datos a serializar y mandar como cadena
	SerializedPart idSP, flagSP, pathSP, pathAddressSP, pcSP, quantumSP, ofaSP, filesSP;
	idSP.size = sizeof(idToSend);
	idSP.data = idToSend;
	flagSP.size = sizeof(flagToSend);
	flagSP.data = flagToSend;
	pathSP.size = strlen(chosenDTB->pathEscriptorio) + 1;
	strcpy(pathSP.data, chosenDTB->pathEscriptorio);
	pathAddressSP.size = sizeof(pathAddressToSend);
	pathAddressSP.data = pathAddressToSend;
	pcSP.size = sizeof(pcToSend);
	pcSP.data = pcToSend;
	quantumSP.size = sizeof(quantumToSend);
	quantumSP.data = quantumToSend;
	ofaSP.size = sizeof(ofaToSend);
	ofaSP.data = ofaToSend;
	filesSP.data = FlattenPathsAndAddresses(chosenDTB->openedFiles);
	filesSP.size = strlen(filesSP.data) + 1;

	//La idea es armar un paquete serializado que va a tener la estructura:
	//|IDdelDTB|Flag|PathEscriptorioAsociado|Dir.LogicaPath|ProgramCounterDelDTB|QuantumAEjecutar|CantArchivosAbiertos|Archivos
	//(cada cual con su respectivo tamanio antes del dato en si)
	//Los archivos se mandan como: "arch1:d1,arch2:d2,...,archN:dN;"
	SerializedPart* message = Serialization_Serialize(8, idSP, flagSP, pathSP, pathAddressSP, pcSP, quantumSP, ofaSP, filesSP);

	//Hago free de todos esos punteros que use para crear la cadena serializada
	free(idToSend);
	free(flagToSend);
	free(pathAddressToSend);
	free(pcToSend);
	free(quantumToSend);
	free(ofaToSend);

	return message;

}

SerializedPart* ScheduleRR(int quantum)
{

	//Round Robin es un FIFO en el cual tengo en cuenta el quantum, no mucho mas que eso
	DTB* chosenDTB = GetNextReadyDTB();
	chosenDTB->quantumRemainder = quantum;

	//Obtengo la cadena a enviarle al CPU asignado; detalle de la misma dentro de la funcion; el free es en otro lado
	SerializedPart* packet = GetMessageForCPU(chosenDTB);

	AddToExec(chosenDTB);
	return packet;

}

SerializedPart* ScheduleVRR(int maxQuantum)
{

	//Closure, interna a esta funcion para poder comparar contra el parametro; me dice si un DTB esta en la cola de prioridad
	bool AtPriorityQueue(void* aDTB)
	{
		DTB* realDTB = (DTB*) aDTB;
		if(realDTB->quantumRemainder != maxQuantum)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	//Tomo el primer DTB de la "cola de prioridad" (el primero que tenga quantum sobrante y no natural)
	DTB* chosenDTB = list_remove_by_condition(READYqueue_VRR, AtPriorityQueue);

	//Si no hubiese ninguno en dicha "cola de prioridad", agarro el primero de la cola (que es una lista, hago get(0))
	if(!chosenDTB)
	{
		chosenDTB = list_remove(READYqueue_VRR, 0);
	}

	//Si le quedara 0 de quantum (se quedo sin) o tuviera mas del maximo (por haber sido
	//planificado con otro algoritmo antes), le actualizo el maximo quantum a ejecutar
	if((chosenDTB->quantumRemainder == 0) || (chosenDTB->quantumRemainder > maxQuantum))
	{
		chosenDTB->quantumRemainder = maxQuantum;
	}

	//Analogo al Round Robin comun, obtengo el mensaje a enviarle al CPU y agrego dicho DTB a EXEC
	SerializedPart* packet = GetMessageForCPU(chosenDTB);
	AddToExec(chosenDTB);
	return packet;

}

SerializedPart* ScheduleIOBF(int quantum)
{

	//Por las dudas, ordeno la "cola" (lista) de nuevo, por si no hubiera habido un cambio de algoritmo pero si de orden
	list_sort(READYqueue_Own, DescendantPriority);

	//Agarro el primero de la lista, seria como hacer el pop de la cola; le fijo el quantum, el algoritmo lo impone
	DTB* chosenDTB = list_remove(READYqueue_Own, 0);
	chosenDTB->quantumRemainder = quantum;

	//Igual que en los otros, obtengo el mensaje a enviar y lo guardo en la estructura global; muevo DTB a EXEC
	SerializedPart* packet = GetMessageForCPU(chosenDTB);
	AddToExec(chosenDTB);
	return packet;

}

////////////////////////////////////////////////////////////

void DeleteSemaphores()
{

	pthread_mutex_destroy(&mutexPLPtask);
	pthread_mutex_destroy(&mutexPCPtask);
	pthread_mutex_destroy(&mutexREADY);
	pthread_mutex_destroy(&mutexScriptsQueue);
	pthread_mutex_destroy(&mutexToBeBlocked);
	pthread_mutex_destroy(&mutexToBeUnlocked);
	pthread_mutex_destroy(&mutexToBeEnded);
	sem_destroy(&workPLP);
	sem_destroy(&assignmentPending);

}

void ScriptDestroyer(void* script)
{

	free(script);

}

void LogicalAddressDestroyer(void* addressPtr)
{

	free(addressPtr);

}

void BlockableInfoDestroyer(void* BI)
{

	BlockableInfo* castBI = (BlockableInfo*) BI;
	dictionary_destroy_and_destroy_elements(castBI->openedFilesUpdate, LogicalAddressDestroyer);
	//Al igual que en DTBDestroyer, el free principal lo hago sobre el parametro
	free(BI);

}

void UnlockableInfoDestroyer(void* UI)
{

	UnlockableInfo* castUI = (UnlockableInfo*) UI;
	dictionary_destroy_and_destroy_elements(castUI->openedFilesUpdate, LogicalAddressDestroyer);
	//Al igual que en DTBDestroyer, el free principal lo hago sobre el parametro
	free(UI);

}

void EndableDestroyer(void* endableID)
{

	free(endableID);

}

void DTBDestroyer(void* aDTB)
{

	DTB* castDTB = (DTB*) aDTB;
	free(castDTB->pathEscriptorio);
	//El free del DTB lo hago sobre el parametro, no el casteado, asi me libera la memoria ocupada por ese
	//Al salir de la funcion, el casteado (necesario para liberar el path) liberara la memoria suya solo, ya que es local
	free(aDTB);

}

void DeleteQueuesAndLists()
{

	queue_destroy_and_destroy_elements(scriptsQueue, ScriptDestroyer);
	queue_destroy_and_destroy_elements(NEWqueue, DTBDestroyer);
	queue_destroy_and_destroy_elements(READYqueue_RR, DTBDestroyer);
	list_destroy_and_destroy_elements(BLOCKEDqueue, DTBDestroyer);
	list_destroy_and_destroy_elements(EXECqueue, DTBDestroyer);
	list_destroy_and_destroy_elements(EXITqueue, DTBDestroyer);
	queue_destroy_and_destroy_elements(toBeUnlocked, UnlockableInfoDestroyer);
	queue_destroy_and_destroy_elements(toBeBlocked, BlockableInfoDestroyer);
	queue_destroy_and_destroy_elements(toBeEnded, EndableDestroyer);

}

void DeleteSchedulingGlobalVariables()
{

	free(justDummied->script);
	free(justDummied);
	//El free del Dummy no se hace aca, sino al hacer el delete de Colas y Listas (estara en alguna)

	//Libero la memoria de la estructura para guardar asignaciones pendientes y su mensaje
	Serialization_CleanupSerializedPacket(toBeAssigned->message);
	free(toBeAssigned);

	DeleteSemaphores();
	DeleteQueuesAndLists();

}
