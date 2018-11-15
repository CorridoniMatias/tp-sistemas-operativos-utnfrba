#include "headerFiles/ResourceManager.h"

void CreateResourcesTable()
{

	resources = dictionary_create();

}

void AddNewResource(char* name)
{

	ResourceStatus* rst;
	rst = (ResourceStatus*) malloc(sizeof(ResourceStatus));
	//Por defecto, cuando lo creo tiene una sola instancia
	rst->availables = 1;
	//Creo la cola, vacia
	rst->waiters = queue_create();
	dictionary_put(resources, name, rst);

}

bool SignalForResource(char* name)
{

	//Si el recurso ya existe, aumento en uno sus instancias disponibles; obtengo sus datos y actualizo
	if(dictionary_has_key(resources, name))
	{
		ResourceStatus* signaled = (ResourceStatus*) dictionary_get(resources, name);
		signaled->availables++;
		dictionary_put(resources, name, signaled);
	}
	//Si no, creo uno nuevo
	else
	{
		AddNewResource(name);
	}
	//ToDo: Revisar como quedo el availables del recurso, y ver si avisar al primero de la cola
	//En caso de ser asi, deberia avisarle al PCP que lo mueva de BLOCKED a EXEC

}

bool WaitForResource(char* name, int requesterID)
{

	//Si el recurso no existe, lo creo y le pongo una instancia (la cual luego asignare)
	if(!dictionary_has_key(resources, name))
	{
		AddNewResource(name);
	}

	//Obtengo los datos de ese recurso, y disminuyo en uno sus instancias libres
	ResourceStatus* waited = (ResourceStatus*) dictionary_get(resources, name);
	waited->availables--;

	//Si la cantidad disponible quedo negativa, es porque no habia instancias libres; lo agrego a la cola de bloqueados
	if(waited->availables < 0)
	{
		int* newWaiter = (int*) malloc(sizeof(int));
		*newWaiter = requesterID;
		queue_push(waited->waiters, newWaiter);
		//Actualizo mis registros de los recursos
		dictionary_put(resources, name, waited);
		//Respuesta positiva
		return true;
	}
	//Si no es asi, puedo asignarlo sin problemas!
	else
	{
		//Respuesta positiva
		return false;
	}

}
