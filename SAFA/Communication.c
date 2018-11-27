#include "headerFiles/Communication.h"

void Comms_DummyFinished(void* arriveData)
{

	//Obtengo los datos del parametro (automatico, del runnable) y los deserializo
	OnArrivedData* data = (OnArrivedData*)arriveData;
	DeserializedData* params = Serialization_Deserialize(data->receivedData);

	//Copio esos datos al justDummied, la estructura que usa Scheduling para saber como actualizar el DTB
	justDummied->dtbID = *((uint32_t*)(params->parts[0]));
	int pathLength = strlen((char*)(params->parts[1])) + 1;
	justDummied = realloc(justDummied, (2 * sizeof(uint32_t)) + pathLength);	//Importante reallocar!!
	strcpy(justDummied->script, (char*)(params->parts[1]));
	justDummied->logicalAddress = *((uint32_t*)(params->parts[1]));

	//Seteo la tarea del PLP en inicializar el DTB (cargar datos tras operacion Dummy), y le aviso
	SetPLPTask(PLP_TASK_INITIALIZE_DTB);

	//Libero la memoria del struct de deserializacion y los parametros de la funcion
	Serialization_CleanupDeserializationStruct(params);
	free(data->receivedData);
	free(arriveData);

}

void Comms_AbrirFinished(void* arriveData)
{

	OnArrivedData* data = (OnArrivedData*)arriveData;
	DeserializedData* params = Serialization_Deserialize(data->receivedData);

	//Cargo los datos necesarios en un struct de info para desbloquear de DTBs
	UnlockableInfo* nextToUnlock = (UnlockableInfo*) malloc(sizeof(UnlockableInfo));
	nextToUnlock->id = *((uint32_t*)(params->parts[0]));
	//Lo pongo en -1 para que sepa que no debe copiarlo al mover el DTB de nuevo a READY
	nextToUnlock->newProgramCounter = -1;
	nextToUnlock->singleOFaddition = true;
	nextToUnlock->openedFilesUpdate = dictionary_create();
	dictionary_putMAESTRO(nextToUnlock->openedFilesUpdate, (char*)(params->parts[1]), (uint32_t*)(params->parts[2]), LogicalAddressDestroyer);

	//Meto ese struct a la cola de DTBs a desbloquear, cuidando la mutua exclusion; cambio la tarea de PCP
	pthread_mutex_lock(&mutexToBeUnlocked);
	queue_push(toBeUnlocked, nextToUnlock);
	pthread_mutex_unlock(&mutexToBeUnlocked);
	SetPCPTask(PCP_TASK_UNLOCK_DTB);

	//Cleanup casero de la deserialization struct; no libero la memoria de parts[2], sino pierdo la address
	//que sirve como value en el diccionario; mas adelante hare el free de ese diccionario, no hay drama
	multiFree(4, params->parts[0], params->parts[1], params->parts, params);
	free(data->receivedData);
	free(arriveData);

}

void Comms_CrearBorrarFlushFinished(void* arriveData)
{

	OnArrivedData* data = (OnArrivedData*)arriveData;
	uint32_t* dtbID = (uint32_t*)(data->receivedData);

	//Creo el struct a meter en la cola; solo le pongo el ID; el PC no importa, el diccionario
	//va vacio; pongo el flag en true para que no borre el diccionario ya existente
	UnlockableInfo* nextToUnlock = (UnlockableInfo*) malloc(sizeof(UnlockableInfo));
	nextToUnlock->id = *dtbID;
	nextToUnlock->newProgramCounter = -1;
	nextToUnlock->singleOFaddition = true;
	nextToUnlock->openedFilesUpdate = dictionary_create();

	//Meto ese struct a la cola de DTBs a desbloquear, cuidando la mutua exclusion; cambio la tarea de PCP
	pthread_mutex_lock(&mutexToBeUnlocked);
	queue_push(toBeUnlocked, nextToUnlock);
	pthread_mutex_unlock(&mutexToBeUnlocked);
	SetPCPTask(PCP_TASK_UNLOCK_DTB);

	//No hago free de dtbID, sino borraria el valor de lo que tiene el nextToUnlock que acabo de guardar
	free(data->receivedData);
	free(arriveData);

}

void Comms_KillDTBRequest(void* arriveData)
{

	OnArrivedData* data = (OnArrivedData*)arriveData;
	uint32_t* dtbID = (uint32_t*)(data->receivedData);

	//Agrego ese ID a la cola de DTBs a terminar, ya que un error en la IO implica un aborto inmediato
	pthread_mutex_lock(&mutexToBeEnded);
	queue_push(toBeEnded, dtbID);
	pthread_mutex_unlock(&mutexToBeEnded);
	SetPCPTask(PCP_TASK_END_DTB);

	//No hago free de dtbID, sino borraria el valor de lo que tiene el nextToUnlock que acabo de guardar
	free(data->receivedData);
	free(arriveData);

}

void Comms_DummyAtDAM(void* arriveData)
{

	OnArrivedData* data = (OnArrivedData*)arriveData;
	//Ahora necesito tanto el ID (que es el del DTB que estaba siendo dummizado) y el ID de la CPU a desalojar
	int cpuSocket = data->calling_SocketID;
	uint32_t* dtbID = (uint32_t*)(data->receivedData);

	//Idem al CrearBorrarFlush, es crear UnlockableInfo con solamente un ID (que lo tenia el Dummy, asi lo desocupo)
	//La diferencia es que aca hay otros parametros para poner con baura; es vuelta de dummy, flag va true
	BlockableInfo* nextToBlock = (BlockableInfo*) malloc(sizeof(BlockableInfo));
	nextToBlock->id = *dtbID;
	nextToBlock->newProgramCounter = -1;
	nextToBlock->quantumRemainder = -1;
	nextToBlock->dummyComeback = true;
	nextToBlock->openedFilesUpdate = dictionary_create();

	//Libero la CPU y la pongo como desocupada
	FreeCPU(cpuSocket);

	//Meto ese struct a la cola de DTBs a desbloquear, cuidando la mutua exclusion; cambio la tarea de PCP
	pthread_mutex_lock(&mutexToBeBlocked);
	queue_push(toBeUnlocked, nextToBlock);
	pthread_mutex_unlock(&mutexToBeBlocked);
	SetPCPTask(PCP_TASK_UNLOCK_DTB);

	//No hago free de dtbID, sino borraria el valor de lo que tiene el nextToUnlock que acabo de guardar
	free(data->receivedData);
	free(arriveData);

}
