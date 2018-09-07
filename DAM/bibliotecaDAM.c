#include "bibliotecaDAM.h"

void configurar(Configuracion* configuracion)
{

	char* campos[] = {
			"PUERTO_ESCUCHA",
			"IP_SAFA",
			"PUERTO_SAFA",
			"IP_MDJ",
			"PUERTO_MDJ",
			"IP_FM9",
			"PUERTO_FM9",
			"TRANSFER_SIZE"
	};

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	strcpy(configuracion->puertoEscucha, archivoConfigSacarStringDe(archivoConfig, "PUERTO_ESCUCHA"));
	strcpy(configuracion->ipSAFA, archivoConfigSacarStringDe(archivoConfig, "IP_SAFA"));
	strcpy(configuracion->puertoSAFA, archivoConfigSacarStringDe(archivoConfig, "PUERTO_SAFA"));
	strcpy(configuracion->ipMDJ, archivoConfigSacarStringDe(archivoConfig, "IP_MDJ"));
	strcpy(configuracion->puertoMDJ, archivoConfigSacarStringDe(archivoConfig, "PUERTO_MDJ"));
	strcpy(configuracion->ipFM9, archivoConfigSacarStringDe(archivoConfig, "IP_FM9"));
	strcpy(configuracion->puertoFM9, archivoConfigSacarStringDe(archivoConfig, "PUERTO_FM9"));
	configuracion->transferSize = archivoConfigSacarIntDe(archivoConfig, "TRANSFER_SIZE");

	archivoConfigDestruir(archivoConfig);

}

void conectarAProceso(char* ip, char* puerto, char* nombreProceso)
{

	int socketConexion;
	socketConexion = SocketClient_ConnectToServerIP(ip, puerto);	//Me conecto al servidor que me dijeron

	Logger_Log(LOG_INFO, "DAM conectado al proceso %s!", nombreProceso);	//Logueo que me conecte

	free(nombreProceso);				//Anti-memory leak

	ThreadPoolRunnable* handshake = ThreadPool_CreateRunnable();

	//handshake->

}
