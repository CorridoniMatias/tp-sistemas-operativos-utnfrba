#include "headerFiles/Scheduling.h"

////////////////////////////////////////////////////////////

void InitQueuesAndLists()
{

	scriptsQueue = queue_create();
	NEWqueue = queue_create();
	READYqueue = queue_create();
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
	sem_init(&workPLP, 0, 0);

}

void CreateDummy()
{

	dummyDTB = (DTB*) malloc(sizeof(DTB));

	dummyDTB->id = 0;									//Valor basura, siempre tendra el del DTB que este cargando
	dummyDTB->initialized = 0;
	dummyDTB->programCounter = 0;
	dummyDTB->status = -1;								//Valor basura, todavia no esta en ninguna cola

	return;

}

void InitGlobalVariables()
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

	return newDTB;

}

void AddToNew(DTB* myDTB)
{

	myDTB->status = DTB_STATUS_NEW;
	//Todavia no prendo el flag initialized, falta la operacion Dummy
	queue_push(NEWqueue, myDTB);

}

void AddToReady(DTB* myDTB)
{

	myDTB->status = DTB_STATUS_READY;
	queue_push(READYqueue, myDTB);

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

DTB* GetNextDTB()
{

	DTB* nextToExecute;
	nextToExecute = queue_pop(READYqueue);
	return nextToExecute;

}

////////////////////////////////////////////////////////////

void PlanificadorLargoPlazo(void* gradoMultiprogramacion)
{
	int multiprogrammingDegree = (int) gradoMultiprogramacion;

	while(1)
	{

		sem_wait(&workPLP);

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
			//Hasta vaciar la cola de scripts, los voy sacando y guardando en DTBs que agrego a NEW
			while(!queue_is_empty(scriptsQueue))
			{
				DTB* newDTB;
				char* newPath = (char*) queue_pop(scriptsQueue);
				//No le hago free aca, cuando elimine todos lo hare
				newDTB = CreateDTB(newPath);
			    AddToNew(newDTB);
			}

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

			AddToReady(toBeInitialized);
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

void PlanificadorCortoPlazo(void* algoritmo)
{

	while(1)
	{

		if(PCPtask == PCP_TASK_NORMAL_SCHEDULE)
		{

			//Si no hay ningun CPU libre, espero dos segundos y salgo del ciclo
			//O deberia quedarme a esperar que haya uno libre? Puede entrar un pedido por consola...
			if(!ExistsIdleCPU())
			{
				sleep(2);
				continue;
			}

			//Agarro el primer CPU libre que haya, lo saco de la lista y lo pongo aca
			CPU* chosenCPU = list_remove_by_condition(cpus, (void*)IsIdle);
			toBeAssigned->cpuSocket = chosenCPU->socket;

			void* messageToSend;

			//Elegir el DTB adecuado, y obtener el mensaje a enviarle al CPU; ponerlo en EXEC
			if((strcmp((char*)algoritmo, "RR")) == 0)
			{
				messageToSend = ScheduleRR(settings->quantum);
			}
			else if((strcmp((char*)algoritmo, "VRR")) == 0)
			{
				messageToSend = ScheduleVRR(settings->quantum);
			}
			//if(algoritmo == "PROPIO") => scheduleSelf()

			memcpy(toBeAssigned->message, messageToSend, strlen(messageToSend) + 1);

			//Enviarle el mensaje al CPU, y actualizarle el estado en la lista
			//Deberia activar algun semaforo para que sepa que debe mandarselo por el socket?

		}

		//Esto es solo pasar el Dummy a READY para poder planificarlo desde ahi
		else if(PCPtask == PCP_TASK_LOAD_DUMMY)
		{
			dummyDTB->status = DTB_STATUS_READY;
			AddToReady(dummyDTB);
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
				UpdateOpenedFiles(target, nextToBlock->openedFilesUpdate);
				AddToBlocked(target);
			}

			//No me olvido de este free! Ni de avisar al planificador que, tras bloquear todos, vuelva a planificar
			free(nextToBlock);
			SetPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

		//Tras aviso desde DAM
		else if(PCPtask == PCP_TASK_UNLOCK_DTB)
		{

			//Puntero al proximo DTB a desbloquear, segun vaya sacando de la cola
			UnlockableInfo* nextToUnlock = (UnlockableInfo*) malloc(sizeof(UnlockableInfo));
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
				if(!target)
				{
					target = list_remove_by_condition(BLOCKEDqueue, IsDTBToBeUnlocked);
				}

				//Le actualizo el program counter (se debe haber movido) y los archivos abiertos, y lo paso a READY
				target->programCounter = nextToUnlock->newProgramCounter;
				UpdateOpenedFiles(target, nextToUnlock->openedFilesUpdate);
				AddToReady(target);

			}

			//No me olvido de este free! Ni de avisar al planificador que, tras desbloquear todos, vuelva a planificar
			free(nextToUnlock);
			SetPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

		else if(PCPtask == PCP_TASK_END_DTB)
		{

			//Puntero a un uint32_t, que va a guardar el ID de cada DTB a abortar/finalizar de la cola
			uint32_t* nextToEnd = (uint32_t*) malloc(sizeof(uint32_t));
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

				//Busco el DTB en EXEC (por si me aviso CPU) que tiene dicho ID; si no esta ahi, lo busco en BLOCKED (aviso DAM)
				DTB* target = list_remove_by_condition(EXECqueue, IsDTBToBeEnded);
				if(!target)
				{
					target = list_remove_by_condition(BLOCKEDqueue, IsDTBToBeEnded);
				}

				//Lo muevo a la "cola" de EXIT, actualizando su estado
				AddToExit(target);

			}

			//Libero este puntero, y ademas le aviso al PCP que siga planificando normal
			free(nextToEnd);
			SetPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

	}

}

void UpdateOpenedFiles(DTB* toBeUpdated, t_dictionary* currentOFs)
{

	//Closure anidada, para que haga el put con el diccionario del DTB pasado por parametro
	void UpdateSingleFile(char* path, void* address)
	{
		//Ojo, si ya existe la key debo liberar su data! Ya que sino un put sobre una key ya existente
		//me hace perder la referencia a dicho data, y provoca un memory leak!
		if(dictionary_has_key(toBeUpdated->openedFiles, path))
		{
			free(dictionary_get(toBeUpdated->openedFiles, path));
		}
		dictionary_put(toBeUpdated->openedFiles, path, address);
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

void* GetMessageForCPU(DTB* chosenDTB)
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
	void* message = Serialization_Serialize(8, idSP, flagSP, pathSP, pathAddressSP, pcSP, quantumSP, ofaSP, filesSP);

	//Hago free de todos esos punteros que use para crear la cadena serializada
	free(idToSend);
	free(flagToSend);
	free(pathAddressToSend);
	free(pcToSend);
	free(quantumToSend);
	free(ofaToSend);

	return message;

}

void* ScheduleRR(int quantum)
{

	//Round Robin es un FIFO en el cual tengo en cuenta el quantum, no mucho mas que eso
	DTB* chosenDTB = GetNextDTB();
	chosenDTB->quantumRemainder = quantum;

	//Obtengo la cadena a enviarle al CPU asignado; detalle de la misma dentro de la funcion
	void* packet = GetMessageForCPU(chosenDTB);

	AddToExec(chosenDTB);
	//OJO: Alguien deberia hacer free de ese packet despues
	return packet;

}

void* ScheduleVRR(int maxQuantum)
{

	DTB* chosenDTB = GetNextDTB();
	//Si le quedara 0 de quantum (se quedo sin) o tuviera mas del maximo (por haber sido
	//planificado con otro algoritmo antes), le actualizo el maximo quantum a ejecutar
	if((chosenDTB->quantumRemainder == 0) || (chosenDTB->quantumRemainder > maxQuantum))
	{
		chosenDTB->quantumRemainder = maxQuantum;
	}

	//Obtengo la cadena a enviarle al CPU asignado; detalle de la misma dentro de la funcion
	void* packet = GetMessageForCPU(chosenDTB);

	AddToExec(chosenDTB);
	//OJO: Alguien deberia hacer free de ese packet despues
	return packet;

}

////////////////////////////////////////////////////////////

void DeleteSemaphores()
{

	pthread_mutex_destroy(&mutexPLPtask);
	pthread_mutex_destroy(&mutexPCPtask);
	sem_destroy(&workPLP);

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
	free(castBI);
	free(castBI);

}

void UnlockableInfoDestroyer(void* UI)
{

	UnlockableInfo* castUI = (UnlockableInfo*) UI;
	dictionary_destroy_and_destroy_elements(castUI->openedFilesUpdate, LogicalAddressDestroyer);
	free(castUI);
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
	dictionary_destroy_and_destroy_elements(castDTB->openedFiles, LogicalAddressDestroyer);
	free(castDTB);
	free(aDTB);

}

void DeleteQueuesAndLists()
{

	queue_destroy_and_destroy_elements(scriptsQueue, ScriptDestroyer);
	queue_destroy_and_destroy_elements(NEWqueue, DTBDestroyer);
	queue_destroy_and_destroy_elements(READYqueue, DTBDestroyer);
	list_destroy_and_destroy_elements(BLOCKEDqueue, DTBDestroyer);
	list_destroy_and_destroy_elements(EXECqueue, DTBDestroyer);
	list_destroy_and_destroy_elements(EXITqueue, DTBDestroyer);
	queue_destroy_and_destroy_elements(toBeUnlocked, UnlockableInfoDestroyer);
	queue_destroy_and_destroy_elements(toBeBlocked, BlockableInfoDestroyer);
	queue_destroy_and_destroy_elements(toBeEnded, EndableDestroyer);

}

void DeleteGlobalVariables()
{

	free(justDummied->script);
	free(justDummied);
	//El free del Dummy no se hace aca, sino al hacer el delete de Colas y Listas (estara en alguna)

	DeleteSemaphores();
	DeleteQueuesAndLists();

}
