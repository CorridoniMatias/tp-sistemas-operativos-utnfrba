#ifndef SETTINGS_H_
#define SETTINGS_H_

///-------------INCLUSION DE BIBLIOTECAS-------------///

//------BIBLIOTECAS EXTERNAS------//

//------BIBLIOTECAS INTERNAS------//
#include "kemmens/config.h"
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

#define RUTA_CONFIG "SAFA.config"

///-------------VARIABLES GLOBALES-------------///

/*extern*/ Configuracion* settings;						//Almacena los datos de configuracion

///-------------FUNCIONES DEFINIDAS------------///

void configurar();
//ToDo: Funcion para reconfigurar ante un inotify()

#endif /* SETTINGS_H_ */
