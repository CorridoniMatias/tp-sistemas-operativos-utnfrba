#include "incs/CPUsManager.h"

t_list* cpus;

void InitCPUsHolder()
{
	cpus = list_create();
}

void AddCPU(int* socketID)
{
	list_add(cpus, socketID);
}

void RemoveCPU(int socketID)
{
	int size = CPUsCount();
	int sock;
	for(int i = 0; i < size; i++)
	{
		sock = *((int*)list_get(cpus, i));

		if(sock == socketID)
			list_remove(cpus, i); //No hacemos el free porque el espacio de memoria que guardamos pertenece a SocketServer y es el quien hace el free de esa memoria!.
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
