#include "incs/CPUsManager.h"

t_list* cpus;

void InitCPUsHolder()
{
	cpus = list_create();
}

void AddCPU(int* socketID)
{

	CPU* newCPU;
	newCPU->socket = *socketID;
	newCPU->busy = false;
	list_add(cpus, newCPU);

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

void DestroyCPUsHolder()
{
	list_destroy(cpus);
}
