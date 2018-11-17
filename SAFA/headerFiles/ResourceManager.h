#ifndef HEADERFILES_RESOURCEMANAGER_H_
#define HEADERFILES_RESOURCEMANAGER_H_

#include <commons/collections/dictionary.h>
#include "../headerFiles/Scheduling.h"

///-------------ESTRUCTURAS DEFINIDAS-------------///

/*
 * 	Estructura que sirve como value en la tabla/diccionario de recursos; el nombre del mismo es la key de la tabla
 * 	CAMPOS:
 * 		availables: Cantidad de instancias disponibles del recurso; cuando se crea, vale 1
 * 		waiters: Cola de enteros que representan a los IDs de los DTBs bloqueados a la espera de que alguna
 * 				 instancia se libere. Es importante mantener el orden para ir desbloqueando de a 1
 */
struct ResourceStatus_s
{
	int availables;
	t_queue* waiters;
} typedef ResourceStatus;

///-------------VARIABLES GLOBALES-------------///

t_dictionary* resources;						//Tabla de recursos; las keys son los nombres, los values son ResourceStatus

pthread_mutex_t tableMutex;						//Mutex para evitar concurrencia sobre la tabla de recursos

///-------------FUNCIONES DEFINIDAS------------///

/*
 * 	ACCION: Inicializa la tabla de recursos y su mutex; deberia llamarse al inicializar todas las variables globales
 */
void CreateResourcesTable();

/*
 * 	ACCION: Agrega un nuevo recurso a la tabla, con una instancia disponible
 * 	PARAMETROS:
 * 		name: Key para identificar al nuevo recurso en la tabla (recordar que es case-sensitive!)
 */
void AddNewResource(char* name);

/*
 * 	ACCION: Registra la accion de una operacion signal sobre un recurso; aumenta en uno la cantidad de instancias
 * 			libres del mismo (o lo crea si no existe), y mueve al primer bloqueado a READY
 * 	PARAMETROS:
 * 		name: Nombre del recurso a liberar, es la key en la tabla
 */
void SignalForResource(char* name);

/*
 * 	ACCION: Registra la accion de una operacion wait sobre un recurso; disminuye en uno la cantidad de instancias libres
 * 			del mismo (o lo crea si no existe, y lo asigna automaticamente); si no hay instancias disponibles, bloquea
 * 			al solicitante; devuelve true si puede asignar, false si no
 * 	PARAMETROS:
 * 		name: Nombre del recurso a retener, es la key en la tabla
 */
bool WaitForResource(char* name, int requesterID);

/*
 *	ACCION: Closure para liberar el espacio ocupado por un solicitante de la cola de bloqueados de un recurso
 *	PARAMETROS:
 *		waiterID: ID del solicitante, es pasado automaticamente por el queue_clean_and_destroy_elements
 */
void WaiterDestroyer(int* waiterID);

/*
 * 	ACCION: Closure para liberar el espacio ocupado por un registro de recurso de la tabla, y su cola de bloqueados
 * 	PARAMETROS:
 * 		rst: Recurso en forma de estructura, pasado automaticamente por la funcion de diccionarios
 */
void ResourceDestroyer(ResourceStatus* rst);

/*
 * 	ACCION: Libera la memoria ocupada por la tabla de recursos y por cada uno de ellos (los borra);
 * 			tambien destruye el mutex; deberia ir al final
 */
void DeleteResources();

#endif /* HEADERFILES_RESOURCEMANAGER_H_ */
