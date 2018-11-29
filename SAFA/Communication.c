#include "headerFiles/Communication.h"

void Comms_DAM_DummyFinished(void* arriveData)
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

void Comms_DAM_AbrirFinished(void* arriveData)
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

void Comms_DAM_CrearBorrarFlushFinished(void* arriveData)
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

void Comms_DAM_IOError(void* arriveData)
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

void Comms_CPU_ErrorOrEOF(void* arriveData)
{

	OnArrivedData* data = (OnArrivedData*)arriveData;
	//Ahora necesito tanto el ID (que es el del DTB que estaba siendo dummizado) y el ID de la CPU a desalojar
	int cpuSocket = data->calling_SocketID;
	uint32_t* dtbID = (uint32_t*)(data->receivedData);

	//Libero la CPU y la pongo como desocupada
	FreeCPU(cpuSocket);

	//Agrego ese ID a la cola de DTBs a terminar, ya que bien no puedo leer mas o hubo un error con el FM9
	pthread_mutex_lock(&mutexToBeEnded);
	queue_push(toBeEnded, dtbID);
	pthread_mutex_unlock(&mutexToBeEnded);
	SetPCPTask(PCP_TASK_END_DTB);

	//No hago free de dtbID, sino borraria el valor de lo que tiene el nextToUnlock que acabo de guardar
	free(data->receivedData);
	free(arriveData);

}

void Comms_CPU_DummyAtDAM(void* arriveData)
{

	OnArrivedData* data = (OnArrivedData*)arriveData;
	//Necesito solo el ID del CPU a desalojar; el DTB ya se que es el Dummy
	int cpuSocket = data->calling_SocketID;

	//Libero la CPU y la pongo como desocupada
	FreeCPU(cpuSocket);

	//Le aviso al PCP que debe liberar el Dummy (ponerlo de nuevo en BLOCKED para poder ser usado)
	SetPCPTask(PCP_TASK_FREE_DUMMY);

	free(data->receivedData);
	free(arriveData);

}

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
		//No le sumo uno por los :, strlen me devuelve el largo (sin el \0 al final); le sumo uno por los : separadores
		offset += (strlen(path) + 1);
		memcpy(logicalAddress, flattened + offset, sizeof(int));
		offset += (sizeof(int) + 1);
		dictionary_putMAESTRO(dict, path, logicalAddress, LogicalAddressDestroyer);
		i++;
	}

	return dict;

}

void Comms_CPU_DTBAtDAM(void* arriveData)
{

	OnArrivedData* data = (OnArrivedData*)arriveData;
	//Uso un DeserializedData, el CPU me mando mucha informacion
	int cpuSocket = data->calling_SocketID;
	DeserializedData* params = Serialization_Deserialize(data->receivedData);

	//Creo un BlockableInfo con la info del DTB a mover a BLOCKED
	//El formato de la cadena es IdDTB|ProgramCounter|Quantum|CantArchivosAbiertos|Archivos
	BlockableInfo* nextToBlock = (BlockableInfo*) malloc(sizeof(BlockableInfo));
	nextToBlock->id = *((uint32_t*)(params->parts[0]));
	nextToBlock->newProgramCounter = *((uint32_t*)(params->parts[1]));
	nextToBlock->quantumRemainder = *((uint32_t*)(params->parts[2]));
	nextToBlock->dummyComeback = false;
	uint32_t ofa = *((uint32_t*)(params->parts[3]));
	nextToBlock->openedFilesUpdate = BuildDictionary(params->parts[4], ofa);

	//Libero la CPU que me aviso y la pongo como desocupada
	FreeCPU(cpuSocket);

	//Meto ese struct a la cola de DTBs a desbloquear, cuidando la mutua exclusion; cambio la tarea de PCP
	pthread_mutex_lock(&mutexToBeBlocked);
	queue_push(toBeBlocked, nextToBlock);
	pthread_mutex_unlock(&mutexToBeBlocked);
	SetPCPTask(PCP_TASK_BLOCK_DTB);

	//Libero la memoria del DeserializedData entero, ya use lo que tenia y les hice copias fieles
	//No hay problema con params->parts[4], ya que lo fui recorriendo para armar el diccionario
	Serialization_CleanupDeserializationStruct(params);
	free(data->receivedData);
	free(arriveData);

}

