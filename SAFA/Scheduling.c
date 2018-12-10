#include "headerFiles/Scheduling.h"

////////////////////////////////////////////////////////////

void InitQueuesAndLists()
{

	PLPtasksQueue = queue_create();
	PCPtasksQueue = queue_create();
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
	beingDummied = list_create();
	dummiedQueue = queue_create();

}

void InitSemaphores()
{

	pthread_mutex_init(&mutexPLPtasksQueue, NULL);
	pthread_mutex_init(&mutexPCPtasksQueue, NULL);
	pthread_mutex_init(&mutexREADY, NULL);
	pthread_mutex_init(&mutexScriptsQueue, NULL);
	pthread_mutex_init(&mutexToBeBlocked, NULL);
	pthread_mutex_init(&mutexToBeUnlocked, NULL);
	pthread_mutex_init(&mutexToBeEnded, NULL);
	pthread_mutex_init(&mutexNEW, NULL);
	pthread_mutex_init(&mutexBeingDummied, NULL);
	pthread_mutex_init(&mutexDummiedQueue, NULL);
	sem_init(&workPLP, 0, 0);
	sem_init(&workPCP, 0, 0);

}

void CreateDummy()
{

	dummyDTB = (DTB*) malloc(sizeof(DTB));

	dummyDTB->id = 0;									//Valor basura, siempre tendra el del DTB que este cargando
	dummyDTB->initialized = 0;
	dummyDTB->programCounter = 0;
	dummyDTB->status = DTB_STATUS_BLOCKED;				//Arranca en BLOCKED, asi ya esta disponible
	dummyDTB->ioOperations = 0;							//Nunca las tendra; ya lo preseteo asi
	dummyDTB->pathEscriptorio = malloc(1);				//Para poder hacerle el realloc
	dummyDTB->pathLogicalAddress =0;
	dummyDTB->openedFiles = dictionary_create();
	dummyDTB->openedFilesAmount = 0 ;
	dummyDTB->firstResponseTime = 0;
	return;

}

void InitSchedulingGlobalVariables()
{

	nextID = 1;										//Arrancan en 1, la 0 es reservada para el Dummy
	inMemoryAmount = 0;								//Al principio, no hay nadie en memoria; se iran cargando

	InitSemaphores();
	InitQueuesAndLists();

	CreateDummy();									//Malloceo y creo el DummyDTB

	return;

}

void AddPLPTask(int taskCode)
{

	pthread_mutex_lock(&mutexPLPtasksQueue);
	int* newTask = (int*) malloc(sizeof(int));
	*newTask = taskCode;
	queue_push(PLPtasksQueue, newTask);
	pthread_mutex_unlock(&mutexPLPtasksQueue);
	sem_post(&workPLP);
	int semaforo;
	sem_getvalue(&workPCP,&semaforo);
	printf("\n\n\n\n\nel semaforo del plp vale %d\n\n\n\n\n\n",semaforo);

}

void AddPCPTask(int taskCode)
{

	pthread_mutex_lock(&mutexPCPtasksQueue);
	int* newTask = (int*) malloc(sizeof(int));
	*newTask = taskCode;
	queue_push(PCPtasksQueue, newTask);
	pthread_mutex_unlock(&mutexPCPtasksQueue);
	sem_post(&workPCP);

}

////////////////////////////////////////////////////////////

DTB* CreateDTB(char* script)
{
	DTB* newDTB = (DTB*) malloc(sizeof(DTB));

	newDTB->id = nextID++;
	//Le pongo 1 para saber que no es el Dummy; solo el dummy lo tiene en 0
	newDTB->initialized = 1;
	newDTB->pathEscriptorio = malloc(strlen(script) + 1);
	newDTB->pathLogicalAddress = 0;
	strcpy(newDTB->pathEscriptorio, script);
	newDTB->programCounter = 0;
	//Le pongo un malloc fantasma para luego poder realocar
	newDTB->openedFiles = dictionary_create();
	//De entrada no tiene ningun archivo abierto
	newDTB->openedFilesAmount = 0;
	//Le pongo un valor basura, para su primera ejecucion (por si fuera con VRR)
	newDTB->quantumRemainder = -1;
	newDTB->status = -1;										//Valor basura, todavia no esta en NEW
	newDTB->ioOperations = 0;									//Al arrancar no tiene ninguna
	newDTB->sentencesWhileAtNEW = 0;							//Todavia no esta en NEW ni pasaron sentencias con el ahi
	newDTB->firstResponseTime = 0;								//Valor por defecto, para verificar si hace falta

	return newDTB;

}

void AddToNew(DTB* myDTB)
{

	//Le pongo el status en NEW y ya registro su instante de ingreso al sistema
	myDTB->status = DTB_STATUS_NEW;
	time(&(myDTB->spawnTime));
	//Todavia no prendo el flag initialized, falta la operacion Dummy
	pthread_mutex_lock(&mutexNEW);
	queue_push(NEWqueue, myDTB);
	pthread_mutex_unlock(&mutexNEW);
	Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Se agrego a NEW el DTB con el path \"%s\", ahora hay %d DTBs ahi", myDTB->pathEscriptorio, queue_size(NEWqueue));

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
		Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Agregado el DTB de ID %d a READY. Hay %d DTBs ahi", myDTB->id, queue_size(READYqueue_RR));
	}
	else if((strcmp(currentAlgorithm, "VRR")) == 0)
	{
		list_add(READYqueue_VRR, myDTB);
		Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Agregado el DTB de ID %d a READY. Hay %d DTBs ahi", myDTB->id, list_size(READYqueue_VRR));
	}
	else if((strcmp(currentAlgorithm, "PROPIO")) == 0)
	{
		list_add(READYqueue_Own, myDTB);
		//Ni bien agrego un DTB nuevo, ordeno la lista asi se "inserta ordenado"
		list_sort(READYqueue_Own, DescendantPriority);
		Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Agregado el DTB de ID %d a READY. Hay %d DTBs ahi", myDTB->id, list_size(READYqueue_VRR));
	}
	pthread_mutex_unlock(&mutexREADY);

}

