#include "headerFiles/CPUsManager.h"

void InitCPUsHolder()
{
	cpus = list_create();
	pthread_mutex_init(&mutexCPUs, NULL);
}

void AddCPU(int* socketID)
{

	CPU* newCPU = (CPU*) malloc(sizeof(CPU));		//free no hace falta aca, va con el destroyer
	newCPU->socket = *socketID;
	newCPU->busy = false;
	pthread_mutex_lock(&mutexCPUs);
	list_add(cpus, newCPU);
	pthread_mutex_unlock(&mutexCPUs);

}

void FreeCPU(int socketID)
{

	int size = CPUsCount();
	CPU* counter;
	CPU* toModify;
	pthread_mutex_lock(&mutexCPUs);
	for(int i = 0; i < size; i++)
	{
		counter = list_get(cpus, i);
		if(counter->socket == socketID)
		{
			toModify = list_remove(cpus, i);		//Saco el CPU buscado de la lista, lo marco como desocupado,
			toModify->busy = false;					//y lo vuelvo a poner en la lista de CPUs
			list_add_in_index(cpus, i, toModify);
			break;			   	  //No hace falta seguir buscando otro con ese ID, salgo del ciclo
		}
	}
	pthread_mutex_unlock(&mutexCPUs);

}

int RemoveCPU(int socketID)
{

	int size = CPUsCount();
	CPU* counter;
	for(int i = 0; i < size; i++)
	{
		counter = list_get(cpus, i);
		if(counter->socket == socketID)
		{
			list_remove(cpus, i); //No hacemos el free porque el espacio de memoria que guardamos pertenece a SocketServer y es el quien hace el free de esa memoria!.
			return 1;			  //Si encontre un CPU con ese socket y lo removi, devuelvo 1 (para el OnDisconnect)
		}
	}

	return 0;					  //Si no encontre, devuelvo un 0 para que no informe nada

}

int CPUsCount()
{

	return list_size(cpus);

}

bool IsIdle(void* myCPU)
{

	CPU* realCPU = (CPU*) myCPU;
	if(realCPU->busy == false)
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool ExistsIdleCPU()
{

	bool exists;
	pthread_mutex_lock(&mutexCPUs);
	exists = list_any_satisfy(cpus, IsIdle);
	pthread_mutex_unlock(&mutexCPUs);
	return exists;

}

void CPUDestroyer(void* aCPU)
{

	free(aCPU);

}

void DestroyCPUsHolder()
{

	list_destroy_and_destroy_elements(cpus, CPUDestroyer);
	pthread_mutex_destroy(&mutexCPUs);

}
