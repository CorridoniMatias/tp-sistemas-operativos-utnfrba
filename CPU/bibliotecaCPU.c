#include "bibliotecaCPU.h"

void configurar(Configuracion* configuracion)
{

	char* campos[] = {
			"IP_SAFA",
			"PUERTO_SAFA",
			"IP_DIEGO",
			"PUERTO_DIEGO",
			"RETARDO"
	};

	t_config* archivoConfig = archivoConfigCrear(RUTA_CONFIG, campos);

	strcpy(configuracion->ipSAFA, archivoConfigSacarStringDe(archivoConfig, "IP_SAFA"));
	strcpy(configuracion->puertoSAFA, archivoConfigSacarStringDe(archivoConfig, "PUERTO_SAFA"));
	strcpy(configuracion->ipDIEGO, archivoConfigSacarStringDe(archivoConfig, "IP_DIEGO"));
	strcpy(configuracion->puertoDIEGO, archivoConfigSacarStringDe(archivoConfig, "PUERTO_DIEGO"));
	strcpy(configuracion->retardo, archivoConfigSacarIntDe(archivoConfig, "RETARDO"));

	archivoConfigDestruir(archivoConfig);

}

void conectarAProceso(char* ip, char* puerto, char* nombreProceso)
{


	SocketClient_ConnectToServer(ip, puerto);

	Logger_Log(LOG_INFO, "CPU conectado al proceso %s!", nombreProceso);

	free(nombreProceso);


}

