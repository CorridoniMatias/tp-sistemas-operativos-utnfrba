#include "kemmens/megekemmen.h"
#include "kemmens/logger.h"
#include "bibliotecaDAM.h"

int main(int argc, char **argv)
{
	Logger_CreateLog("./DAM.log", "DAM", true);
	inicializarVariablesGlobales();
	Logger_Log(LOG_INFO, "Proceso DAM iniciado; escuchando en el puerto %d", settings->puertoEscucha);
	printf("Loggee lo primero LOL\n");

	int socketSAFA, socketFM9, socketMDJ; 		//Sockets para conectarme a los procesos, como CLIENTE

	pthread_t hebraSAFA, hebraFM9, hebraMDJ;	//Hebras para manejar la comunicacion a traves de cada socket en paralelo

	//Me conecto a SAFA, FM9 y MDJ, y levanto el servidor de CPUs
	socketSAFA = conectarAProceso(/*settings->ipSAFA, */settings->puertoSAFA, "S-AFA");
	socketFM9 = conectarAProceso(/*settings->ipFM9, */settings->puertoFM9, "FM9");
	socketMDJ = conectarAProceso(/*settings->ipMDJ, */settings->puertoMDJ, "MDJ");
	levantarServidor();

	//Levanto la funcion esperarRespuesta en tres hilos distintos, uno por socket; TEMPORAL, solo para probar la conexion basica
	ThreadManager_CreateThread(&hebraSAFA, (void*) esperarRespuesta, (void*) socketSAFA);
	ThreadManager_CreateThread(&hebraFM9, (void*) esperarRespuesta, (void*) socketFM9);
	ThreadManager_CreateThread(&hebraMDJ, (void*) esperarRespuesta, (void*) socketMDJ);

	//Espero a que los hilos terminen
	pthread_join(hebraSAFA, NULL);
	pthread_join(hebraFM9, NULL);
	pthread_join(hebraMDJ, NULL);

	liberarVariablesGlobales();					//Libero la memoria de las variables globales
	exit_gracefully(0);
}

