#include "headerFiles/CPUsManager.h"

void InitCPUsHolder()
{
	cpus = list_create();
}

void AddCPU(int* socketID)
{

	CPU* newCPU = (CPU*) malloc(sizeof(CPU));		//free no hace falta aca, va con el destroyer
	newCPU->socket = *socketID;
	newCPU->busy = false;
	list_add(cpus, newCPU);

}

void FreeCPU(int socketID)
{

	int size = CPUsCount();
	CPU* counter;
	CPU* toModify;
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

}

void RemoveCPU(int socketID)
{

	int size = CPUsCount();
	CPU* counter;
	for(int i = 0; i < size; i++)
	{
		counter = list_get(cpus, i);
		if(counter->socket == socketID)
		{
			list_remove(cpus, i); //No hacemos el free porque el espacio de memoria que guardamos pertenece a SocketServer y es el quien hace el free de esa memoria!.
			break;			   	  //No hace falta seguir buscando otro con ese ID, salgo del ciclo
		}
	}

}

int CPUsCount()
{

	return list_size(cpus);

}

void CPUDestroyer(void* aCPU)
{

	free(aCPU);

}

void DestroyCPUsHolder()
{

	list_destroy_and_destroy_elements(cpus, CPUDestroyer);

}

bool IsIdle(CPU* myCPU)
{

	if(myCPU->busy == false)
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
	exists = list_any_satisfy(cpus, (void*)IsIdle);
	return exists;

}
