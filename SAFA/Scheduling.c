#include "headerFiles/Scheduling.h"

////////////////////////////////////////////////////////////

void InitQueuesAndLists()
{

	NEWqueue = list_create();
	READYqueue = queue_create();
	BLOCKEDqueue = list_create();
	EXECqueue = list_create();
	EXITqueue = list_create();

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

	dummyDTB->id = 0;
	dummyDTB->initialized = 0;
	dummyDTB->programCounter = 0;
	dummyDTB->status = -1;								//Valor basura, todavia no esta en ninguna cola

	return;

}

void InitGlobalVariables()
{

	nextID = 1;										//Arrancan en 1, la 0 es reservada para el Dummy

	InitSemaphores();
	InitQueuesAndLists();

	//Al ser extern en main, deberian inicializarse ahi mismo
	toBeCreated = (CreatableGDT*) malloc(sizeof(CreatableGDT));
	toBeCreated->script = malloc(1);				//Medio paragua, para poder hacer reallocs

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
			toBeCreated = realloc(toBeCreated, sizeof(int) + pathLength);
			toBeCreated->script = realloc(toBeCreated->script, pathLength);
			strcpy(toBeCreated->script, path);

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
	//Le pongo 0 para saber que no lo inicialice; nunca llegara a PCP con 0, solo el Dummy lo haria
	newDTB->initialized = 0;
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
	list_add(NEWqueue, myDTB);

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

bool IsInitialized(DTB* myDTB)
{

	if(myDTB->initialized == 1)
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool IsDummy(DTB* myDTB)
{

	if(myDTB->id == 0)
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool IsToBeMoved(DTB* myDTB)
{

	if(myDTB->id == toBeMoved->dtbID)
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

			int inMemoryAmount = queue_size(READYqueue) + list_size(BLOCKEDqueue) + list_size(EXECqueue);
			//Descuento el dummy de la cantidad de procesos en memoria (no afecta para el grado de multiprogramacion)
			//No lo descuento si su estado es basura (esto solo pasaria si aun no se lo utilizo)
			if(dummyDTB->status > 0)
			{
				inMemoryAmount--;
			}

			//Si el grado de multiprogramacion no lo permite, o no hay procesos en NEW, voy a la proxima iteracion del while
			if(inMemoryAmount >= multiprogrammingDegree || list_is_empty(NEWqueue))
			{
				sleep(3);				//Retardo ficticio, para debuggear; puede servir, para esperar
				continue;
			}

			//Si el Dummy esta en estado BLOCKED (no en la cola, sino esperando a ser asignado), agarro el primero de la cola
			if(dummyDTB->status == DTB_STATUS_BLOCKED)
			{
				DTB* queuesFirst = list_remove(NEWqueue, 1);

				//Si el primero no esta inicializado, es porque debo avisar al PCP que haga la operacion Dummy
				if(queuesFirst->initialized == 0)
				{

					//Devuelvo el que saque a NEW
					AddToNew(queuesFirst);

					//Realloco el dummyDTB, porque su char* cambio; cargo el path del DTB a inicializar (no lo hace el PCP)
					int pathLength = strlen(queuesFirst->pathEscriptorio) + 1;
					int newSize = (sizeof(int) * 5) + pathLength;		//Medio cabeza, mejorar; 5 por los 5 enteros fijos
					dummyDTB = realloc(dummyDTB, newSize);
					dummyDTB->pathEscriptorio = realloc(dummyDTB->pathEscriptorio, pathLength);
					strcpy(dummyDTB->pathEscriptorio, queuesFirst->pathEscriptorio);

					//Le aviso al PCP que debe hacer la tarea de LOAD_DUMMY (pasarlo a READY, nada mas)
					SetPCPTask(PCP_TASK_LOAD_DUMMY);

				}
				//Si el primero esta inicializado, lo agrego a READY; no aviso al PCP, ya deberia estar planificando (?)
				else if(queuesFirst->initialized == 1)
				{
					AddToReady(queuesFirst);
				}
			}

			//Si el Dummy esta en READY o en EXEC, es porque no puede usarse para ningun DTB a inicializar
			//Por eso, agarro el primero que ya este inicializado (que posea el flag en 1)
			else if(dummyDTB->status == DTB_STATUS_READY || dummyDTB->status == DTB_STATUS_EXEC)
			{

				//Si no hay ninguno inicializado, no hay nada que hacer, voy a la siguiente iteracion (bloqueo en el semaforo)
				if(!list_any_satisfy(NEWqueue, (void*)IsInitialized))
				{
					sleep(3);
					continue;
				}
				DTB* firstInitialized = list_remove_by_condition(NEWqueue, (void*)IsInitialized);
				AddToReady(firstInitialized);

			}

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
			DTB* toBeInitialized = list_remove_by_condition(NEWqueue, (void*)IsDTBtoBeInitialized);
			toBeInitialized->initialized = 1;
			AddToNew(toBeInitialized);
			//Vuelvo a poner la tarea del PLP en Planificacion Normal (1)
			SetPCPTask(PCP_TASK_NORMAL_SCHEDULE);
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

		else if(PCPtask == PCP_TASK_BLOCK_DUMMY)
		{

			//Saco el Dummy de la cola de EXEC, y lo paso a la de BLOCKED (modifico su estado)
			list_remove_by_condition(EXECqueue, (void*)IsDummy);
			AddToBlocked(dummyDTB);
			//Aca habria que liberar el CPU, con FreeCPU(toBeMoved.cpuSocket)
			SetPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

		//Tras aviso desde CPU
		else if(PCPtask == PCP_TASK_BLOCK_DTB)
		{

			DTB* target = list_remove_by_condition(EXECqueue, (void*)IsToBeMoved);
			AddToBlocked(target);
			//Aca habria que liberar el CPU, con FreeCPU(toBeMoved.cpuSocket)
			SetPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

		//Tras aviso desde DAM
		else if(PCPtask == PCP_TASK_UNLOCK_DTB)
		{

			DTB* target = list_remove_by_condition(BLOCKEDqueue, (void*)IsToBeMoved);
			AddToReady(target);
			SetPCPTask(PCP_TASK_NORMAL_SCHEDULE);

		}

	}

}

void* FlattenPathsAndAddresses(t_dictionary* openFilesTable)
{

	void* result = malloc(1);
	int offset = 0, totalSize = 0;
	int nextSize;

	printf("Reserve el espacio\n");

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

	int* idToSend = malloc(sizeof(int));
	*idToSend = chosenDTB->id;
	int* pathAddressToSend = malloc(sizeof(int));
	*pathAddressToSend = chosenDTB->pathLogicalAddress;
	int* pcToSend = malloc(sizeof(int));
	*pcToSend = chosenDTB->programCounter;
	int* quantumToSend = malloc(sizeof(int));
	*quantumToSend = chosenDTB->quantumRemainder;
	//Cantidad de archivos abiertos
	int* ofaToSend = malloc(sizeof(int));
	*ofaToSend = chosenDTB->openedFilesAmount;

	//Estructuras con los datos a serializar y mandar como cadena
	SerializedPart idSP, pathSP, pathAddressSP, pcSP, quantumSP, ofaSP, filesSP;
	idSP.size = sizeof(idToSend);
	idSP.data = idToSend;
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
	//|IDdelDTB|PathEscriptorioAsociado|Dir.LogicaPath|ProgramCounterDelDTB|QuantumAEjecutar|CantArchivosAbiertos|Archivos
	//(cada cual con su respectivo tamanio antes del dato en si)
	//Los archivos se mandan como: "arch1:d1,arch2:d2,...,archN:dN;"
	void* message = Serialization_Serialize(7, idSP, pathSP, pathAddressSP, pcSP, quantumSP, ofaSP, filesSP);

	//Funcion anidada, para buscar el DTB con el mismo script que el DTB elegido, solo para el caso de estar mandando el Dummy
	bool IsDTBWithTheSamePath(DTB* aDTB)
	{
		if((strcmp(aDTB->pathEscriptorio, pathSP.data)) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	//Si estoy mandandole el Dummy (ID = 0), busco el DTB con el mismo script y lo guardo en mi variable global
	//Nadie deberia modificar este toBeCreated->dtbID hasta no hacer otra operacion asi (previo a ello
	//desbloqueando el Dummy y cerrando esta inicializacion). Guarda con eso
	if(*idToSend == 0)
	{
		DTB* wanted;
		wanted = list_find(NEWqueue, (void*)IsDTBWithTheSamePath);
		toBeCreated->dtbID = wanted->id;
	}

	return message;

}

void* ScheduleRR(int quantum)
{

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
	if((chosenDTB->quantumRemainder == 0) || ((chosenDTB->quantumRemainder == 0) > maxQuantum))
	{
		chosenDTB->quantumRemainder = maxQuantum;
	}

	//Obtengo la cadena a enviarle al CPU asignado; detalle de la misma dentro de la funcion
	void* packet = GetMessageForCPU(chosenDTB);

	AddToExec(chosenDTB);
	//OJO: Alguien deberia hacer free de ese packet despues
	return packet;

}

/////////////////////MOVER AL CPU////////////////////
t_dictionary* BuildDictionary(void* flattened, int amount)
{

	t_dictionary* dict = dictionary_create();
	int i = 1;
	int offset = 0;
	int* logicalAddress;
	while(i <= amount)
	{
		char* path;
		//OJO: NO VA EL FREE DE ESTE PUNTERO ACA, SINO PIERDO LA REFERENCIA DEL ULTIMO PUT
		//EL FREE SE HACE SOLO CUANDO DESTRUYA EL DICCIONARIO Y SUS ELEMENTOS
		logicalAddress = malloc(sizeof(int));
		path = strtok((char*)(flattened + offset), ":");
		//No le sumo uno por los :, strlen me devuelve el largo + 1 por el \0 al final
		offset += (strlen(path) + 1);
		memcpy(logicalAddress, flattened + offset, sizeof(int));
		offset += (sizeof(int) + 1);
		dictionary_put(dict, path, logicalAddress);
		i++;
	}

	return dict;

}

///PRUEBA BUILDDICTIONARY///

/*void ShowKeyAndValue(char* key, void* value)
{
	printf("%s : ", key);
	printf("%d\n",*((int*)value));
}

int main(void)
{

	void* aplanado = malloc(100);
	int offset = 0;
	int* dir1 = (int*)malloc(sizeof(int));
	*dir1 = 12560;
	int* dir2 = (int*)malloc(sizeof(int));
	*dir2 = 1310;
	int* dir3 = (int*)malloc(sizeof(int));
	*dir3 = 8826;
	memcpy(aplanado + offset, "script1.txt:", 12);
	offset += 12;
	memcpy(aplanado + offset, dir1, sizeof(int));
	offset += sizeof(int);
	memcpy(aplanado + offset, ",script2.txt:", 13);
	offset += 13;
	memcpy(aplanado + offset, dir2, sizeof(int));
	offset += sizeof(int);
	memcpy(aplanado + offset, ",otroArch.bat:", 14);
	offset += 14;
	memcpy(aplanado + offset, dir3, sizeof(int));
	offset += sizeof(int);
	memcpy(aplanado + offset, ";\0", 2);

	t_dictionary* d = BuildDictionary(aplanado, 3);

	dictionary_iterator(d, ShowKeyAndValue);

	return 0;

}*/