void AddToBlocked(DTB* myDTB)
{

	myDTB->status = DTB_STATUS_BLOCKED;
	list_add(BLOCKEDqueue, myDTB);
	Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Agregado el DTB de ID %d a BLOCKED. Hay %d DTBs ahi", myDTB->id, list_size(BLOCKEDqueue));

}

void AddToExec(DTB* myDTB)
{

	myDTB->status = DTB_STATUS_EXEC;
	list_add(EXECqueue, myDTB);
	Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Agregado el DTB de ID %d a EXEC. Hay %d DTBs ahi", myDTB->id, list_size(EXECqueue));

}

void AddToExit(DTB* myDTB)
{

	myDTB->status = DTB_STATUS_EXIT;
	list_add(EXITqueue, myDTB);
	Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Agregado el DTB de ID %d a EXIT. Hay %d DTBs ahi", myDTB->id, list_size(EXITqueue));
	//No me olvido de disminuir la cantidad de procesos en memoria del sistema
	inMemoryAmount--;
	Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Ahora hay %d procesos en memoria", inMemoryAmount);

}

bool IsDummy(void* myDTB)
{

	DTB* realDTB = (DTB*) myDTB;
	if(realDTB->initialized == 0)
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
	pthread_mutex_lock(&mutexNEW);
	DTB* wanted = list_find(NEWqueue->elements, IsDesiredDTB);
	pthread_mutex_unlock(&mutexNEW);

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

void SetDummy(uint32_t id, char* path)
{

	//Realloco el dummyDTB, porque su char* cambio; cargo el path del DTB a inicializar (no lo hace el PCP)
	int pathLength = strlen(path) + 1;
	dummyDTB->id = id;
	dummyDTB->pathEscriptorio = realloc(dummyDTB->pathEscriptorio, pathLength);
	strcpy(dummyDTB->pathEscriptorio, path);
	Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Cargado el Dummy con el path %s", dummyDTB->pathEscriptorio);
	//Le aviso al PCP que debe hacer la tarea de LOAD_DUMMY (pasarlo a READY, nada mas)
	AddPCPTask(PCP_TASK_LOAD_DUMMY);

}

void PlanificadorLargoPlazo()
{

	Logger_Log(LOG_INFO, "SAFA::PLANIF->Planificador de Largo Plazo ya operativo");
	int currentTask;

	while(1)
	{

		//Espero a que le indiquen al PLP que se ejecute
		sem_wait(&workPLP);
		Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Tarea del PLP en ejecucion");

		//En cada iteracion, saco la primer tarea de la cola, y guardo su valor para luego compararla y saber que modulo realizar
		pthread_mutex_lock(&mutexPLPtasksQueue);
		currentTask = *((int*) queue_pop(PLPtasksQueue));
		pthread_mutex_unlock(&mutexPLPtasksQueue);

		//Me agarro el grado de multiprogramacion, antes que alguien lo cambie
		pthread_mutex_lock(&mutexSettings);
		int multiprogrammingDegree = settings->multiprogramacion;
		pthread_mutex_unlock(&mutexSettings);

		if(currentTask == PLP_TASK_NORMAL_SCHEDULE)
		{

			//Si el grado de multiprogramacion no lo permite, o no hay procesos en NEW, voy a la proxima iteracion del while
			pthread_mutex_lock(&mutexNEW);
			if(inMemoryAmount >= multiprogrammingDegree || queue_is_empty(NEWqueue))
			{
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->El PLP no puede planificar");
				pthread_mutex_unlock(&mutexNEW);
				sleep(3);							//Retardo ficticio, para debuggear; puede servir, para esperar
				continue;
			}
			else
			{
				pthread_mutex_unlock(&mutexNEW);
			}

			//Si el Dummy esta en estado BLOCKED (no en la cola, sino esperando a ser asignado), agarro el primero de la cola
			if(dummyDTB->status == DTB_STATUS_BLOCKED)
			{
				// Se pone el dummy en ready aca para que no se sobreescriba el path y se pierda un proceso a ejecutar.
				dummyDTB->status = DTB_STATUS_READY;
				//Removemos el dummy de la cola BLOCKED.
				list_remove_by_condition(BLOCKEDqueue, IsDummy);
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->El Dummy esta desocupado, se intentara cargarlo");
				//Saco el primero de la cola, y seteo su informacion en el Dummy; borro su referencia?
				pthread_mutex_lock(&mutexNEW);
				DTB* queuesFirst = queue_pop(NEWqueue);
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->El primer DTB de NEW tiene el path %s. Saco y quedan %d en la cola", queuesFirst->pathEscriptorio, list_size(NEWqueue->elements));
				pthread_mutex_unlock(&mutexNEW);
				//Aumento en uno la cantidad de procesos en memoria, asi ya le guardo un lugar
				inMemoryAmount++;
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Hay ahora %d procesos en memoria", inMemoryAmount);
				//Aca seteo su info en el Dummy, y le indico al PCP que lo pase a READY
				SetDummy(queuesFirst->id, queuesFirst->pathEscriptorio);
				//Agrego el DTB que saque de la cola a la lista de DTBs siendo inicializados, para conservar su info
				queuesFirst->status = DTB_STATUS_DUMMYING;
				pthread_mutex_lock(&mutexBeingDummied);
				list_add(beingDummied, queuesFirst);
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Hay en total %d procesos siendo dummyados", list_size(beingDummied));
				pthread_mutex_unlock(&mutexBeingDummied);
			}
			//Si el Dummy esta en READY o en EXEC, esta ocupado; espero un poco mas y reactivo el semaforo
			else if(dummyDTB -> status == DTB_STATUS_READY || dummyDTB -> status == DTB_STATUS_EXEC)
			{
				sleep(3);
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Dummy ocupado, se intentara planificar con normalidad");
				AddPLPTask(PLP_TASK_NORMAL_SCHEDULE);
			}

		}

		else if(currentTask == PLP_TASK_CREATE_DTB)
		{
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Se registro un pedido de crear un DTB");
			//Para que no se recorra la cola mientras se estan agregando scripts a ejecutar (por las dudas)
			pthread_mutex_lock(&mutexScriptsQueue);
			//Hasta vaciar la cola de scripts, los voy sacando y guardando en DTBs que agrego a NEW
			while(!queue_is_empty(scriptsQueue))
			{
				DTB* newDTB;
				char* newPath = (char*) queue_pop(scriptsQueue);
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Se saco el path \"%s\" de la cola de scripts a cargar", newPath);
				//No le hago free aca, cuando elimine todos lo hare
				newDTB = CreateDTB(newPath);
			    AddToNew(newDTB);
			}
			pthread_mutex_unlock(&mutexScriptsQueue);

			//Vuelvo a poner la tarea del PLP en Planificacion Normal (1)
			AddPLPTask(PLP_TASK_NORMAL_SCHEDULE);
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Scripts pendientes ya creados, vuelvo a planificacion normal");
		}

		else if(currentTask == PLP_TASK_INITIALIZE_DTB)
		{

			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->PLP Bajo la orden de inicializar DTBs cuya carga Dummy ya termino");

			//Uso este mutex para acceder de a uno a la cola de DTBs ya dummiados (desbloqueo luego del while)
			pthread_mutex_lock(&mutexDummiedQueue);

			//Hasta vaciar la cola de DTBs ya cargados con el dummy, hago este algoritmo
			while(!queue_is_empty(dummiedQueue))
			{
				CreatableGDT* justDummied = queue_pop(dummiedQueue);

				//Closure para poder encontrar el DTB con los datos y el mismo ID que el que volvio del Dummy y fue sacado de la cola
				bool IsDTBBeingDummied(void* aDTB)
				{
					DTB* castDTB = (DTB*) aDTB;
					if(castDTB->id == justDummied->dtbID)
					{
						return true;
					}
					else
					{
						return false;
					}
				}

				//En base a lo que me mando el DAM (protocolo 220) y se encolo en dummiedQueue, busco el DTB expectante con el ID del Dummy
				pthread_mutex_lock(&mutexBeingDummied);
				DTB* toBeInitialized = list_remove_by_condition(beingDummied, IsDTBBeingDummied);
				pthread_mutex_unlock(&mutexBeingDummied);
				//Le guardo la direccion logica que proporciono la operacion Dummy, le pongo el flag en 1 y el PC en 0
				toBeInitialized->pathLogicalAddress = justDummied->logicalAddress;
				toBeInitialized->initialized = 1;
				toBeInitialized->programCounter = 0;

				Logger_Log(LOG_DEBUG, "Se quiere inicializar el DTB de id %d y path %s, con la dir %d", toBeInitialized->id, toBeInitialized->pathEscriptorio, toBeInitialized->pathLogicalAddress);

				//Mutexeo las settings para tomar el algoritmo en ese momento y aislarme de un cambio en simultaneo
				pthread_mutex_lock(&mutexSettings);
				AddToReady(toBeInitialized, settings->algoritmo);
				pthread_mutex_unlock(&mutexSettings);

			}

			//Ya vacie la cola de DTBs dummiados, la desbloqueo
			pthread_mutex_unlock(&mutexDummiedQueue);

			//Vuelvo a poner la tarea del PLP en Planificacion Normal (1)
			AddPCPTask(PCP_TASK_NORMAL_SCHEDULE);
			AddPLPTask(PLP_TASK_NORMAL_SCHEDULE);

		}

	}

}

