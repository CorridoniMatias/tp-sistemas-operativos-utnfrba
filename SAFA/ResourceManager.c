#include "headerFiles/ResourceManager.h"

void CreateResourcesTable()
{

	resources = dictionary_create();
	pthread_mutex_init(&tableMutex, NULL);

}

//NOTA: Antes de cada consulta/update al diccionario, deberia hacer lock y unlock del mutex (region critica)

void AddNewResource(char* name)
{

	ResourceStatus* rst;
	rst = (ResourceStatus*) malloc(sizeof(ResourceStatus));
	//Por defecto, cuando lo creo tiene una sola instancia
	rst->availables = 1;
	//Creo la cola, vacia
	rst->waiters = queue_create();
	pthread_mutex_lock(&tableMutex);
	dictionary_putMAESTRO(resources, name, rst, ResourceDestroyer);
	pthread_mutex_unlock(&tableMutex);
	Logger_Log(LOG_INFO, "SAFA::RESOURCES->Creado el recurso %s. Recursos existentes: %d", name, dictionary_size(resources));

}

void SignalForResource(char* name, uint32_t requesterID)
{
//	printf("\n\nse entro al signal\n\n");
	//Closure para encontrar el DTB que solicita el recurso entre los DTBs de EXEC
	bool IsRequesterDTB(void* aDTB)
	{
		DTB* realDTB = (DTB*) aDTB;
		if(realDTB->id == requesterID)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	//Closure para hallar el recurso involucrado en la lista de recursos tomados de un DTB
	bool IsInvolvedResource(void* res)
	{
		char* realRes = (char*) res;
		if(strcmp(realRes, name) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	//Si el recurso ya existe, aumento en uno sus instancias disponibles; obtengo sus datos y actualizo
	pthread_mutex_lock(&tableMutex);
//	printf("\n\nname =\"%s\"\n",name);
	if(dictionary_has_key(resources, name))
	{
		//Puntero a los datos del recurso al que se le hizo signal
		ResourceStatus* signaled = (ResourceStatus*) dictionary_get(resources, name);
		signaled->availables++;
		pthread_mutex_unlock(&tableMutex);
		Logger_Log(LOG_DEBUG, "SAFA::RESOURCES->Se registro signal sobre recurso %s", name);
		Logger_Log(LOG_DEBUG, "SAFA::RESOURCES->Instancias libres: %d. Procesos esperando: %d", signaled->availables, queue_size(signaled->waiters));
//		printf("\n\nvalor actual del contador %d\n\n\n", signaled->availables);
//		dictionary_putMAESTRO(resources, name, signaled, ResourceDestroyer);
	}
	//Si no, creo uno nuevo
	else
	{
		pthread_mutex_unlock(&tableMutex);
		AddNewResource(name);
	}

	//Si fuera 0 su ID (no por el Dummy, es mas por valor default) no intento tocar sus recursos en su estructura propia
	if(requesterID != 0)
	{
		pthread_mutex_lock(&mutexEXEC);
		//No me fijo que sea Null, ya deberia estar aca
		DTB* requester = list_find(EXECqueue, IsRequesterDTB);
		pthread_mutex_unlock(&mutexEXEC);
		if(requester == NULL){
			requester = list_find(BLOCKEDqueue, IsRequesterDTB);
		}
		if (requester == NULL) {
			if ((strcmp(settings->algoritmo, "RR")) == 0) {
				requester = list_find(READYqueue_Own, IsRequesterDTB);
			} else if ((strcmp(settings->algoritmo, "VRR")) == 0) {
				requester = list_find(READYqueue_RR->elements, IsRequesterDTB);
			} else if ((strcmp(settings->algoritmo, "PROPIO")) == 0) {
				requester = list_find(READYqueue_VRR, IsRequesterDTB);
			}
		}

		char* rName = list_find(requester->resourcesKept, IsInvolvedResource);
		//Solo si ese recurso esta entre los tomados por el DTB involucrado, lo remuevo de esa lista
		if(rName)
		{
			list_remove_by_condition(requester->resourcesKept, IsInvolvedResource);
		}
	}

	pthread_mutex_lock(&tableMutex);
	//Otro puntero, apunta a los datos del recurso al que se le hizo signal
	ResourceStatus* involved = dictionary_get(resources, name);
	//Si el recurso tenia DTBs esperando a que se libere, debo desbloquear el primero y darle la instancia
	if(!queue_is_empty(involved->waiters))
	{
		uint32_t* firstWaiter = malloc(sizeof(uint32_t));
		//Bajo la cantidad de instancias disponibles, ya que recien ahora habria podido hacer el wait
//		involved->availables--;
		//Me guardo el ID del primer DTB que lo estaba esperando
		*firstWaiter = *((uint32_t*) queue_pop(involved->waiters));
		pthread_mutex_unlock(&tableMutex);
		//Actualizo los datos del recurso en el diccionario, hago un put sobre la misma key
//		dictionary_putMAESTRO(resources, name, involved, ResourceDestroyer);
		//Creo la estructura a meter en la cola; no modifica el PC del DTB (va en -1) ni debe registrar cambios en los archivos abiertos
		UnlockableInfo* toBeAwakened = (UnlockableInfo*) malloc(sizeof(UnlockableInfo));
		toBeAwakened->id = *firstWaiter;
		toBeAwakened->newProgramCounter = -1;
		toBeAwakened->openedFilesUpdate = dictionary_create();
		toBeAwakened->overwritePC = false;
		toBeAwakened->appendOFs = true;
		bool toUnlockDTB(void* data){
			DTB* dtb = data;
			return dtb->id == toBeAwakened->id;
		}

//		printf("\n\n id a buscar=%d",toBeAwakened->id);
		DTB* blockedDTB = list_find(BLOCKEDqueue,toUnlockDTB);
		if(!blockedDTB){
			if ((strcmp(settings->algoritmo, "RR")) == 0) {
				blockedDTB = list_find(READYqueue_Own, toUnlockDTB);
			} else if ((strcmp(settings->algoritmo, "VRR")) == 0) {
				blockedDTB = list_find(READYqueue_RR->elements, toUnlockDTB);
			} else if ((strcmp(settings->algoritmo, "PROPIO")) == 0) {
				blockedDTB = list_find(READYqueue_VRR, toUnlockDTB);
			}
		}
		if(!blockedDTB){
			blockedDTB = list_find(EXECqueue,toUnlockDTB);
		}
//		if (!blockedDTB) {
//			printf("\n\nno estaba en en ready\n\n");
//			blockedDTB = list_get(READYqueue_Own, toUnlockDTB);
//		}
//		printf("\n\nname =\"%s\"\n\n",name);
//		printf("\n\nblocked id =%d",blockedDTB->id);
		char* resource = string_duplicate(name);
		list_add(blockedDTB->resourcesKept, resource);
		//Agrego el DTB a desbloquear a la cola de DTBs a mover (mutex mediante) de nuevo a READY; y le aviso al PCP
		pthread_mutex_lock(&mutexToBeUnlocked);
		queue_push(toBeUnlocked, toBeAwakened);
		Logger_Log(LOG_DEBUG, "SAFA::RESOURCES->Se aviso al PCP que desbloquee al DTB de id = %d", toBeAwakened->id);
		pthread_mutex_unlock(&mutexToBeUnlocked);
		AddPCPTask(PCP_TASK_UNLOCK_DTB);
	}
	pthread_mutex_unlock(&tableMutex);
	//NO hago free de signaled ni de involved, salieron de un get (no hay malloc)
}

bool WaitForResource(char* name, uint32_t requesterID)
{

	//Closure para encontrar el DTB que solicita el recurso entre los DTBs de EXEC
	bool IsRequesterDTB(void* aDTB)
	{
		DTB* realDTB = (DTB*) aDTB;
		if(realDTB->id == requesterID)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

//	printf("\n\nse entro al waitforResource\n\n");
	pthread_mutex_lock(&tableMutex);

//	printf("\n\nlock tablemutex adquirido\n\n");
	//Si el recurso no existe, lo creo y le pongo una instancia (la cual luego asignare)
	if(!dictionary_has_key(resources, name))
	{
		pthread_mutex_unlock(&tableMutex);
//		printf("\n\nlock tablemutex liberado\n\n");
		AddNewResource(name);
	}
	else
		pthread_mutex_unlock(&tableMutex);

//	printf("\n\n pasado por addnewResourse\n\n");

	pthread_mutex_lock(&tableMutex);
//	printf("\n\nlock tablemutex adquirido nuevamente\n\n");
	//Obtengo los datos de ese recurso, y disminuyo en uno sus instancias libres
	ResourceStatus* waited = dictionary_get(resources, name);
	waited->availables--;
	Logger_Log(LOG_DEBUG, "SAFA::RESOURCES->Se registro wait sobre recurso %s", name);
	Logger_Log(LOG_DEBUG, "SAFA::RESOURCES->Instancias libres: %d. Procesos esperando: %d", waited->availables, queue_size(waited->waiters));
	//Si la cantidad disponible quedo negativa, es porque no habia instancias libres; lo agrego a la cola de bloqueados
	if(waited->availables < 0)
	{
		pthread_mutex_unlock(&tableMutex);
		//Respuesta negativa
		return false;
	}
	//Si no es asi, puedo asignarlo sin problemas!
	else
	{
		pthread_mutex_unlock(&tableMutex);
		//Respuesta positiva
		pthread_mutex_lock(&mutexEXEC);
		//No me fijo que sea Null, ya deberia estar aca
		DTB* requester = (DTB*) list_find(EXECqueue, IsRequesterDTB);
		list_add(requester->resourcesKept, name);
		pthread_mutex_unlock(&mutexEXEC);
		return true;
	}

}

void WaiterDestroyer(void* waiterID)
{

	free(waiterID);

}

void ResourceDestroyer(void* r)
{

	ResourceStatus* rscInfo = (ResourceStatus*) r;
	queue_clean_and_destroy_elements(rscInfo->waiters, WaiterDestroyer);
	//Recordar: el free principal es sobre el parametro; el casteado se liberara al retornar de esta funcion
	free(r);

}

void DeleteResources()
{

	dictionary_destroy_and_destroy_elements(resources, ResourceDestroyer);
	pthread_mutex_destroy(&tableMutex);

}
