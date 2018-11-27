#ifndef SETTINGS_H_
#define SETTINGS_H_

///-------------INCLUSION DE BIBLIOTECAS-------------///

//------BIBLIOTECAS EXTERNAS------//
#include "unistd.h"
#include "sys/inotify.h"

//------BIBLIOTECAS INTERNAS------//
#include "kemmens/config.h"
#include "kemmens/logger.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "kemmens/SocketClient.h"

//------BIBLIOTECAS PROPIAS------//


///-------------ESTRUCTURAS DEFINIDAS-------------///

/*
 * 	Estructura para guardar los datos de configuracion del SAFA, levantados del SAFA.config
 * 	CAMPOS:
 * 		puerto: Puerto de escucha de conexiones (el SAFA es servidor universal)
 * 		algoritmo: Nombre del algoritmo de planificacion del PCP; puede ser "RR", "VRR" o "PROPIO" (IOBF)
 * 		quantum: Quantum maximo de ejecucion que puede llevar a cabo un DTB; sirve para todos los algoritmos
 * 		multiprogramacion: Grado de multiprogramacion del sistema (cuantos procesos puede haber en memoria)
 * 		retardo: Retardo, en milisegundos, que sufre cada ciclo de planificacion (del PCP?)
 */
struct Configuracion_s
{
	int puerto;
	char* algoritmo;
	int quantum;
	int multiprogramacion;
	int retardo;

} typedef Configuracion;


///-------------CONSTANTES DEFINIDAS-------------///

#define RUTA_CONFIG "Config/SAFA.config"
#define RUTA_WATCHABLE "Config"

//Para los eventos de inotify; maximo numero de cambios a registrar, largo del nombre del archivo, tamanios de evento y buffer
#define MAX_CHANGES_ALLOWED 1024
#define FILE_LENGTH 		14
#define EVENT_SIZE 			( sizeof (struct inotify_event) )
#define BUFFER_SIZE			( MAX_CHANGES_ALLOWED * ( EVENT_SIZE + FILE_LENGTH ))


///-------------VARIABLES GLOBALES-------------///

/*extern*/ Configuracion* settings;						//Almacena los datos de configuracion


///-------------FUNCIONES DEFINIDAS------------///

/*
 * 	ACCION: Mallocear la estructura de settings y cargarla con los datos del archivo de configuracion
 * 			Debe llamarse a esta funcion al arrancar el programa, antes que nada
 */
void Configurar();

/*
 * 	ACCION: Actualizar los datos de las variables de configuracion (excepto el puerto), sin mallocear
 * 			Esta funcion es llamada desde el monitor del archivo, ejecutado en un thread
 */
void UpdateSettings();

/*
 * 	ACCION: Monitorear el archivo de SAFA.config ante eventuales cambios en su contenido
 * 			Esta funcion debe ser ejecutada en un hilo aparte, posee el read que es bloqueante
 */
void MonitorConfigFile();

#endif /* SETTINGS_H_ */