////////////////////////////////////////////////////////////

void AggregateSentencesWhileAtNEW(int amount)
{

	//Closure interna, anidada, le suma las sentencias esperadas al DTB en base al parametro pasado
	void UpdateSingleDTB(void* aDTB)
	{
		DTB* realDTB = (DTB*) aDTB;
		realDTB->sentencesWhileAtNEW += amount;
	}

	//Sin joder a nadie mas que quiera usar la cola, actualizo las sentencias leidas de todos los DTBs de NEW
	pthread_mutex_lock(&mutexNEW);
	list_iterate(NEWqueue->elements, UpdateSingleDTB);
	pthread_mutex_unlock(&mutexNEW);

}

void PlanificadorCortoPlazo()
{
	AlgorithmStatus schedulingRules;				//Para consultar aca y reducir la region critica
	int currentTask;
	while(1)
	{

		//Espero a que le indiquen al PCP que se ejecute
		sem_wait(&workPCP);

		//En cada iteracion, saco la primer tarea de la cola, y guardo su valor para luego compararla y saber que modulo realizar
		pthread_mutex_lock(&mutexPCPtasksQueue);
		currentTask = *((int*) queue_pop(PCPtasksQueue));
		pthread_mutex_unlock(&mutexPCPtasksQueue);

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
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Hubo un cambio de algoritmo, se movieron las colas");
			algorithmChange = ALGORITHM_CHANGE_UNALTERED;
		}

		if(currentTask == PCP_TASK_NORMAL_SCHEDULE)
		{

			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->PCP bajo planificacion normal");
			//Si no hay ningun CPU libre o ningun DTB en READY, espero dos segundos y voy a la proxima iteracion
			if(!ExistsIdleCPU() || NoReadyDTBs(schedulingRules.name))
			{
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Imposible planificar PCP, no hay nada en READY, o no hay CPUs libres");
				sleep(2);
				continue;
			}

			//Agarro el primer CPU libre que haya y lo marco como ocupado (sin sacarlo de la lista)

			pthread_mutex_lock(&mutexCPUs);
			Logger_Log(LOG_DEBUG, "SAFA::CPUS->Intentando planificar, hay %d CPUs libres", IdleCPUsAmount());
			CPU* chosenCPU = list_find(cpus,IsIdle);
			chosenCPU->busy = true;
			Logger_Log(LOG_DEBUG, "SAFA::CPUS->Hallada CPU libre! Socket: %d. Quedan %d CPUs libres", chosenCPU->socket, IdleCPUsAmount());
			pthread_mutex_unlock(&mutexCPUs);

			SerializedPart* messageToSend;

			//Elegir el DTB adecuado, y obtener el mensaje a enviarle al CPU; ponerlo en EXEC; mutexear las colas READY
			pthread_mutex_lock(&mutexREADY);
			if((strcmp(schedulingRules.name, "RR")) == 0)
			{
				printf("\n\nse esta usando RR\n\n");
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
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Armado el mensaje a enviarle al CPU");
			//Envio, a traves del socket del CPU elegido, el mensaje acerca del DTB y su tamanio

			printf("\n\n\nsize=%d\n\n\n",messageToSend->size);


			SocketCommons_SendData(chosenCPU->socket, MESSAGETYPE_SAFA_CPU_EXECUTE, messageToSend->data, messageToSend->size);

			pthread_mutex_lock(&mutexCPUs);
			Logger_Log(LOG_INFO,"SAFA::CPUS->CPU elegido marcado como ocupado. Vuelven a haber %d CPUs en total", CPUsCount());
			pthread_mutex_unlock(&mutexCPUs);

			//Libero la memoria de esto, ya lo mande asi que no pierdo nada
			Serialization_CleanupSerializedPacket(messageToSend);

		}

		//Esto es solo pasar el Dummy a READY para poder planificarlo desde ahi
		else if(currentTask == PCP_TASK_LOAD_DUMMY)
		{

			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->PCP intentara cargar el Dummy con los datos del DTB correspondiente");
			dummyDTB->status = DTB_STATUS_READY;
			AddToReady(dummyDTB, schedulingRules.name);
			AddPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

		else if(currentTask == PCP_TASK_FREE_DUMMY)
		{

			//Previo a ello, deberia haber liberado el CPU ni bien este me hablo
			//Saco el Dummy de la lista de EXEC, y lo paso a la de BLOCKED (modifico su estado)
			//El DAM, por su lado, me va a avisar cuando la carga del archivo termine => PLP_TASK_INITIALIZE_DTB
			list_remove_by_condition(EXECqueue, IsDummy);
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Dummy extraido de la cola de EXEC, lo dejare libre en la cola de BLOCKED");
			dummyDTB->id = 0;
			AddToBlocked(dummyDTB);
			AddPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

		//Tras aviso desde CPU de que lo desaloje; se da cuando el GDT requiere un Abrir, Flush, Crear o Borrar
		//Debe recorrer la cola entera de toBeBlocked, por si hubiera varios acumulados que deban ser movidos
		else if(currentTask == PCP_TASK_BLOCK_DTB)
		{

			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->PCP bajo la orden de pasar DTBs a BLOCKED");
			//Cuando el CPU me aviso que lo desaloje, deberia haberlo liberado con FreeCPU desde otro modulo
			//Puntero a uint32_t, de ahi voy leyendo el ID del proximo DTB a bloquear (segun la cola)
			BlockableInfo* nextToBlock;
			pthread_mutex_lock(&mutexToBeBlocked);
			while(!queue_is_empty(toBeBlocked))
			{
				nextToBlock = (BlockableInfo*) queue_pop(toBeBlocked);
				printf("\n\nbabyComeback=%d\n\n",nextToBlock->dummyComeback);
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Se intentara bloquear el DTB de ID = %d", nextToBlock->id);

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

				//Me fijo cuantas sentencias ejecuto (sacando una diferencia con el PC anterior)
				int sentencesRun;
				sentencesRun = (nextToBlock->newProgramCounter) - (target->programCounter);
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Se bloqueara el DTB de id %d, leyo %d sentencias", target->id, sentencesRun);
				//Le agrego esa cantidad de sentencias esperadas a todos los DTBs de NEW
				AggregateSentencesWhileAtNEW(sentencesRun);

				//Le actualizo el PC, el quantum sobrante, y los archivos abiertos (por las dudas)
				target->programCounter = nextToBlock->newProgramCounter;
				target->quantumRemainder = nextToBlock->quantumRemainder;
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->El DTB bloqueado tiene el PC en % d y le sobraron %d del quantum", target->programCounter, target->quantumRemainder);
				if(!nextToBlock->dummyComeback)
				{
					UpdateOpenedFiles(target, nextToBlock->openedFilesUpdate, false);
				}
				AddToBlocked(target);
			}
			pthread_mutex_unlock(&mutexToBeBlocked);

			//No me olvido de este free! Ni de avisar al planificador que, tras bloquear todos, vuelva a planificar
			free(nextToBlock);
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Bloqueados todos los DTBs pendientes. Volviendo a planificacion normal");
			AddPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

		//Tras aviso desde DAM
		else if(currentTask == PCP_TASK_UNLOCK_DTB)
		{

			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->PCP bajo la orden de pasar DTBs a READY y desbloquearlos");
			//Puntero al proximo DTB a desbloquear, segun vaya sacando de la cola
			UnlockableInfo* nextToUnlock;
			pthread_mutex_lock(&mutexToBeUnlocked);
			while(!queue_is_empty(toBeUnlocked))
			{

				nextToUnlock = (UnlockableInfo*) queue_pop(toBeUnlocked);
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Se intentara desbloquear el DTB de id = %d", nextToUnlock->id);
				printf("\n\nhere we go=%d\n\n",nextToUnlock->overwritePC);
				//Closure anidada, para poder hallar de las colas el DTB con el mismo ID que el recien sacado de la cola
				bool IsDTBToBeUnlocked(void* aDTB)
				{
					DTB* realDTB = (DTB*) aDTB;
					if(realDTB->id == nextToUnlock->id && realDTB->initialized)
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
				if(target==NULL)
				{
					Logger_Log(LOG_ERROR,"\n\nERROR CRÍTICO: No se encontro el DTB en ninguna cola\n\n");
					exit(-1);
				}

				//Me fijo cuantas sentencias leyo, con una diferencia de PCs
				int sentencesRun;

				//Le actualizo el program counter (se debe haber movido) y los archivos abiertos, y lo paso a READY
				//Ojo, solo le altero el PC si UnlockableInfo me informa que debo hacerlo
				if(nextToUnlock->overwritePC)
				{
					sentencesRun = (nextToUnlock->newProgramCounter) - (target->programCounter);
					target->programCounter = nextToUnlock->newProgramCounter;
				}
				else
				{
					//Pero si volvia de IO, avanzo una sola sentencia (IOs son atomicas)
					sentencesRun = 1;
					//Pelotudos del futuro no incrementen el program counter aca.
				}

				//Agrego las sentencias esperadas de los de NEW, y actualizo los archivos abiertos
				AggregateSentencesWhileAtNEW(sentencesRun);
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Se desbloqueara el DTB de id %d, leyo %d sentencias", target->id, sentencesRun);
				UpdateOpenedFiles(target, nextToUnlock->openedFilesUpdate, nextToUnlock->appendOFs);
				Logger_Log(LOG_DEBUG, "SAFA:PLANIF->Actualizados los archivos abiertos de dicho DTB");
				printf("\n\nfirst=%d\n\n",target->firstResponseTime);
				//Si no tiene marcado el instante de la primer respuesta (esta como 0),
				//entonces debo marcarlo en este instante (es una respuesta del sistema)
				if(target->firstResponseTime == 0)
				{
					time(&(target->firstResponseTime));
					Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Primer respuesta del sistema registrada para el DTB!");
				}

				//Muevo este DTB a la cola de READY
				AddToReady(target, schedulingRules.name);

			}
			pthread_mutex_unlock(&mutexToBeUnlocked);

			//No me olvido de este free! Ni de avisar al planificador que, tras desbloquear todos, vuelva a planificar
			free(nextToUnlock);
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Desbloqueados todos los DTBs pendientes. Volviendo a planificacion normal");
			AddPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

		else if(currentTask == PCP_TASK_END_DTB)
		{

			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->PCP bajo la orden de pasar DTBs a EXIT y eliminarlos");
			//Puntero a un uint32_t, que va a guardar el ID de cada DTB a abortar/finalizar de la cola
			uint32_t* nextToEnd;
			pthread_mutex_lock(&mutexToBeEnded);
			while(!queue_is_empty(toBeEnded))
			{

				nextToEnd = (uint32_t*) queue_pop(toBeEnded);
				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Se intentara eliminar el DTB de id = %d", *nextToEnd);

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
				DTB* target = (DTB*) list_remove_by_condition(BLOCKEDqueue, IsDTBToBeEnded);
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

				Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Se eliminará el DTB de id %d", target->id);

				//Si no tiene marcado el instante de la primer respuesta (esta como 0),
				//entonces debo marcarlo en este instante (es una respuesta del sistema)
				if(target->firstResponseTime == 0)
				{
					time(&(target->firstResponseTime));
					Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Primer respuesta del sistema registrada para el DTB!");
				}

				//Lo muevo a la "cola" de EXIT, actualizando su estado
				AddToExit(target);

			}
			pthread_mutex_unlock(&mutexToBeEnded);

			//Libero este puntero, y ademas le aviso al PCP que siga planificando normal
			free(nextToEnd);
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Terminados todos los DTBs pendientes. Volviendo a planificacion normal");
			AddPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

		//Aplico retardo de planificacion; multiplico por mil, ya que son milisegundos y el usleep es con microsegundos
		usleep((settings->retardo) * 1000);

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
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Cambio de algoritmo RR a VRR. Colas movidas");
			break;

		case ALGORITHM_CHANGE_RR_TO_OWN :
			queue_to_list(READYqueue_RR, READYqueue_Own);
			list_sort(READYqueue_Own, DescendantPriority);
			queue_clean(READYqueue_RR);
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Cambio de algoritmo RR a IOBF. Colas movidas");
			break;

		case ALGORITHM_CHANGE_VRR_TO_RR :
			list_to_queue(READYqueue_VRR, READYqueue_RR);
			list_clean(READYqueue_VRR);
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Cambio de algoritmo VRR a RR. Colas movidas");
			break;

		case ALGORITHM_CHANGE_VRR_TO_OWN :
			list_copy(READYqueue_VRR, READYqueue_Own);
			list_sort(READYqueue_Own, DescendantPriority);
			list_clean(READYqueue_VRR);
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Cambio de algoritmo VRR a IOBF. Colas movidas");
			break;

		//NOTA: Al pasar de IOBF a otro algoritmo, el orden de la cola es el que adquirieron por las prioridades
		//Sin embargo, al agregar DTBs a dicha cola, no se insertara y se ordenara, sino que ira al final
		case ALGORITHM_CHANGE_OWN_TO_RR :
			list_to_queue(READYqueue_Own, READYqueue_RR);
			list_clean(READYqueue_Own);
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Cambio de algoritmo IOBF a RR. Colas movidas");
			break;

		case ALGORITHM_CHANGE_OWN_TO_VRR :
			list_copy(READYqueue_Own, READYqueue_VRR);
			list_clean(READYqueue_VRR);
			Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Cambio de algoritmo IOBF a VRR. Colas movidas");
			break;

	}

	return;

}

bool NoReadyDTBs(char* algorithm)
{

	//Asumo, de entrada, que las colas no estan vacias y si hay DTBs en READY
	bool noneExist = false;
	//Uso este mutex por las dudas, voy a revisar la cola READY y justo tal vez la tocan desde otro hilo
	pthread_mutex_lock(&mutexREADY);

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

	pthread_mutex_unlock(&mutexREADY);

	if(noneExist)
	{
		Logger_Log(LOG_DEBUG, "SAFA::PLANIF->No hay DTBs en READY, el PCP no tiene nada que planificar");
	}


	return noneExist;

}

void UpdateOpenedFiles(DTB* toBeUpdated, t_dictionary* currentOFs, bool dontOverwrite)
{

	//SI Y SOLO SI debo sobreescribir el diccionario (por una operacion Abrir exitosa; si vale true, es agregar 0 o 1 archivos abiertos)
	if(!dontOverwrite)
	{
		//Limpio el diccionario y borro todos sus elementos, tal vez cerro alguno y no quiero que quede de mas
		dictionary_clean_and_destroy_elements(toBeUpdated->openedFiles, LogicalAddressDestroyer);
		toBeUpdated->openedFilesAmount = 0;
	}

	//Haya vaciado el diccionario anterior o no, actualizo el diccionario, es el mismo codigo
	//Closure anidada, para que haga el put en el diccionario del DTB pasado por parametro

	void UpdateSingleFile(char* path, void* address)
	{
		//Uso el putMAESTRO por las dudas, las commons se la comen
		dictionary_putMAESTRO(toBeUpdated->openedFiles, path, address, LogicalAddressDestroyer);
		toBeUpdated->openedFilesAmount++;
		printf("\n\ncant archivos abiertos=%d\n\n",toBeUpdated->openedFilesAmount);
	}

	//Recorro el diccionario parametro (el actualizado) entero, y ejecuto la closure para que sobreescriba cada una
	dictionary_iterator(currentOFs, UpdateSingleFile);

	dictionary_destroy(currentOFs);
}

SerializedPart FlattenPathsAndAddresses(t_dictionary* openFilesTable)
{

	void* result = malloc(1);
	int offset = 0, totalSize = 0;
	int nextSize;
	if(openFilesTable==NULL)
		Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Tabla de archivos abiertos invalida");
	else
		Logger_Log(LOG_DEBUG, "SAFA::PLANIF->Tabla de archivos abiertos OK");
	printf("\n\npor imprimir diciconario?\n\n");
	void CopyPath(char* path, void* address)
	{

		printf("\n\npath=%s-address=%d\n\n",path,*((uint32_t*)address));
		nextSize = strlen(path);							//Obtengo el largo del path
		totalSize += (nextSize + 2 + sizeof(uint32_t));		//Sumo a totalSize, y sumo 2 mas por los : y la ,
		result = realloc(result, totalSize);				//Realloco memoria
		memcpy(result + offset, path, nextSize);			//Copio el path y muevo el offset
		offset += nextSize;
		memcpy(result + offset, ":", 1);					//Copio el : (separa path de DL) y muevo el offset
		offset++;
		memcpy(result + offset, address, sizeof(uint32_t));	//Copio la DL y muevo el offset
		offset += sizeof(uint32_t);
		memcpy(result + offset, ",", 1);					//Copio la , (separa registros) y muevo el offset
		offset++;
	}

	dictionary_iterator(openFilesTable, CopyPath);			//Llamo al closure de arriba para hacerlo con todos los registros
	if (totalSize == 0)
			totalSize = 1;
//	memcpy(result + offset - 1, ";", 1);					//Pongo el ; al final de la cadena

	memcpy(result + totalSize - 1, ";", 1);						//Pongo el ; al final de la cadena
	SerializedPart sp = {.data = result, .size = totalSize};
	return sp;												//Queda : "arch1:d1,arch2:d2,...,archN:dN;"

}

SerializedPart* GetMessageForCPU(DTB* chosenDTB)
{

	declare_and_init(idToSend, uint32_t, chosenDTB->id)
	declare_and_init(flagToSend, uint32_t, chosenDTB->initialized)
	declare_and_init(pathAddressToSend, uint32_t, chosenDTB->pathLogicalAddress)


	printf("\n\npath address int=%d\n\n",chosenDTB->pathLogicalAddress);

	printf("\n\npc del chosen dtb=%d\n\n",chosenDTB->programCounter);
	declare_and_init(pcToSend, uint32_t, chosenDTB->programCounter)
	declare_and_init(quantumToSend, uint32_t, chosenDTB->quantumRemainder)
	//Cantidad de archivos abiertos
	declare_and_init(ofaToSend, uint32_t, chosenDTB->openedFilesAmount)

	//Estructuras con los datos a serializar y mandar como cadena
	SerializedPart idSP, flagSP, pathSP, pathAddressSP, pcSP, quantumSP, ofaSP, filesSP;
	idSP.size = sizeof(uint32_t);
	idSP.data = idToSend;
	printf("\n\nid=%d\n\n",*((uint32_t*)(idSP.data)));
	flagSP.size = sizeof(uint32_t);
	flagSP.data = flagToSend;

	printf("\n\nflag =%d\n\n",*((uint32_t*)(flagSP.data)));
	pathSP.size = strlen(chosenDTB->pathEscriptorio) + 1;
	printf("\n\nsize=%d--path=%s\n\n",pathSP.size,chosenDTB->pathEscriptorio);
	pathSP.data = malloc(pathSP.size);
	strcpy(pathSP.data, chosenDTB->pathEscriptorio);
	pathAddressSP.size = sizeof(uint32_t);
	pathAddressSP.data = pathAddressToSend;

	printf("\n\nlogical address int=%d\n\n",*((uint32_t*)(pathAddressSP.data)));
	pcSP.size = sizeof(uint32_t);
	pcSP.data = pcToSend;

	printf("\n\npc int=%d\n\n",*((uint32_t*)(pcSP.data)));
	quantumSP.size = sizeof(uint32_t);
	quantumSP.data = quantumToSend;

	printf("\n\nquantum int=%d\n\n",*((uint32_t*)(quantumSP.data)));
	ofaSP.size = sizeof(uint32_t);
	ofaSP.data = ofaToSend;

	printf("\n\ncant archivos abiertos int=%d\n\n",*((uint32_t*)(ofaSP.data)));
	filesSP = FlattenPathsAndAddresses(chosenDTB->openedFiles);

	//La idea es armar un paquete serializado que va a tener la estructura:
	//|IDdelDTB|Flag|PathEscriptorioAsociado|Dir.LogicaPath|ProgramCounterDelDTB|QuantumAEjecutar|CantArchivosAbiertos|Archivos
	//(cada cual con su respectivo tamanio antes del dato en si)
	//Los archivos se mandan como: "arch1:d1,arch2:d2,...,archN:dN;"
	SerializedPart* message = Serialization_Serialize(8, idSP, flagSP, pathSP, pathAddressSP, pcSP, quantumSP, ofaSP, filesSP);

//	SerializedPart* message = Serialization_Serialize(6, idSP, flagSP, pathAddressSP, pcSP, quantumSP, ofaSP);

	//Hago free de todos esos punteros que use para crear la cadena serializada
	free(idToSend);
	free(flagToSend);
	free(pathAddressToSend);
	free(pcToSend);
	free(quantumToSend);
	free(ofaToSend);
	free(filesSP.data);
	return message;

}

SerializedPart* ScheduleRR(int quantum)
{

	//Round Robin es un FIFO en el cual tengo en cuenta el quantum, no mucho mas que eso
	DTB* chosenDTB = GetNextReadyDTB();
	chosenDTB->quantumRemainder = quantum;
	Logger_Log(LOG_DEBUG, "SAFA::PLANIF_RR->Confeccionando mensaje con DTB de path %s", chosenDTB->pathEscriptorio);
	//Obtengo la cadena a enviarle al CPU asignado; detalle de la misma dentro de la funcion; el free es en otro lado
	printf("\n\n\ndtb= %s\n\n\n",chosenDTB->pathEscriptorio);
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

	Logger_Log(LOG_DEBUG, "SAFA::PLANIF_VRR->Confeccionando mensaje con DTB de path %s, con un quantum de %d", chosenDTB->pathEscriptorio, chosenDTB->quantumRemainder);

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

	Logger_Log(LOG_DEBUG, "SAFA::PLANIF_IOBF->Confeccionando mensaje con DTB de path %s", chosenDTB->pathEscriptorio);

	//Igual que en los otros, obtengo el mensaje a enviar y lo guardo en la estructura global; muevo DTB a EXEC
	SerializedPart* packet = GetMessageForCPU(chosenDTB);
	AddToExec(chosenDTB);
	return packet;

}

////////////////////////////////////////////////////////////

int Metrics_TotalRunSentencesAmount()
{

	//Va con un mutex por afuera, se asume que ya tiene acceso exclusivo a las colas
	int totalAmount = 0;

	//Closure a aplicar en todas las colas
	void AddSentencesFromDTB(void* aDTB)
	{
		DTB* castDTB = (DTB*) aDTB;
		if(castDTB->initialized != 0)
		{
			totalAmount += castDTB->programCounter;
		}
	}

	//Para las de READY, recorro las tres y fue; hay dos que van a estar vacias, me ahorro ifs y parametro
	list_iterate(READYqueue_RR->elements, AddSentencesFromDTB);
	list_iterate(READYqueue_VRR, AddSentencesFromDTB);
	list_iterate(READYqueue_Own, AddSentencesFromDTB);
	list_iterate(BLOCKEDqueue, AddSentencesFromDTB);
	list_iterate(EXECqueue, AddSentencesFromDTB);
	list_iterate(EXITqueue, AddSentencesFromDTB);

	return totalAmount;

}

int Metrics_TotalIOSentencesAmount()
{

	//Va con un mutex por afuera, se asume que ya tiene acceso exclusivo a las colas
	int ioAmount = 0;

	//Closure a aplicar en todas las colas
	void AddIOOpsFromDTB(void* aDTB)
	{
		DTB* castDTB = (DTB*) aDTB;
		if(castDTB->initialized != 0)
		{
			ioAmount += castDTB->ioOperations;
		}
	}

	//Para las de READY, recorro las tres y fue; hay dos que van a estar vacias, me ahorro ifs y parametro
	list_iterate(READYqueue_RR->elements, AddIOOpsFromDTB);
	list_iterate(READYqueue_VRR, AddIOOpsFromDTB);
	list_iterate(READYqueue_Own, AddIOOpsFromDTB);
	list_iterate(BLOCKEDqueue, AddIOOpsFromDTB);
	list_iterate(EXECqueue, AddIOOpsFromDTB);
	list_iterate(EXITqueue, AddIOOpsFromDTB);

	return ioAmount;

}

int Metrics_TotalDTBPopulation()
{

	//Ojo, no cuento los procesos en NEW ya que esos no tuvieron operaciones de IO ni sentencias ejecutadas
	int quantity = 0;

	//Para las de READY, recorro las tres y fue; hay dos que van a estar vacias, me ahorro ifs y parametro
	quantity += queue_size(READYqueue_RR);
	quantity += list_size(READYqueue_VRR);
	quantity += list_size(READYqueue_Own);
	quantity += list_size(BLOCKEDqueue);
	quantity += list_size(EXECqueue);
	quantity += list_size(EXITqueue);

	//Descuento el Dummy
	return quantity - 1;

}

float Metrics_AverageIOSentences()
{

	float avg;
	//Para hallar el promedio, hago SentenciasIO/CantidadDTBs
	avg = (float) (Metrics_TotalIOSentencesAmount() / Metrics_TotalDTBPopulation());
	return avg;

}

float Metrics_AverageExitingSentences()
{

	float avg;
	int totalAmount = 0;

	//Closure a aplicar en la cola de EXIT, para obtener la cantidad de sentencias ejecutadas de los DTBs de alli
	void AddSentencesFromDTB(void* aDTB)
	{
		DTB* castDTB = (DTB*) aDTB;
		if(castDTB->initialized != 0)
		{
			totalAmount += castDTB->programCounter;
		}
	}

	list_iterate(EXITqueue, AddSentencesFromDTB);

	//Divido la cantidad de sentencias recien hallada por la cantidad de DTBs en EXIT (solo evaluo esos)
	avg = (float) (totalAmount / list_size(EXITqueue));
	return avg;

}

float Metrics_IOSentencesPercentage()
{

	float percentage;
	//Casteo a float para que tome decimales, y multiplico por 100 para que quede tipo porcentaje
	percentage = (float) (Metrics_TotalIOSentencesAmount() / Metrics_TotalRunSentencesAmount());
	percentage *= 100;
	return percentage;

}

float Metrics_SumAllResponseTimes()
{

	float totalTime = 0;

	//Closure para sumar el tiempo esperado (primera respuesta - spawn) de cada DTB
	//Solo debo tener en cuenta aquellos cuyo tiempo de primera respuesta no sea 0 (hayan recibido una)
	void SumTimeFromSingleDTB(void* aDTB)
	{
		DTB* castDTB = (DTB*) aDTB;
		if((castDTB->firstResponseTime != 0) && (castDTB->initialized != 0))
		{
			totalTime += (difftime(castDTB->firstResponseTime, castDTB->spawnTime));
		}
	}

	list_iterate(READYqueue_RR->elements, SumTimeFromSingleDTB);
	list_iterate(READYqueue_VRR, SumTimeFromSingleDTB);
	list_iterate(READYqueue_Own, SumTimeFromSingleDTB);
	list_iterate(BLOCKEDqueue, SumTimeFromSingleDTB);
	list_iterate(EXECqueue, SumTimeFromSingleDTB);
	list_iterate(EXITqueue, SumTimeFromSingleDTB);

	return totalTime;

}

int Metrics_ResponsedDTBPopulation()
{

	//Ojo, no cuento los procesos en NEW ya que esos no tuvieron operaciones de IO ni sentencias ejecutadas
	int quantity = 0;

	//Closure para aplicar sobre las colas; solo tengo en cuenta aquellos procesos que ya han recibido una respuesta
	void CountResponsedDTB(void* aDTB)
	{
		DTB* castDTB = (DTB*) aDTB;
		if((castDTB->firstResponseTime != 0) && (castDTB->initialized != 0))
		{
			quantity++;
		}
	}

	//Para las de READY, recorro las tres y fue; hay dos que van a estar vacias, me ahorro ifs y parametro
	list_iterate(READYqueue_RR->elements, CountResponsedDTB);
	list_iterate(READYqueue_VRR, CountResponsedDTB);
	list_iterate(READYqueue_Own, CountResponsedDTB);
	list_iterate(BLOCKEDqueue, CountResponsedDTB);
	list_iterate(EXECqueue, CountResponsedDTB);
	list_iterate(EXITqueue, CountResponsedDTB);

	return quantity;

}

float Metrics_AverageResponseTime()
{

	float avg;
	//Para el tiempo promedio hago SumaTiemposRespuesta/CantidadDTBsRespondidos
	avg = (float) (Metrics_SumAllResponseTimes() / Metrics_ResponsedDTBPopulation());
	return avg;

}

////////////////////////////////////////////////////////////

void DeleteSemaphores()
{

	pthread_mutex_destroy(&mutexPLPtasksQueue);
	pthread_mutex_destroy(&mutexPCPtasksQueue);
	pthread_mutex_destroy(&mutexREADY);
	pthread_mutex_destroy(&mutexScriptsQueue);
	pthread_mutex_destroy(&mutexToBeBlocked);
	pthread_mutex_destroy(&mutexToBeUnlocked);
	pthread_mutex_destroy(&mutexToBeEnded);
	pthread_mutex_destroy(&mutexNEW);
	pthread_mutex_destroy(&mutexBeingDummied);
	pthread_mutex_destroy(&mutexDummiedQueue);
	sem_destroy(&workPLP);
	sem_destroy(&workPCP);

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

void CreatableGDTDestroyer(void* aDummied)
{

	CreatableGDT* realDummied = (CreatableGDT*) aDummied;
	free(realDummied->script);
	free(realDummied);

}

void DeleteQueuesAndLists()
{

	queue_destroy_and_destroy_elements(PCPtasksQueue, free);
	queue_destroy_and_destroy_elements(PLPtasksQueue, free);
	queue_destroy_and_destroy_elements(scriptsQueue, ScriptDestroyer);
	queue_destroy_and_destroy_elements(NEWqueue, DTBDestroyer);
	queue_destroy_and_destroy_elements(READYqueue_RR, DTBDestroyer);
	list_destroy_and_destroy_elements(BLOCKEDqueue, DTBDestroyer);
	list_destroy_and_destroy_elements(EXECqueue, DTBDestroyer);
	list_destroy_and_destroy_elements(EXITqueue, DTBDestroyer);
	queue_destroy_and_destroy_elements(toBeUnlocked, UnlockableInfoDestroyer);
	queue_destroy_and_destroy_elements(toBeBlocked, BlockableInfoDestroyer);
	queue_destroy_and_destroy_elements(toBeEnded, EndableDestroyer);
	list_destroy_and_destroy_elements(beingDummied, DTBDestroyer);
	queue_destroy_and_destroy_elements(dummiedQueue, CreatableGDTDestroyer);

}

void DeleteSchedulingGlobalVariables()
{

	//El free del Dummy no se hace aca, sino al hacer el delete de Colas y Listas (estara en alguna)
	DeleteSemaphores();
	DeleteQueuesAndLists();

}