void Comms_CPU_OutOfQuantum(void* arriveData)
{

	OnArrivedData* data = (OnArrivedData*)arriveData;
	//Uso un DeserializedData, el CPU me mando mucha informacion
	int cpuSocket = data->calling_SocketID;
	DeserializedData* params = Serialization_Deserialize(data->receivedData);

	//Creo un BlockableInfo con la info del DTB a mover a BLOCKED
	//El formato de la cadena es IdDTB|ProgramCounter|CantArchivosAbiertos|Archivos
	UnlockableInfo* nextToUnlock = (UnlockableInfo*) malloc(sizeof(UnlockableInfo));
	nextToUnlock->id = *((uint32_t*)(params->parts[0]));
	nextToUnlock->newProgramCounter = *((uint32_t*)(params->parts[1]));
	//El flag va en false, quiero que pise cualquier diccionario que haya con estos datos
	nextToUnlock->singleOFaddition = false;
	uint32_t ofa = *((uint32_t*)(params->parts[2]));
	nextToUnlock->openedFilesUpdate = BuildDictionary(params->parts[3], ofa);

	//Libero la CPU que me aviso y la pongo como desocupada
	FreeCPU(cpuSocket);

	//Meto ese struct a la cola de DTBs a desbloquear, cuidando la mutua exclusion; cambio la tarea de PCP
	pthread_mutex_lock(&mutexToBeUnlocked);
	queue_push(toBeUnlocked, nextToUnlock);
	pthread_mutex_unlock(&mutexToBeUnlocked);
	SetPCPTask(PCP_TASK_UNLOCK_DTB);

	//Libero la memoria del DeserializedData entero, ya use lo que tenia y les hice copias fieles
	//No hay problema con params->parts[4], ya que lo fui recorriendo para armar el diccionario
	Serialization_CleanupDeserializationStruct(params);
	free(data->receivedData);
	free(arriveData);

}

void Comms_CPU_WaitResource(void* arriveData)
{

	OnArrivedData* data = (OnArrivedData*)arriveData;
	//Uso un DeserializedData, el CPU me mando mucha informacion
	int cpuSocket = data->calling_SocketID;
	DeserializedData* params = Serialization_Deserialize(data->receivedData);

	//Me fijo si el DTB solicitante puede tomar ese recurso o no, y veo que le contesto
	bool success = WaitForResource((char*)(params->parts[1]), *((uint32_t*)(params->parts[0])));
	uint32_t* result = (uint32_t*) malloc(sizeof(uint32_t));

	//Le voy a mandar un 1 si puede tomar el recurso, o un 0 si no y debe bloquearse,
	//lo cual lo va a llevar a mandarme otro mensaje con DTB_BLOCK
	if(success)
	{
		*result = 1;
	}
	else
	{
		*result = 0;
	}

	//Le respondo al CPU que me hablo, mandandole el resultado de la operacion
	SocketCommons_SendData(cpuSocket, MESSAGETYPE_SAFA_CPU_WAITRESPONSE, result, sizeof(uint32_t));

	//No libero todo el DeserializedData, necesito conservar el nombre del recurso por el Dictionary
	multiFree(3, params->parts[0], params->parts, params);
	//TODO: free(result) haria falta??
	free(data->receivedData);
	free(arriveData);

}

void Comms_CPU_SignalResource(void* arriveData)
{

	OnArrivedData* data = (OnArrivedData*)arriveData;
	DeserializedData* params = Serialization_Deserialize(data->receivedData);

	//Solo necesito el nombre del recurso; el ResourceManager prueba hacer el signal
	//No hace falta responderle nada al CPU, el seguira con su ejecucion
	SignalForResource((char*)(params->parts[1]));

	//No libero el DeserializedData entero, necesito conservar el nombre del recurso por el Dictionary
	multiFree(3, params->parts[0], params->parts, params);
	free(data->receivedData);
	free(arriveData);


}
