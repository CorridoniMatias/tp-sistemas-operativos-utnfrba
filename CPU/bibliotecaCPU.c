#include "bibliotecaCPU.h"

void configurar()
{
	settings = (Configuracion*)malloc(sizeof(Configuracion));

	char* campos[] = {
			"IP_SAFA",
			"PUERTO_SAFA",
			"IP_DIEGO",
			"PUERTO_DIEGO",
			"RETARDO"
	};

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	strcpy(settings->ipSAFA, archivoConfigSacarStringDe(archivoConfig, "IP_SAFA"));
	strcpy(settings->puertoSAFA, archivoConfigSacarStringDe(archivoConfig, "PUERTO_SAFA"));
	strcpy(settings->ipDIEGO, archivoConfigSacarStringDe(archivoConfig, "IP_DIEGO"));
	strcpy(settings->puertoDIEGO, archivoConfigSacarStringDe(archivoConfig, "PUERTO_DIEGO"));
	settings->retardo = archivoConfigSacarIntDe(archivoConfig, "RETARDO");

	free(campos);
	archivoConfigDestruir(archivoConfig);

}

int conectarAProceso(char* ip, char* puerto, char* nombreProceso)
{
	//todo PODRIAMOS PONER ESTA FUNCION PARA QUE LA USEMOS TODOS AUQNEU LA MIA PODRIA SER VOID TRANQUILAMENTE
	int socket;
	socket = SocketClient_ConnectToServerIP(ip, puerto);	//Me conecto al servidor que me dijeron

	if(socket == -1)
	{
		Logger_Log(LOG_ERROR, "Error al conectar el CPU al proceso %s!", nombreProceso);
		return -1;
	}

	Logger_Log(LOG_INFO, "CPU conectado al proceso %s!", nombreProceso);	//Logueo que me conecte
	SocketCommons_SendMessageString(socket, "iam CPU");

	free(nombreProceso);				//Anti-memory leak

	return socket;


}

