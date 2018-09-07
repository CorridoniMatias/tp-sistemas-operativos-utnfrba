#ifndef BIBLIOTECADAM_H_
#define BIBLIOTECADAM_H_

#include "kemmens/config.h"
#include "kemmens/ThreadPool.h"
#include "kemmens/CommandInterpreter.h"
#include "kemmens/SocketClient.h"
#include "kemmens/SocketCommons.h"
#include "kemmens/SocketServer.h"

//-------------CONSTANTES PREDEFINIDAS-------------//

#define RUTA_CONFIG "DAM.config"				//Ruta de acceso al archivo de configuracion
#define TAMMAXPUERTO 6							//Tamanio maximo de un nombre de puerto (contando \0)
#define TAMMAXIP 20								//Tamanio maximo de un nombre de ip (contando \0)


//-------------ESTRUCTURAS Y VARIABLES GLOBALES-------------//

/*
 * Estructura para almacenar los datos de configuracion
 * CAMPOS: A documentar...
 */

struct Configuracion_s
{
	char puertoEscucha[TAMMAXPUERTO];
	char ipSAFA[TAMMAXIP];
	char puertoSAFA[TAMMAXPUERTO];
	char ipMDJ[TAMMAXIP];
	char puertoMDJ[TAMMAXPUERTO];
	char ipFM9[TAMMAXIP];
	char puertoFM9[TAMMAXPUERTO];
	int transferSize;
} typedef Configuracion;

t_list* cpus;					//Lista de CPUs conectadas
Configuracion* settings;
ThreadPool* pozoDeHebras;		//"ThreadPool" para encolar tareas e hilos


//-------------FUNCIONES-------------//

/*
 * 	ACCION: Guarda los datos del archivo de configuracion en la estructura global
 */
void configurar();

/*
 * 	ACCION: Configura segun el archivo, inicializa el CommandInterpreter, y crea el ThreadPool
 */
void inicializarVariablesGlobales();

/*
 * 	ACCION: Libera la memoria ocupada por la configuracion y el pozo
 */
void liberarVariablesGlobales();

/*
 * 	ACCION: Conectarse como cliente a un proceso y loguearlo; mandar un "iam ElDiego" como handshake
 * 	PARAMETROS:
 * 		ip:	IP del proceso al cual me quiero conectar
 * 		puerto: Puerto del proceso al cual me quiero conectar
 * 		nombreProceso: Nombre informal del proceso, para loguearlo
 */
int conectarAProceso(char* ip, char* puerto, char* nombreProceso);

/*
 * 	ACCION: Funcion threadeable para quedarme esperando un mensaje como respuesta
 * 	PARAMETROS:
 * 		socket: Socket a traves del cual espero la respuesta
 */
void esperarRespuesta(void* socket);

/*
 * 	ACCION: Levanta un servidor, lo pone a escuchar, y registra los listeners a hacer segun las acciones;
 * 			define el comportamiento al tener que encolar trabajos en el pozo
 */
void levantarServidor();

/*
 * 	ACCION: Funcion a realizar ante la lectura del comando iam, para el Command Interpreter
 * 	PARAMETROS: A documentar...
 */
void* comandoIAm (int argc, char** args, char* comando, void* datos);

void clienteConectado(int socket);

void clienteDesconectado(int unSocket);

void llegoUnPaquete(int socket, int tipoMensaje, void* datos);

void* aRealizar(char* cmd, char* sep, void* args, bool fired);

#endif /* BIBLIOTECADAM_H_ */
